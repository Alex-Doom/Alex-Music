#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDirIterator>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QTimer>
#include <QScrollBar>
#include <QShortcut>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenuBar>
#include <QHeaderView>
#include <QProgressDialog>
#include <QElapsedTimer>
#include <QStatusBar>

#include "HtmlDelegate.h"
#include "TrackValidator.h"
#include "BadTrackDialog.h"
#include "MainWindow.moc"
#include "FastMetadataReader.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <commctrl.h>
#include <shobjidl.h>
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , playlist_(new Playlist(this))
    , validator_(new TrackValidator(this))
    , volumeBeforeMute_(70)
    , autoSkipBad_(false)
    , isScanning_(false)
#ifdef Q_OS_WIN
    , taskbarList_(nullptr)
    , thumbBarReady_(false)
#endif
{
    setupUI();
    setupShortcuts();
    createMenu();

    // Инициализация плеера
    player_ = new QMediaPlayer(this);
    audio_ = new QAudioOutput(this);
    player_->setAudioOutput(audio_);
    audio_->setVolume(volumeBeforeMute_ / 100.0);

    // Соединения плеера
    connect(player_, &QMediaPlayer::positionChanged,
            this, &MainWindow::onPositionChanged);
    connect(player_, &QMediaPlayer::durationChanged,
            this, &MainWindow::onDurationChanged);
    connect(player_, &QMediaPlayer::mediaStatusChanged,
            this, &MainWindow::onMediaStatusChanged);
    connect(player_, &QMediaPlayer::errorOccurred,
            [this](QMediaPlayer::Error error, const QString& errorString) {
                qDebug() << "Player error:" << error << errorString;
                if (autoSkipBad_) {
                    onNext();
                }
            });

    // Соединения плейлиста
    connect(playlist_, &Playlist::currentChanged,
            this, &MainWindow::updateUI);

    // Загрузка сохранённых настроек
    QSettings settings("AlexMusic", "Player");
    autoSkipBad_ = settings.value("autoSkipBad", false).toBool();
    volumeBeforeMute_ = settings.value("volume", 70).toInt();
    controls_->setVolume(volumeBeforeMute_);
    audio_->setVolume(volumeBeforeMute_ / 100.0);

    // Автозагрузка папки Music
    QString musicPath = QDir::homePath() + "/Music";
    if (QDir(musicPath).exists()) {
        QTimer::singleShot(100, [this, musicPath]() {
            loadFolder(musicPath);
        });
    }
}

MainWindow::~MainWindow() {
#ifdef Q_OS_WIN
    cleanupThumbnailToolbar();
#endif
}

void MainWindow::setupUI() {
    setWindowTitle("AlexMusic");
    resize(1200, 800);

    auto* central = new QWidget;
    setCentralWidget(central);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // Верхняя панель: Папка | Поиск | Сортировка
    auto* topPanel = new QHBoxLayout;

    auto* folderBtn = new QPushButton("📁 Открыть папку");
    folderBtn->setStyleSheet("padding: 8px 15px; font-weight: bold;");

    searchEdit_ = new QLineEdit;
    searchEdit_->setPlaceholderText("🔍 Поиск треков...");
    searchEdit_->setClearButtonEnabled(true);
    searchEdit_->setStyleSheet(
        "QLineEdit {"
        "  background: #2d2d2d;"
        "  border: 1px solid #444;"
        "  border-radius: 20px;"
        "  padding: 8px 15px;"
        "  color: white;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus { border: 2px solid #0078d4; }"
        );

    topPanel->addWidget(folderBtn);
    topPanel->addSpacing(20);
    topPanel->addWidget(searchEdit_, 1);

    mainLayout->addLayout(topPanel);

    // Центральная часть: Обложка слева, таблица справа
    auto* contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(20);

    // Левая панель (обложка и инфо)
    auto* leftPanel = new QVBoxLayout;
    leftPanel->setAlignment(Qt::AlignHCenter);

    coverLabel_ = new QLabel;
    coverLabel_->setFixedSize(350, 350);
    coverLabel_->setStyleSheet(
        "QLabel {"
        "  background: #1a1a1a;"
        "  border: 2px solid #333;"
        "  border-radius: 8px;"
        "  color: #666;"
        "}"
        );
    coverLabel_->setAlignment(Qt::AlignCenter);
    coverLabel_->setText("Нет обложки");

    titleLabel_ = new QLabel("Выберите папку с музыкой");
    titleLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(true);

    artistLabel_ = new QLabel("");
    artistLabel_->setStyleSheet("font-size: 14px; color: #666;");
    artistLabel_->setAlignment(Qt::AlignCenter);

    leftPanel->addWidget(coverLabel_);
    leftPanel->addSpacing(15);
    leftPanel->addWidget(titleLabel_);
    leftPanel->addWidget(artistLabel_);
    leftPanel->addStretch();

    contentLayout->addLayout(leftPanel, 0);

    // Правая панель (таблица треков)
    table_ = new QTableWidget;
    table_->setColumnCount(6);
    table_->setHorizontalHeaderLabels({
        "Название", "Исполнитель", "Альбом", "Жанр", "Год", "★"
    });

    // Настройка таблицы
    table_->horizontalHeader()->setStretchLastSection(false);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    table_->horizontalHeader()->resizeSection(5, 100);

    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->setSortingEnabled(true);
    table_->verticalHeader()->setVisible(false);
    table_->setStyleSheet(
        "QTableWidget {"
        "  background: #ffffff;"
        "  border: 1px solid #ddd;"
        "  border-radius: 8px;"
        "  gridline-color: #eee;"
        "}"
        "QTableWidget::item:selected {"
        "  background: #0078d4;"
        "  color: white;"
        "}"
        "QHeaderView::section {"
        "  background: #f0f0f0;"
        "  padding: 8px;"
        "  border: none;"
        "  border-bottom: 2px solid #ddd;"
        "  font-weight: bold;"
        "}"
        );

    // Делегаты
    auto* htmlDelegate = new HtmlDelegate(this);
    table_->setItemDelegateForColumn(0, htmlDelegate);
    table_->setItemDelegateForColumn(1, htmlDelegate);

    ratingDelegate_ = new RatingDelegate(this);
    table_->setItemDelegateForColumn(5, ratingDelegate_);
    connect(ratingDelegate_, &RatingDelegate::ratingChanged,
            this, &MainWindow::onRatingInTable);

    table_->installEventFilter(this);
    contentLayout->addWidget(table_, 1);

    mainLayout->addLayout(contentLayout, 1);

    // Панель управления внизу
    controls_ = new PlayerControls;
    controls_->setStyleSheet(
        "PlayerControls {"
        "  background: #f5f5f5;"
        "  border: 1px solid #ddd;"
        "  border-radius: 8px;"
        "}"
        );
    mainLayout->addWidget(controls_);

    // Соединения элементов управления
    connect(folderBtn, &QPushButton::clicked, this, &MainWindow::openFolder);
    connect(searchEdit_, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    connect(table_, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onTableDoubleClick);
    connect(table_->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onSort);

    // Соединения плеера
    connect(controls_, &PlayerControls::playPause, this, &MainWindow::onPlayPause);
    connect(controls_, &PlayerControls::next, this, &MainWindow::onNext);
    connect(controls_, &PlayerControls::prev, this, &MainWindow::onPrev);
    connect(controls_, &PlayerControls::seek, this, &MainWindow::onSeek);
    connect(controls_, &PlayerControls::volumeChanged, this, &MainWindow::onVolumeChanged);
    connect(controls_, &PlayerControls::muteToggled, this, &MainWindow::onMute);
    connect(controls_, &PlayerControls::repeatToggled, this, &MainWindow::onRepeat);
    connect(controls_, &PlayerControls::shuffleToggled, this, &MainWindow::onShuffle);
}

void MainWindow::setupShortcuts() {
    // Воспроизведение
    new QShortcut(QKeySequence(Qt::Key_Space), this, this, &MainWindow::onPlayPause);
    new QShortcut(QKeySequence("B"), this, this, &MainWindow::onPrev);
    new QShortcut(QKeySequence("N"), this, this, &MainWindow::onNext);

    // Навигация по треку
    new QShortcut(QKeySequence(Qt::Key_Left), this, [this]() {
        if (player_->position() > 5000) {
            player_->setPosition(player_->position() - 5000);
        } else {
            player_->setPosition(0);
        }
    });
    new QShortcut(QKeySequence(Qt::Key_Right), this, [this]() {
        player_->setPosition(qMin(player_->duration(), player_->position() + 5000));
    });
    new QShortcut(QKeySequence(Qt::Key_Home), this, [this]() {
        player_->setPosition(0);
    });

    // Громкость
    new QShortcut(QKeySequence("M"), this, [this]() {
        controls_->onMuteClicked();
    });
    new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left), this, [this]() {
        int vol = qMax(0, controls_->volume() - 10);
        controls_->setVolume(vol);
        onVolumeChanged(vol);
    });
    new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right), this, [this]() {
        int vol = qMin(100, controls_->volume() + 10);
        controls_->setVolume(vol);
        onVolumeChanged(vol);
    });

    // Поиск и навигация
    new QShortcut(QKeySequence("Ctrl+F"), this, [this]() {
        searchEdit_->setFocus();
        searchEdit_->selectAll();
    });
    new QShortcut(QKeySequence("Ctrl+G"), this, &MainWindow::scrollToCurrent);
    new QShortcut(QKeySequence("Ctrl+O"), this, &MainWindow::openFolder);

    // Режимы
    new QShortcut(QKeySequence("Ctrl+R"), this, &MainWindow::onRepeat);
    new QShortcut(QKeySequence("Ctrl+S"), this, &MainWindow::onShuffle);
}

void MainWindow::createMenu() {
    auto* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // Файл
    auto* fileMenu = menuBar->addMenu("Файл");
    auto* openAct = fileMenu->addAction("📁 Открыть папку", this, &MainWindow::openFolder);
    openAct->setShortcut(QKeySequence("Ctrl+O"));
    fileMenu->addSeparator();
    fileMenu->addAction("❌ Выход", this, &QWidget::close, QKeySequence("Alt+F4"));

    // Настройки
    auto* settingsMenu = menuBar->addMenu("Настройки");
    auto* settingsAct = settingsMenu->addAction("⚙ Настройки", this, &MainWindow::showSettings);
    settingsAct->setShortcut(QKeySequence("Ctrl+P"));

    auto* skipAct = settingsMenu->addAction("Пропускать повреждённые треки");
    skipAct->setCheckable(true);
    skipAct->setChecked(autoSkipBad_);
    connect(skipAct, &QAction::triggered, [this](bool checked) {
        autoSkipBad_ = checked;
        QSettings("AlexMusic", "Player").setValue("autoSkipBad", checked);
    });

    // Справка
    auto* helpMenu = menuBar->addMenu("Справка");
    helpMenu->addAction("⌨ Горячие клавиши", this, [this]() {
        QMessageBox::information(this, "Горячие клавиши",
                                 "<b>Управление:</b><br>"
                                 "Пробел — Play/Pause<br>"
                                 "B — Предыдущий трек<br>"
                                 "N — Следующий трек<br>"
                                 "← → — Перемотка на 5 сек<br>"
                                 "Ctrl+F — Поиск<br>"
                                 "Ctrl+G — К текущему треку<br>"
                                 "M — Отключить звук");
    });
    helpMenu->addAction("ℹ О программе", this, [this]() {
        QMessageBox::about(this, "О программе",
                           "<b>AlexMusic v2.0</b><br><br>"
                           "Музыкальный плеер с поддержкой MP3,<br>"
                           "рейтингами и интеллектуальной сортировкой.");
    });
}

void MainWindow::loadFolder(const QString& path) {
    if (isScanning_) return;
    isScanning_ = true;

    QDirIterator it(path, {"*.mp3"}, QDir::Files, QDirIterator::Subdirectories);
    QStringList files;
    while (it.hasNext()) {
        files << it.next();
    }

    if (files.isEmpty()) {
        isScanning_ = false;
        return;
    }

    QProgressDialog progress("Сканирование MP3...", "Отмена", 0, files.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    playlist_->clear();
    playlist_->reserve(files.size());

    table_->setUpdatesEnabled(false);
    table_->setSortingEnabled(false);
    table_->clearContents();
    table_->setRowCount(files.size());

    for (int i = 0; i < files.size(); ++i) {
        progress.setValue(i);
        if (progress.wasCanceled()) break;

        Track track(files[i]);
        playlist_->add(track);

        // Быстрое заполнение таблицы
        auto* titleItem = new QTableWidgetItem(track.title());
        titleItem->setData(Qt::UserRole, track.path()); // Полный путь
        titleItem->setData(Qt::UserRole + 1, track.title()); // Оригинал для поиска
        table_->setItem(i, 0, titleItem);

        table_->setItem(i, 1, new QTableWidgetItem(track.artist()));
        table_->setItem(i, 2, new QTableWidgetItem(track.album()));
        table_->setItem(i, 3, new QTableWidgetItem(track.genre()));
        table_->setItem(i, 4, new QTableWidgetItem(track.year()));

        auto* ratingItem = new QTableWidgetItem();
        ratingItem->setData(Qt::UserRole + 1, 0.0);
        table_->setItem(i, 5, ratingItem);

        if (i % 100 == 0) QApplication::processEvents();
    }

    table_->setUpdatesEnabled(true);
    table_->setSortingEnabled(true);

    // Загрузка рейтингов
    playlist_->loadRatings();
    for (int i = 0; i < table_->rowCount() && i < static_cast<int>(playlist_->size()); ++i) {
        double rating = playlist_->tracks()[i].rating();
        if (rating > 0) {
            table_->item(i, 5)->setData(Qt::UserRole + 1, rating);
        }
    }

    isScanning_ = false;

    if (playlist_->size() > 0) {
        playlist_->setCurrent(0);
    }

    // Фоновая загрузка метаданных
    QtConcurrent::run([this, files]() {
        for (int i = 0; i < files.size() && i < table_->rowCount(); ++i) {
            auto meta = MetadataCache::instance().get(files[i]);
            if (!meta.valid) continue;

            QMetaObject::invokeMethod(this, [this, i, meta]() {
                if (i >= table_->rowCount()) return;

                // Обновляем только если данные отличаются от дефолтных
                if (!meta.title.isEmpty() && meta.title != table_->item(i, 0)->text()) {
                    table_->item(i, 0)->setText(meta.title);
                    table_->item(i, 0)->setData(Qt::UserRole + 1, meta.title);
                }
                if (!meta.artist.isEmpty()) table_->item(i, 1)->setText(meta.artist);
                if (!meta.album.isEmpty()) table_->item(i, 2)->setText(meta.album);
                if (!meta.genre.isEmpty()) table_->item(i, 3)->setText(meta.genre);
                if (!meta.year.isEmpty()) table_->item(i, 4)->setText(meta.year);
            }, Qt::QueuedConnection);
        }
    });
}

void MainWindow::openFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с музыкой");
    if (!dir.isEmpty()) {
        loadFolder(dir);
    }
}

void MainWindow::onPlayPause() {
    if (player_->playbackState() == QMediaPlayer::PlayingState) {
        player_->pause();
        controls_->setPlaying(false);
    } else {
        if (player_->source().isEmpty() && playlist_->size() > 0) {
            playTrackAt(static_cast<int>(playlist_->currentIndex()));
        } else {
            player_->play();
            controls_->setPlaying(true);
        }
    }
#ifdef Q_OS_WIN
    updateThumbnailButtons();
#endif
}

void MainWindow::onNext() {
    if (playlist_->size() == 0) return;

    if (!playlist_->next()) return;

    auto track = playlist_->current();
    if (track) {
        validateAndPlay(track->path(), true);
    }
}

void MainWindow::onPrev() {
    if (playlist_->size() == 0) return;

    qint64 pos = player_->position();
    if (!playlist_->prev(pos)) return;

    // Правило 3 секунд
    if (pos > 3000 && player_->duration() > 0) {
        player_->setPosition(0);
        updateUI();
        return;
    }

    auto track = playlist_->current();
    if (track) {
        validateAndPlay(track->path(), false);
    }
}

bool MainWindow::validateAndPlay(const QString& path, bool forward) {
    if (validator_->validate(path)) {
        player_->setSource(QUrl::fromLocalFile(path));
        player_->play();
        controls_->setPlaying(true);
#ifdef Q_OS_WIN
        updateThumbnailButtons();
#endif
        return true;
    }

    // Обработка битого трека
    if (autoSkipBad_) {
        // Рекурсивный пропуск
        if (forward) {
            onNext();
        } else {
            onPrev();
        }
        return false;
    } else {
        BadTrackDialog dlg(this);
        dlg.setTrack(path, validator_->lastError());
        if (dlg.exec() == QDialog::Accepted) {
            if (dlg.skipAlways()) {
                autoSkipBad_ = true;
                QSettings("AlexMusic", "Player").setValue("autoSkipBad", true);
            }
            if (forward) {
                onNext();
            } else {
                onPrev();
            }
            return false;
        } else {
            player_->stop();
            controls_->setPlaying(false);
            return false;
        }
    }
}

void MainWindow::playTrackAt(int row) {
    if (row < 0 || row >= static_cast<int>(playlist_->size())) return;

    playlist_->setCurrent(static_cast<size_t>(row));
    auto track = playlist_->current();
    if (track) {
        validateAndPlay(track->path(), true);
    }
}

void MainWindow::onSeek(qint64 position) {
    if (player_->duration() > 0) {
        qint64 target = (position * player_->duration()) / 1000;
        player_->setPosition(target);
    }
}

void MainWindow::onVolumeChanged(int vol) {
    volumeBeforeMute_ = vol;
    audio_->setVolume(vol / 100.0);
    QSettings("AlexMusic", "Player").setValue("volume", vol);
}

void MainWindow::onMute(bool muted) {
    if (muted) {
        audio_->setVolume(0);
    } else {
        audio_->setVolume(volumeBeforeMute_ / 100.0);
    }
}

void MainWindow::onRepeat() {
    auto mode = playlist_->repeatMode();
    playlist_->setRepeatMode(mode == Playlist::RepeatMode::One ?
                                 Playlist::RepeatMode::None :
                                 Playlist::RepeatMode::One);
    controls_->setRepeat(mode == Playlist::RepeatMode::None);
}

void MainWindow::onShuffle() {
    bool shuffled = !playlist_->isShuffled();
    playlist_->setShuffle(shuffled);
    controls_->setShuffle(shuffled);
}

void MainWindow::onPositionChanged(qint64 pos) {
    controls_->setPosition(pos, player_->duration());
}

void MainWindow::onDurationChanged(qint64 dur) {
    controls_->setPosition(player_->position(), dur);
}

void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        if (playlist_->repeatMode() == Playlist::RepeatMode::One) {
            player_->setPosition(0);
            player_->play();
        } else {
            onNext();
        }
    }
}

void MainWindow::onTableDoubleClick(int row, int) {
    playTrackAt(row);
}

void MainWindow::onRatingInTable(int row, int rating) {
    if (row < 0 || row >= static_cast<int>(playlist_->size())) return;

    // Обновляем модель
    const_cast<Track&>(playlist_->tracks()[row]).setRating(rating);
    playlist_->saveRatings();

    // Обновляем текущий трек если нужно
    if (static_cast<size_t>(row) == playlist_->currentIndex()) {
        updateUI();
    }
}

void MainWindow::onSearch(const QString& text) {
    QString lowerText = text.toLower();

    for (int i = 0; i < table_->rowCount(); ++i) {
        bool visible = text.isEmpty();

        if (!visible) {
            // Поиск по всем колонкам кроме рейтинга
            for (int col = 0; col < 5; ++col) {
                auto* item = table_->item(i, col);
                if (item && item->text().toLower().contains(lowerText)) {
                    visible = true;
                    break;
                }
            }
        }

        table_->setRowHidden(i, !visible);

        // Подсветка совпадений
        if (visible && !text.isEmpty()) {
            auto* titleItem = table_->item(i, 0);
            if (titleItem) {
                QString original = titleItem->data(Qt::UserRole + 1).toString();
                if (original.isEmpty()) original = titleItem->text();

                // HTML подсветка
                QString highlighted = original;
                int idx = highlighted.toLower().indexOf(lowerText);
                if (idx >= 0) {
                    highlighted.insert(idx + text.length(), "</span>");
                    highlighted.insert(idx, "<span style='background:#ffeb3b;color:black;'>");
                    titleItem->setText(highlighted);
                }
            }
        } else {
            // Восстановление оригинала
            auto* titleItem = table_->item(i, 0);
            if (titleItem) {
                QString original = titleItem->data(Qt::UserRole + 1).toString();
                if (!original.isEmpty()) titleItem->setText(original);
            }
        }
    }
}

void MainWindow::onSort(int column) {
    // Qt автоматически сортирует при включенном setSortingEnabled
    // Дополнительная логика для специальных случаев
    if (column == 5) { // Рейтинг
        // Сортировка по числовому значению, а не строке
        table_->sortByColumn(column, table_->horizontalHeader()->sortIndicatorOrder());
    }
}

void MainWindow::scrollToCurrent() {
    int row = static_cast<int>(playlist_->currentIndex());
    if (row >= 0 && row < table_->rowCount()) {
        table_->selectRow(row);
        table_->scrollToItem(table_->item(row, 0), QAbstractItemView::PositionAtCenter);
    }
}

void MainWindow::updateUI() {
    auto track = playlist_->current();
    if (!track) {
        titleLabel_->setText("Нет трека");
        artistLabel_->setText("");
        coverLabel_->setText("Нет обложки");
        return;
    }

    titleLabel_->setText(track->title());
    artistLabel_->setText(track->artist());

    // Обложка
    QImage cover = track->cover();
    if (!cover.isNull()) {
        coverLabel_->setPixmap(QPixmap::fromImage(cover).scaled(
            330, 330, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        coverLabel_->setText("");
    } else {
        coverLabel_->setText("Нет обложки");
        coverLabel_->setPixmap(QPixmap());
    }

    highlightCurrentRow();
}

void MainWindow::highlightCurrentRow() {
    int current = static_cast<int>(playlist_->currentIndex());
    for (int i = 0; i < table_->rowCount(); ++i) {
        if (i == current) {
            table_->selectRow(i);
            // Прокрутка только если строка не видна
            QRect visual = table_->visualItemRect(table_->item(i, 0));
            if (visual.top() < 0 || visual.bottom() > table_->viewport()->height()) {
                table_->scrollToItem(table_->item(i, 0), QAbstractItemView::EnsureVisible);
            }
            break;
        }
    }
}

void MainWindow::showSettings() {
    if (!settingsDlg_) {
        settingsDlg_ = new SettingsDialog(this);
    }

    settingsDlg_->setAutoSkip(autoSkipBad_);
    settingsDlg_->setVolume(volumeBeforeMute_);

    if (settingsDlg_->exec() == QDialog::Accepted) {
        autoSkipBad_ = settingsDlg_->autoSkipBadTracks();
        volumeBeforeMute_ = settingsDlg_->defaultVolume();
        audio_->setVolume(volumeBeforeMute_ / 100.0);
        controls_->setVolume(volumeBeforeMute_);

        QSettings settings("AlexMusic", "Player");
        settings.setValue("autoSkipBad", autoSkipBad_);
        settings.setValue("volume", volumeBeforeMute_);
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == table_ && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            auto selected = table_->selectedItems();
            if (!selected.isEmpty()) {
                playTrackAt(selected.first()->row());
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// Windows Thumbnail Toolbar
#ifdef Q_OS_WIN

void MainWindow::setupThumbnailToolbar() {
    if (thumbBarReady_) return;

    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITaskbarList3, &taskbarList_);
    if (SUCCEEDED(hr)) {
        ITaskbarList3* tbl = static_cast<ITaskbarList3*>(taskbarList_);
        hr = tbl->HrInit();

        if (SUCCEEDED(hr)) {
            THUMBBUTTON buttons[3] = {};

            // Prev
            buttons[0].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            buttons[0].iId = 0;
            buttons[0].hIcon = LoadIcon(NULL, IDI_HAND); // Заменить на свою иконку
            wcscpy(buttons[0].szTip, L"Предыдущий");
            buttons[0].dwFlags = THBF_ENABLED;

            // Play/Pause
            buttons[1].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            buttons[1].iId = 1;
            buttons[1].hIcon = LoadIcon(NULL, IDI_HAND);
            wcscpy(buttons[1].szTip, L"Воспроизведение");
            buttons[1].dwFlags = THBF_ENABLED;

            // Next
            buttons[2].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            buttons[2].iId = 2;
            buttons[2].hIcon = LoadIcon(NULL, IDI_HAND);
            wcscpy(buttons[2].szTip, L"Следующий");
            buttons[2].dwFlags = THBF_ENABLED;

            hr = tbl->ThumbBarAddButtons(reinterpret_cast<HWND>(winId()), 3, buttons);
            if (SUCCEEDED(hr)) {
                thumbBarReady_ = true;
            }
        }
    }
}

void MainWindow::updateThumbnailButtons() {
    if (!thumbBarReady_ || !taskbarList_) return;

    ITaskbarList3* tbl = static_cast<ITaskbarList3*>(taskbarList_);
    THUMBBUTTON button = {};
    button.dwMask = THB_TOOLTIP | THB_ICON;
    button.iId = 1;

    bool playing = player_->playbackState() == QMediaPlayer::PlayingState;
    wcscpy(button.szTip, playing ? L"Пауза" : L"Воспроизведение");
    // button.hIcon = playing ? pauseIcon_ : playIcon_; // Установить реальные иконки

    tbl->ThumbBarUpdateButtons(reinterpret_cast<HWND>(winId()), 1, &button);
}

void MainWindow::cleanupThumbnailToolbar() {
    if (taskbarList_) {
        static_cast<ITaskbarList3*>(taskbarList_)->Release();
        taskbarList_ = nullptr;
    }
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_COMMAND && thumbBarReady_) {
            int btnId = LOWORD(msg->wParam);
            switch (btnId) {
            case 0: onPrev(); return true;
            case 1: onPlayPause(); return true;
            case 2: onNext(); return true;
            }
        }
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    if (!thumbBarReady_) {
        QTimer::singleShot(100, this, &MainWindow::setupThumbnailToolbar);
    }
}

#else // Non-Windows stubs

void MainWindow::setupThumbnailToolbar() {}
void MainWindow::updateThumbnailButtons() {}
void MainWindow::cleanupThumbnailToolbar() {}

#endif
