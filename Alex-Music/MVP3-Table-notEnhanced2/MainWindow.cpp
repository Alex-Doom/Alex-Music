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
#include <QButtonGroup>
#include <QCoreApplication>
#include <QTimer>
#include <QScrollBar>
#include <QShortcut>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenuBar>
#include <QHeaderView>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QApplication>

#include "HtmlDelegate.h"
#include "TrackValidator.h"
#include "BadTrackDialog.h"
#include "FastMetadataReader.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <commctrl.h>
#include <shobjidl.h>
#endif

                                                               MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AlexMusic");

    savedShuffleState_ = false;
    savedRepeatMode_ = Playlist::RepeatMode::None;
    alwaysSkipBadTracks_ = false;
    lastWasForward_ = true;

    trackValidator = new TrackValidator(this);
    connect(trackValidator, &TrackValidator::validationFailed,
            this, &MainWindow::handleInvalidTrack);

    scannerThread = new ScannerThread(this);
    connect(scannerThread, &ScannerThread::trackFound, this, &MainWindow::onTrackFound);
    connect(scannerThread, &ScannerThread::scanProgress, this, &MainWindow::onScanProgress);
    connect(scannerThread, &ScannerThread::scanFinished, this, &MainWindow::onScanFinished);

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(volumeBeforeMute_ / 100.0);

    settingsDialog = new SettingsDialog(this);

    setupUI();
    setupShortcuts();
    createMenuBar();
    loadSettings();
    updateMenuBar();

    thumbnailToolbarInitialized = false;
    taskbarList = nullptr;

    QString defaultFolder = "C:\\Users\\User\\Music";
    if (QDir(defaultFolder).exists()) {
        QTimer::singleShot(100, [this, defaultFolder]() {
            scanFolder(defaultFolder);
        });
    }
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QHBoxLayout* topBar = new QHBoxLayout;

    QPushButton* folderBtn = new QPushButton("📁 Выбрать папку с музыкой");
    connect(folderBtn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с MP3");
        if (!dir.isEmpty()) scanFolder(dir);
    });
    topBar->addWidget(folderBtn);
    topBar->addStretch();

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск треков...");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setStyleSheet(
        "QLineEdit { "
        "background: #222; border: 1px solid #444; border-radius: 15px; "
        "padding: 8px 12px; color: #fff; font-size: 14px; }"
        "QLineEdit:focus { border: 2px solid #0078d4; }"
        );
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    topBar->addWidget(searchEdit);

    sortAlphabeticalBtn = new QPushButton("А-Я");
    sortStandardBtn = new QPushButton("Станд");
    sortReverseBtn = new QPushButton("Реверс");

    for (auto* btn : {sortAlphabeticalBtn, sortStandardBtn, sortReverseBtn}) {
        btn->setFixedSize(50, 35);
        topBar->addWidget(btn);
    }

    connect(sortAlphabeticalBtn, &QPushButton::clicked, this, &MainWindow::onSortAlphabeticalClicked);
    connect(sortStandardBtn, &QPushButton::clicked, this, &MainWindow::onSortStandardClicked);
    connect(sortReverseBtn, &QPushButton::clicked, this, &MainWindow::onSortReverseClicked);

    mainLayout->addLayout(topBar);

    QHBoxLayout* contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(30);

    QWidget* leftPanel = new QWidget;
    leftPanel->setFixedWidth(400);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);

    coverLabel = new QLabel;
    coverLabel->setFixedSize(250, 250);
    coverLabel->setStyleSheet("QLabel { background: #222; border: 2px solid #444; border-radius: 10px; color: #fff; }");
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setText("No Cover");
    leftLayout->addWidget(coverLabel, 0, Qt::AlignCenter);

    albumLabel = new QLabel("Выберите папку с музыкой");
    albumLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #000; }");
    albumLabel->setWordWrap(true);
    albumLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    albumLabel->setCursor(Qt::IBeamCursor);
    leftLayout->addWidget(albumLabel);

    artistLabel = new QLabel();
    artistLabel->setStyleSheet("QLabel { font-size: 16px; color: #000; }");
    artistLabel->setWordWrap(true);
    artistLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    artistLabel->setCursor(Qt::IBeamCursor);
    leftLayout->addWidget(artistLabel);

    QWidget* ratingWidget = new QWidget;
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setSpacing(5);
    ratingLayout->setAlignment(Qt::AlignLeft);

    for (int i = 0; i < 5; ++i) {
        starButtons[i] = new QPushButton("☆");
        starButtons[i]->setFixedSize(30, 30);
        starButtons[i]->setStyleSheet(
            "QPushButton { background: #333; border: 1px solid #555; "
            "border-radius: 15px; color: #ffcc00; font-size: 16px; }"
            "QPushButton:hover { background: #444; }"
            );
        connect(starButtons[i], &QPushButton::clicked, [this, i]() {
            onRatingChanged(i + 1);
        });
        ratingLayout->addWidget(starButtons[i]);
    }
    ratingLayout->addStretch();
    leftLayout->addWidget(ratingWidget);

    contentLayout->addWidget(leftPanel);

    trackTable = new QTableWidget;
    setupTrackTable();
    contentLayout->addWidget(trackTable, 1);

    mainLayout->addLayout(contentLayout, 1);

    controls = new PlayerControls;
    controls->setStyleSheet(
        "PlayerControls { background: #111; border: 1px solid #333; border-radius: 10px; }"
        "QPushButton { background: #333; color: #fff; border: 1px solid #444; "
        "border-radius: 8px; padding: 8px; font-size: 16px; }"
        "QPushButton:hover { background: #444; }"
        "QPushButton:pressed { background: #555; }"
        );
    mainLayout->addWidget(controls);

    connect(trackTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableDoubleClicked);
    connect(controls, &PlayerControls::playPauseClicked, this, &MainWindow::onPlayPauseClicked);
    connect(controls, &PlayerControls::nextClicked, this, &MainWindow::onNextClicked);
    connect(controls, &PlayerControls::prevClicked, this, &MainWindow::onPrevClicked);
    connect(controls, &PlayerControls::seek, this, &MainWindow::onSeek);
    connect(controls, &PlayerControls::volumeChanged, this, &MainWindow::onVolumeChanged);
    connect(controls, &PlayerControls::repeatClicked, this, &MainWindow::onRepeatClicked);
    connect(controls, &PlayerControls::shuffleClicked, this, &MainWindow::onShuffleClicked);
    connect(controls, &PlayerControls::muteToggled, this, &MainWindow::onMuteToggled);

    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onMediaStatusChanged);

    trackTable->installEventFilter(this);

    setStyleSheet(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #80A6FF, stop:1 #f0fff0); }"
        );
    resize(1000, 700);
}

void MainWindow::setupTrackTable() {
    trackTable->setColumnCount(COL_COUNT);
    QStringList headers;
    headers << "Название трека" << "Исполнитель" << "Жанр" << "Альбом" << "Рейтинг" << "Год";
    trackTable->setHorizontalHeaderLabels(headers);

    trackTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    trackTable->setSelectionMode(QAbstractItemView::SingleSelection);
    trackTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trackTable->setAlternatingRowColors(true);
    trackTable->setSortingEnabled(true);

    trackTable->horizontalHeader()->setSectionResizeMode(COL_TITLE, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_ARTIST, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_GENRE, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_ALBUM, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_RATING, QHeaderView::Fixed);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_YEAR, QHeaderView::Fixed);

    trackTable->horizontalHeader()->resizeSection(COL_TITLE, 250);
    trackTable->horizontalHeader()->resizeSection(COL_ARTIST, 150);
    trackTable->horizontalHeader()->resizeSection(COL_GENRE, 100);
    trackTable->horizontalHeader()->resizeSection(COL_ALBUM, 150);
    trackTable->horizontalHeader()->resizeSection(COL_RATING, 120);
    trackTable->horizontalHeader()->resizeSection(COL_YEAR, 80);

    HtmlDelegate* htmlDelegate = new HtmlDelegate(this);
    trackTable->setItemDelegate(htmlDelegate);

    RatingTableDelegate* ratingDelegate = new RatingTableDelegate(this);
    trackTable->setItemDelegateForColumn(COL_RATING, ratingDelegate);
    connect(ratingDelegate, &RatingTableDelegate::ratingChanged,
            this, &MainWindow::onRatingChangedInTable);

    trackTable->setStyleSheet(
        "QTableWidget { background: #fff; border: 1px solid #333; "
        "border-radius: 10px; color: #000; font-size: 13px; }"
        "QTableWidget::item:selected { background: #0078d4; color: #fff; }"
        );

    connect(trackTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onHeaderClicked);
}

void MainWindow::setupShortcuts() {
    QShortcut* playPauseShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(playPauseShortcut, &QShortcut::activated, this, &MainWindow::onPlayPauseClicked);

    QShortcut* prevShortcut = new QShortcut(QKeySequence("B"), this);
    connect(prevShortcut, &QShortcut::activated, this, &MainWindow::onPrevClicked);

    QShortcut* nextShortcut = new QShortcut(QKeySequence("N"), this);
    connect(nextShortcut, &QShortcut::activated, this, &MainWindow::onNextClicked);

    QShortcut* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, [this]() {
        searchEdit->setFocus();
        searchEdit->selectAll();
    });

    QShortcut* volumeUpShortcut1 = new QShortcut(QKeySequence("Shift+Right"), this);
    QShortcut* volumeUpShortcut2 = new QShortcut(QKeySequence("+"), this);
    connect(volumeUpShortcut1, &QShortcut::activated, controls, &PlayerControls::onVolumeUpClicked);
    connect(volumeUpShortcut2, &QShortcut::activated, controls, &PlayerControls::onVolumeUpClicked);

    QShortcut* volumeDownShortcut1 = new QShortcut(QKeySequence("Shift+Left"), this);
    QShortcut* volumeDownShortcut2 = new QShortcut(QKeySequence("-"), this);
    connect(volumeDownShortcut1, &QShortcut::activated, controls, &PlayerControls::onVolumeDownClicked);
    connect(volumeDownShortcut2, &QShortcut::activated, controls, &PlayerControls::onVolumeDownClicked);

    QShortcut* seekBackShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(seekBackShortcut, &QShortcut::activated, [this]() {
        qint64 newPos = qMax(0LL, player->position() - 5000);
        player->setPosition(newPos);
        controls->setPosition(newPos, player->duration());
    });

    QShortcut* seekForwardShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(seekForwardShortcut, &QShortcut::activated, [this]() {
        qint64 duration = player->duration();
        qint64 newPos = qMin(duration, player->position() + 5000);
        player->setPosition(newPos);
        controls->setPosition(newPos, duration);
    });

    QShortcut* muteShortcut = new QShortcut(QKeySequence("M"), this);
    connect(muteShortcut, &QShortcut::activated, controls, &PlayerControls::onMuteClicked);

    QShortcut* seekStartShortcut = new QShortcut(QKeySequence(Qt::Key_Home), this);
    connect(seekStartShortcut, &QShortcut::activated, [this]() {
        player->setPosition(0);
        controls->setPosition(0, player->duration());
    });

    QShortcut* seekEndShortcut = new QShortcut(QKeySequence(Qt::Key_End), this);
    connect(seekEndShortcut, &QShortcut::activated, [this]() {
        qint64 duration = player->duration();
        player->setPosition(duration - 1000);
        controls->setPosition(duration - 1000, duration);
    });

    QShortcut* repeatShortcut = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(repeatShortcut, &QShortcut::activated, this, &MainWindow::onRepeatClicked);

    QShortcut* shuffleShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(shuffleShortcut, &QShortcut::activated, this, &MainWindow::onShuffleClicked);

    QShortcut* scrollToCurrentShortcut = new QShortcut(QKeySequence("Ctrl+G"), this);
    connect(scrollToCurrentShortcut, &QShortcut::activated, this, &MainWindow::onScrollToCurrentClicked);

    QShortcut* enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), trackTable);
    QShortcut* enterShortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), trackTable);
    connect(enterShortcut, &QShortcut::activated, this, &MainWindow::playSelectedTrack);
    connect(enterShortcut2, &QShortcut::activated, this, &MainWindow::playSelectedTrack);
}

void MainWindow::createMenuBar() {
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    fileMenu = menuBar->addMenu("Файл");
    QAction* openFolderAction = new QAction("📁 Открыть папку с музыкой", this);
    openFolderAction->setShortcut(QKeySequence("Ctrl+O"));
    connect(openFolderAction, &QAction::triggered, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с MP3");
        if (!dir.isEmpty()) scanFolder(dir);
    });
    fileMenu->addAction(openFolderAction);

    fileMenu->addSeparator();

    QAction* exitAction = new QAction("🚪 Выход", this);
    exitAction->setShortcut(QKeySequence("Alt+F4"));
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(exitAction);

    settingsMenu = menuBar->addMenu("Настройки");
    QAction* settingsAction = new QAction("⚙ Настройки", this);
    settingsAction->setShortcut(QKeySequence("Ctrl+P"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    settingsMenu->addAction(settingsAction);

    settingsMenu->addSeparator();

    QAction* autoSkipAction = settingsMenu->addAction("Всегда пропускать повреждённые треки");
    autoSkipAction->setCheckable(true);
    autoSkipAction->setChecked(alwaysSkipBadTracks_);
    connect(autoSkipAction, &QAction::triggered, [this, autoSkipAction]() {
        alwaysSkipBadTracks_ = autoSkipAction->isChecked();
        saveSettings();
        if (settingsDialog) settingsDialog->setAlwaysSkipBadTracks(alwaysSkipBadTracks_);

        QMessageBox::information(this, "Настройки",
                                 alwaysSkipBadTracks_
                                     ? "✅ Теперь повреждённые треки будут пропускаться автоматически"
                                     : "❌ Пропуск повреждённых треков отключен");
    });

    helpMenu = menuBar->addMenu("Справка");
    QAction* hotkeysAction = new QAction("⌨ Горячие клавиши", this);
    hotkeysAction->setShortcut(QKeySequence("F1"));
    connect(hotkeysAction, &QAction::triggered, this, &MainWindow::showHotkeysDialog);
    helpMenu->addAction(hotkeysAction);

    QAction* helpAction = new QAction("❓ Помощь", this);
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelpDialog);
    helpMenu->addAction(helpAction);

    helpMenu->addSeparator();

    QAction* aboutAction = new QAction("ℹ О программе", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    helpMenu->addAction(aboutAction);
}

void MainWindow::scanFolder(const QString& path) {
    savedShuffleState_ = controls->isShuffleEnabled();
    savedRepeatMode_ = static_cast<Playlist::RepeatMode>(controls->getRepeatState());

    playlist.clear();
    originalTracks_.clear();
    pendingFiles_.clear();

    trackTable->setUpdatesEnabled(false);
    trackTable->setSortingEnabled(false);
    trackTable->clearContents();
    trackTable->setRowCount(0);

    if (scannerThread->isRunning()) {
        scannerThread->stop();
        scannerThread->wait();
    }

    albumLabel->setText("Сканирование...");
    artistLabel->setText("");

    scannerThread->setScanPath(path);
    scannerThread->start();
}

void MainWindow::onTrackFound(const Track& track) {
    int row = trackTable->rowCount();
    trackTable->insertRow(row);

    playlist.add(track);
    originalTracks_.push_back(track);

    QTableWidgetItem* titleItem = new QTableWidgetItem(track.qTitle());
    titleItem->setData(Qt::UserRole, QString::fromStdString(track.path()));
    titleItem->setData(Qt::UserRole + 1, track.qTitle());
    trackTable->setItem(row, COL_TITLE, titleItem);

    QTableWidgetItem* artistItem = new QTableWidgetItem(track.qArtist());
    artistItem->setData(Qt::UserRole, QString::fromStdString(track.path()));
    trackTable->setItem(row, COL_ARTIST, artistItem);

    trackTable->setItem(row, COL_GENRE, new QTableWidgetItem(track.qGenre()));
    trackTable->setItem(row, COL_ALBUM, new QTableWidgetItem(track.qAlbum()));

    QTableWidgetItem* ratingItem = new QTableWidgetItem("☆☆☆☆☆");
    ratingItem->setTextAlignment(Qt::AlignCenter);
    ratingItem->setData(Qt::UserRole, QString::fromStdString(track.path()));
    ratingItem->setData(Qt::UserRole + 2, 0.0);
    trackTable->setItem(row, COL_RATING, ratingItem);

    trackTable->setItem(row, COL_YEAR, new QTableWidgetItem(track.qYear()));

    pendingFiles_.append(QString::fromStdString(track.path()));
}

void MainWindow::onScanProgress(int current, int total, const QString& currentFile) {
    albumLabel->setText(QString("Загрузка: %1/%2").arg(current).arg(total));
}

void MainWindow::onScanFinished(int totalTracks) {
    trackTable->setUpdatesEnabled(true);
    trackTable->setSortingEnabled(true);

    playlist.loadRatings();

    if (playlist.size() > 0) {
        playlist.setCurrent(0);
        playlist.setRepeatMode(savedRepeatMode_);
        playlist.setShuffle(savedShuffleState_);
        controls->setRepeatState(static_cast<int>(savedRepeatMode_));
        controls->setShuffleState(savedShuffleState_);
        updateUI();
    }

    isAlphabeticalSort_ = false;
    isReverseSort_ = false;
    updateSortButtonsStyle();

    albumLabel->setText(QString("Загружено треков: %1").arg(totalTracks));

    QTimer::singleShot(100, [this]() {
        for (int i = 0; i < pendingFiles_.size() && i < static_cast<int>(playlist.size()); ++i) {
            TrackMetadata metadata = FastMetadataReader::instance().getMetadata(pendingFiles_[i]);

            Track& track = const_cast<Track&>(playlist.all()[i]);
            bool updated = false;

            if (!metadata.title.isEmpty() && metadata.title != track.qTitle()) {
                track.setTitle(metadata.title.toStdString());
                updated = true;
            }
            if (!metadata.artist.isEmpty() && metadata.artist != track.qArtist()) {
                track.setArtist(metadata.artist.toStdString());
                updated = true;
            }
            if (!metadata.album.isEmpty() && metadata.album != track.qAlbum()) {
                track.setAlbum(metadata.album.toStdString());
                updated = true;
            }
            if (!metadata.genre.isEmpty()) {
                track.setGenre(metadata.genre.toStdString());
                updated = true;
            }
            if (!metadata.year.isEmpty()) {
                track.setYear(metadata.year.toStdString());
                updated = true;
            }

            if (updated) {
                if (trackTable->item(i, COL_TITLE))
                    trackTable->item(i, COL_TITLE)->setText(track.qTitle());
                if (trackTable->item(i, COL_ARTIST))
                    trackTable->item(i, COL_ARTIST)->setText(track.qArtist());
                if (trackTable->item(i, COL_ALBUM))
                    trackTable->item(i, COL_ALBUM)->setText(track.qAlbum());
                if (trackTable->item(i, COL_GENRE))
                    trackTable->item(i, COL_GENRE)->setText(track.qGenre());
                if (trackTable->item(i, COL_YEAR))
                    trackTable->item(i, COL_YEAR)->setText(track.qYear());
            }
        }
        updateUI();
    });
}

void MainWindow::playSelectedTrack() {
    QList<QTableWidgetItem*> selectedItems = trackTable->selectedItems();
    if (selectedItems.isEmpty()) return;

    int row = trackTable->row(selectedItems.first());
    if (row < 0 || row >= static_cast<int>(playlist.size())) return;

    if (playlist.setCurrent(row, true)) {
        auto current = playlist.current();
        if (!current) return;

        QString filePath = QString::fromStdString(current->path());

        if (!validateTrack(filePath)) {
            if (alwaysSkipBadTracks_) {
                if (!navigateAutoSkip(true)) {
                    player->stop();
                    controls->setPlaying(false);
                }
            } else {
                showBadTrackDialog(filePath, true);
            }
            return;
        }

        player->setSource(QUrl::fromLocalFile(filePath));
        player->play();
        controls->setPlaying(true);
        updateThumbnailButtons();
        updateUI();
        highlightCurrentTrack();
    }
}

void MainWindow::onTableDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    if (row >= 0 && row < static_cast<int>(playlist.size())) {
        playlist.setCurrent(row, true);
        playCurrentTrack();
    }
}

void MainWindow::onRatingChanged(int rating) {
    playlist.setCurrentTrackRating(static_cast<double>(rating));
    updateUI();
    int currentRow = static_cast<int>(playlist.currentIndex());
    updateTrackTableRow(currentRow);
}

void MainWindow::onRatingChangedInTable(int row, int rating) {
    if (row < 0 || row >= static_cast<int>(playlist.size())) return;

    playlist.setCurrentTrackRating(static_cast<double>(rating));
    updateUI();
    updateTrackTableRow(row);
}

void MainWindow::onHeaderClicked(int column) {
    if (column == COL_RATING) {
        trackTable->sortByColumn(column, trackTable->horizontalHeader()->sortIndicatorOrder());
    }
}

void MainWindow::playCurrentTrack() {
    auto current = playlist.current();
    if (!current) return;

    QString filePath = QString::fromStdString(current->path());

    if (!validateTrack(filePath)) {
        if (alwaysSkipBadTracks_) {
            if (!navigateAutoSkip(true)) {
                player->stop();
                controls->setPlaying(false);
            }
        } else {
            showBadTrackDialog(filePath, true);
        }
        return;
    }

    player->setSource(QUrl::fromLocalFile(filePath));
    player->play();
    controls->setPlaying(true);
    updateThumbnailButtons();
    updateUI();
    highlightCurrentTrack();
}

void MainWindow::restartCurrentTrack() {
    player->setPosition(0);
    player->play();
}

void MainWindow::onPlayPauseClicked() {
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
        controls->setPlaying(false);
    } else {
        if (player->source().isEmpty() && !playlist.all().empty()) {
            playCurrentTrack();
        } else {
            player->play();
            controls->setPlaying(true);
        }
    }
    updateThumbnailButtons();
}

void MainWindow::onNextClicked() {
    navigateWithSkip(true);
}

void MainWindow::onPrevClicked() {
    if (playlist.shouldRestartTrack(player->position())) {
        restartCurrentTrack();
        return;
    }
    navigateWithSkip(false);
}

bool MainWindow::navigateWithSkip(bool forward) {
    lastWasForward_ = forward;

    if (playlist.size() == 0) return false;

    if (alwaysSkipBadTracks_) {
        return navigateAutoSkip(forward);
    } else {
        bool success = forward ? playlist.next() : playlist.prev(0, true);
        if (!success) return false;

        auto current = playlist.current();
        if (!current) return false;

        QString filePath = QString::fromStdString(current->path());

        if (validateTrack(filePath)) {
            player->setSource(QUrl::fromLocalFile(filePath));
            player->play();
            controls->setPlaying(true);
            updateUI();
            highlightCurrentTrack();
            return true;
        } else {
            showBadTrackDialog(filePath, forward);
            return false;
        }
    }
}

bool MainWindow::navigateAutoSkip(bool forward) {
    size_t startIndex = playlist.currentIndex();
    int attempts = 0;
    const int maxAttempts = playlist.size() * 2;

    while (attempts < maxAttempts) {
        bool success = forward ? playlist.next() : playlist.prev(0, true);
        if (!success) return false;

        auto current = playlist.current();
        if (!current) return false;

        QString filePath = QString::fromStdString(current->path());

        if (validateTrack(filePath)) {
            player->setSource(QUrl::fromLocalFile(filePath));
            player->play();
            controls->setPlaying(true);
            updateUI();
            highlightCurrentTrack();
            return true;
        }

        attempts++;
        if (playlist.currentIndex() == startIndex) break;
    }
    return false;
}

void MainWindow::showBadTrackDialog(const QString& filePath, bool wasForward) {
    BadTrackDialog dialog(this);
    dialog.setTrackInfo(filePath, "Трек поврежден или недоступен");

    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.skipAlways()) {
            alwaysSkipBadTracks_ = true;
            saveSettings();
            updateMenuBar();
            if (settingsDialog) settingsDialog->setAlwaysSkipBadTracks(true);
        }

        if (navigateAutoSkip(wasForward)) return;
    }

    player->stop();
    controls->setPlaying(false);
}

bool MainWindow::validateTrack(const QString& filePath) {
    if (!trackValidator) trackValidator = new TrackValidator(this);
    return trackValidator->validateTrack(filePath);
}

void MainWindow::handleInvalidTrack(const QString& filePath, const QString& error) {
    Q_UNUSED(error);

    if (alwaysSkipBadTracks_) {
        if (navigateAutoSkip(lastWasForward_)) return;
        player->stop();
        controls->setPlaying(false);
    } else {
        showBadTrackDialog(filePath, lastWasForward_);
    }
}

void MainWindow::onRepeatClicked() {
    Playlist::RepeatMode currentMode = playlist.repeatMode();
    Playlist::RepeatMode newMode;

    switch (currentMode) {
    case Playlist::RepeatMode::None:
        newMode = Playlist::RepeatMode::One;
        break;
    case Playlist::RepeatMode::One:
        newMode = Playlist::RepeatMode::None;
        break;
    default:
        newMode = Playlist::RepeatMode::None;
    }

    playlist.setRepeatMode(newMode);
    controls->setRepeatState(static_cast<int>(newMode));
    savedRepeatMode_ = newMode;
}

void MainWindow::onShuffleClicked() {
    bool newState = !playlist.isShuffled();
    playlist.setShuffle(newState);
    controls->setShuffleState(newState);
    savedShuffleState_ = newState;
}

void MainWindow::onSeek(qint64 position) {
    player->setPosition(position);
}

void MainWindow::onVolumeChanged(int volume) {
    audioOutput->setVolume(volume / 100.0);
    volumeBeforeMute_ = volume;
}

void MainWindow::onPositionChanged(qint64 position) {
    controls->setPosition(position, player->duration());
}

void MainWindow::onDurationChanged(qint64 duration) {
    controls->setPosition(player->position(), duration);
}

void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        if (playlist.repeatMode() == Playlist::RepeatMode::One) {
            player->setPosition(0);
            player->play();
            return;
        }

        if (!playlist.next()) {
            player->stop();
            controls->setPlaying(false);
        } else {
            auto current = playlist.current();
            if (current) {
                QString filePath = QString::fromStdString(current->path());

                if (!validateTrack(filePath)) {
                    if (alwaysSkipBadTracks_) {
                        if (!navigateAutoSkip(true)) {
                            player->stop();
                            controls->setPlaying(false);
                        }
                    } else {
                        showBadTrackDialog(filePath, true);
                    }
                    return;
                }

                player->setSource(QUrl::fromLocalFile(filePath));
                player->play();
                controls->setPlaying(true);
                updateUI();
                highlightCurrentTrack();
            }
        }
    }
    updateThumbnailButtons();
}

void MainWindow::onMuteToggled(bool muted) {
    if (muted) {
        audioOutput->setVolume(0);
    } else {
        audioOutput->setVolume(volumeBeforeMute_ / 100.0);
    }
}

void MainWindow::onSearchTextChanged(const QString& text) {
    trackTable->setSortingEnabled(false);

    for (int i = 0; i < trackTable->rowCount(); ++i) {
        QTableWidgetItem* titleItem = trackTable->item(i, COL_TITLE);
        if (!titleItem) continue;

        QString original = titleItem->data(Qt::UserRole + 1).toString();
        if (original.isEmpty()) {
            original = titleItem->text();
            titleItem->setData(Qt::UserRole + 1, original);
        }

        bool shouldShow = text.isEmpty();
        if (!shouldShow) {
            shouldShow = original.contains(text, Qt::CaseInsensitive);
            if (!shouldShow) {
                for (int col = 1; col < COL_COUNT; ++col) {
                    if (col == COL_RATING) continue;
                    QTableWidgetItem* item = trackTable->item(i, col);
                    if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                        shouldShow = true;
                        break;
                    }
                }
            }
        }

        trackTable->setRowHidden(i, !shouldShow);

        if (shouldShow && !text.isEmpty()) {
            titleItem->setText(simpleHighlight(original, text));
        } else if (shouldShow) {
            titleItem->setText(original);
        }
    }

    trackTable->setSortingEnabled(true);
    highlightCurrentTrack();
}

QString MainWindow::simpleHighlight(const QString& text, const QString& searchText) const {
    if (searchText.isEmpty() || text.isEmpty()) return text;

    QString result;
    QString remaining = text;
    QString searchLower = searchText.toLower();

    while (!remaining.isEmpty()) {
        int foundIndex = remaining.toLower().indexOf(searchLower);
        if (foundIndex == -1) {
            result += remaining;
            break;
        }

        result += remaining.left(foundIndex);
        QString found = remaining.mid(foundIndex, searchText.length());
        result += QString("<span style='background-color:#5ac3ff;color:black;font-weight:bold;'>%1</span>").arg(found);
        remaining = remaining.mid(foundIndex + searchText.length());
    }
    return result;
}

void MainWindow::onSortAlphabeticalClicked() {
    if (originalTracks_.empty()) return;

    std::vector<Track> sortedTracks = originalTracks_;

    if (!isAlphabeticalSort_) {
        std::sort(sortedTracks.begin(), sortedTracks.end(),
                  [](const Track& a, const Track& b) {
                      QString artistA = QString::fromStdString(a.artist());
                      QString artistB = QString::fromStdString(b.artist());
                      QString titleA = QString::fromStdString(a.title());
                      QString titleB = QString::fromStdString(b.title());

                      if (artistA != artistB) return artistA.toLower() < artistB.toLower();
                      return titleA.toLower() < titleB.toLower();
                  });
        isAlphabeticalSort_ = true;
        isReverseSort_ = false;
    } else {
        std::sort(sortedTracks.begin(), sortedTracks.end(),
                  [](const Track& a, const Track& b) {
                      QString artistA = QString::fromStdString(a.artist());
                      QString artistB = QString::fromStdString(b.artist());
                      QString titleA = QString::fromStdString(a.title());
                      QString titleB = QString::fromStdString(b.title());

                      if (artistA != artistB) return artistA.toLower() > artistB.toLower();
                      return titleA.toLower() > titleB.toLower();
                  });
        isAlphabeticalSort_ = false;
        isReverseSort_ = true;
    }

    applySorting(sortedTracks, isAlphabeticalSort_ ? "А-Я" : "Я-А");
}

void MainWindow::onSortStandardClicked() {
    if (originalTracks_.empty()) return;
    applySorting(originalTracks_, "Стандарт");
    isAlphabeticalSort_ = false;
    isReverseSort_ = false;
    updateSortButtonsStyle();
}

void MainWindow::onSortReverseClicked() {
    if (originalTracks_.empty()) return;

    std::vector<Track> reversedTracks = originalTracks_;
    std::reverse(reversedTracks.begin(), reversedTracks.end());

    applySorting(reversedTracks, "Реверс");
    isAlphabeticalSort_ = false;
    isReverseSort_ = true;
}

void MainWindow::applySorting(const std::vector<Track>& tracks, const QString& sortName) {
    auto currentTrack = playlist.current();
    std::string currentPath = currentTrack ? currentTrack->path() : "";

    playlist.clear();
    trackTable->setRowCount(0);

    for (size_t i = 0; i < tracks.size(); ++i) {
        const Track& track = tracks[i];
        playlist.add(track);

        int row = trackTable->rowCount();
        trackTable->insertRow(row);

        QTableWidgetItem* titleItem = new QTableWidgetItem(track.qTitle());
        titleItem->setData(Qt::UserRole, QVariant::fromValue(i));
        titleItem->setData(Qt::UserRole + 1, track.qTitle());
        trackTable->setItem(row, COL_TITLE, titleItem);

        trackTable->setItem(row, COL_ARTIST, new QTableWidgetItem(track.qArtist()));
        trackTable->setItem(row, COL_GENRE, new QTableWidgetItem(track.qGenre()));
        trackTable->setItem(row, COL_ALBUM, new QTableWidgetItem(track.qAlbum()));

        QString ratingText;
        int fullStars = static_cast<int>(track.rating());
        for (int s = 0; s < 5; ++s) ratingText += (s < fullStars) ? "★" : "☆";
        QTableWidgetItem* ratingItem = new QTableWidgetItem(ratingText);
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setData(Qt::UserRole, QVariant::fromValue(i));
        ratingItem->setData(Qt::UserRole + 2, track.rating());
        trackTable->setItem(row, COL_RATING, ratingItem);

        trackTable->setItem(row, COL_YEAR, new QTableWidgetItem(track.qYear()));

        if (track.path() == currentPath) playlist.setCurrent(i);
    }

    trackTable->scrollToTop();
    updateUI();
    onSearchTextChanged(searchEdit->text());
    updateSortButtonsStyle();
}

void MainWindow::updateSortButtonsStyle() {
    QString activeStyle = "QPushButton { background: #0078d4; border: 1px solid #0078d4; "
                          "border-radius: 8px; color: #fff; font-size: 12px; }";
    QString inactiveStyle = "QPushButton { background: #333; border: 1px solid #444; "
                            "border-radius: 8px; color: #fff; font-size: 12px; }"
                            "QPushButton:hover { background: #444; }";

    sortAlphabeticalBtn->setStyleSheet(isAlphabeticalSort_ ? activeStyle : inactiveStyle);
    sortStandardBtn->setStyleSheet(!isAlphabeticalSort_ && !isReverseSort_ ? activeStyle : inactiveStyle);
    sortReverseBtn->setStyleSheet(isReverseSort_ ? activeStyle : inactiveStyle);

    if (isAlphabeticalSort_) {
        sortAlphabeticalBtn->setToolTip("Сортировка А-Я (нажмите для Я-А)");
    } else {
        sortAlphabeticalBtn->setToolTip("Сортировка по алфавиту");
    }
}

void MainWindow::updateUI() {
    auto current = playlist.current();
    if (!current) {
        albumLabel->setText("Выберите папку с музыкой");
        artistLabel->setText("");
        return;
    }

    QImage coverImage = current->getCoverImage();
    if (!coverImage.isNull()) {
        QPixmap coverPixmap = QPixmap::fromImage(coverImage)
        .scaled(coverLabel->width(), coverLabel->height(),
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
        coverLabel->setPixmap(coverPixmap);
        coverLabel->setText("");
    } else {
        QPixmap coverPixmap(coverLabel->width(), coverLabel->height());
        coverPixmap.fill(Qt::darkGray);
        coverLabel->setPixmap(coverPixmap);
        coverLabel->setText("No Cover");
    }

    albumLabel->setText(current->qTitle());
    artistLabel->setText(current->qArtist());

    double rating = current->rating();
    for (int i = 0; i < 5; ++i) {
        starButtons[i]->setText(i < rating ? "★" : "☆");
    }

    int currentRow = static_cast<int>(playlist.currentIndex());
    updateTrackTableRow(currentRow);
    highlightCurrentTrack();
}

void MainWindow::updateTrackTableRow(int row) {
    if (row < 0 || row >= trackTable->rowCount()) return;
    if (row >= static_cast<int>(playlist.size())) return;

    const Track& track = playlist.all()[row];
    QString ratingText;
    int fullStars = static_cast<int>(track.rating());
    for (int s = 0; s < 5; ++s) ratingText += (s < fullStars) ? "★" : "☆";

    QTableWidgetItem* ratingItem = trackTable->item(row, COL_RATING);
    if (ratingItem) {
        ratingItem->setText(ratingText);
        ratingItem->setData(Qt::UserRole + 2, track.rating());
    }
}

void MainWindow::highlightCurrentTrack() {
    int currentRow = static_cast<int>(playlist.currentIndex());
    trackTable->clearSelection();

    if (currentRow >= 0 && currentRow < trackTable->rowCount()) {
        trackTable->selectRow(currentRow);

        QRect itemRect = trackTable->visualItemRect(trackTable->item(currentRow, 0));
        QRect viewportRect = trackTable->viewport()->rect();

        if (!viewportRect.contains(itemRect)) {
            trackTable->scrollToItem(trackTable->item(currentRow, 0),
                                     QAbstractItemView::EnsureVisible);
        }
    }
}

void MainWindow::onScrollToCurrentClicked() {
    int currentRow = static_cast<int>(playlist.currentIndex());
    if (currentRow >= 0 && currentRow < trackTable->rowCount()) {
        QTableWidgetItem* item = trackTable->item(currentRow, 0);
        if (item) {
            trackTable->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            trackTable->clearSelection();
            trackTable->selectRow(currentRow);
        }
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == trackTable && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            playSelectedTrack();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showSettingsDialog() {
    settingsDialog->setAlwaysSkipBadTracks(alwaysSkipBadTracks_);
    settingsDialog->setDefaultVolume(volumeBeforeMute_);

    if (settingsDialog->exec() == QDialog::Accepted) {
        bool newSkip = settingsDialog->alwaysSkipBadTracks();
        int newVol = settingsDialog->defaultVolume();

        if (alwaysSkipBadTracks_ != newSkip) {
            alwaysSkipBadTracks_ = newSkip;
            updateMenuBar();
            QMessageBox::information(this, "Настройки",
                                     alwaysSkipBadTracks_
                                         ? "✅ Автопропуск повреждённых треков включен"
                                         : "❌ Автопропуск отключен");
        }

        volumeBeforeMute_ = newVol;
        audioOutput->setVolume(volumeBeforeMute_ / 100.0);
        controls->setVolume(volumeBeforeMute_);
        saveSettings();
    }
}

void MainWindow::showHelpDialog() {
    QString helpText = R"(
<b>AlexMusic - Музыкальный плеер</b><br><br>
<b>Основные возможности:</b><br>
• Воспроизведение MP3 файлов<br>
• Управление плейлистами<br>
• Поиск и сортировка треков<br>
• Рейтинг треков (звездочки)<br>
• Поддержка обложек альбомов<br><br>
<b>Горячие клавиши:</b><br>
Пробел — Play/Pause<br>
B/N — Предыдущий/Следующий<br>
←/→ — Перемотка на 5 сек<br>
M — Вкл/выкл звук<br>
Ctrl+F — Поиск
)";
    QMessageBox::information(this, "Справка", helpText);
}

void MainWindow::showHotkeysDialog() {
    QString hotkeys = R"(
<b>Горячие клавиши:</b><br><br>
<b>Воспроизведение:</b><br>
• Пробел — Play/Pause<br>
• B — Предыдущий трек<br>
• N — Следующий трек<br>
• Ctrl+R — Повтор трека<br>
• Ctrl+S — Случайный порядок<br><br>
<b>Громкость:</b><br>
• M — Вкл/выкл звук<br>
• +/- — Изменить громкость<br><br>
<b>Навигация:</b><br>
• ←/→ — -5/+5 секунд<br>
• Home/End — Начало/конец<br>
• Ctrl+G — К текущему треку
)";
    QMessageBox::information(this, "Горячие клавиши", hotkeys);
}

void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "О программе",
                       "<b>AlexMusic v2.0</b><br><br>"
                       "Простой и удобный музыкальный плеер для Windows.<br>"
                       "Поддержка MP3, управление плейлистами, рейтинги,<br>"
                       "поиск и сортировка.<br><br>"
                       "С улучшенной производительностью и кэшированием."
                       );
}

void MainWindow::saveSettings() {
    QSettings settings("AlexMusic", "Player");
    settings.setValue("shuffleState", savedShuffleState_);
    settings.setValue("repeatMode", static_cast<int>(savedRepeatMode_));
    settings.setValue("alwaysSkipBadTracks", alwaysSkipBadTracks_);
    settings.setValue("volumeBeforeMute", volumeBeforeMute_);
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::loadSettings() {
    QSettings settings("AlexMusic", "Player");
    savedShuffleState_ = settings.value("shuffleState", false).toBool();
    savedRepeatMode_ = static_cast<Playlist::RepeatMode>(settings.value("repeatMode", 0).toInt());
    alwaysSkipBadTracks_ = settings.value("alwaysSkipBadTracks", false).toBool();
    volumeBeforeMute_ = settings.value("volumeBeforeMute", 70).toInt();

    if (settings.contains("windowGeometry")) restoreGeometry(settings.value("windowGeometry").toByteArray());
    if (settings.contains("windowState")) restoreState(settings.value("windowState").toByteArray());

    audioOutput->setVolume(volumeBeforeMute_ / 100.0);
    controls->setVolume(volumeBeforeMute_);

    if (settingsDialog) {
        settingsDialog->setAlwaysSkipBadTracks(alwaysSkipBadTracks_);
        settingsDialog->setDefaultVolume(volumeBeforeMute_);
    }
}

void MainWindow::updateMenuBar() {
    QList<QAction*> actions = settingsMenu->actions();
    for (QAction* action : actions) {
        if (action->text() == "Всегда пропускать повреждённые треки") {
            action->setChecked(alwaysSkipBadTracks_);
            break;
        }
    }
}

#ifdef Q_OS_WIN
HICON MainWindow::createIconFromText(const wchar_t* text, int size) {
    HDC hdc = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
    SelectObject(hdcMem, hBitmap);

    RECT rect = {0, 0, size, size};
    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);

    HFONT hFont = CreateFont(size-4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
    SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(0, 0, 0));
    SetBkMode(hdcMem, TRANSPARENT);
    DrawText(hdcMem, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    HBITMAP hMask = CreateBitmap(size, size, 1, 1, nullptr);
    ICONINFO iconInfo = {TRUE, hMask, hBitmap};
    HICON hIcon = CreateIconIndirect(&iconInfo);

    DeleteObject(hFont);
    DeleteObject(hBitmap);
    DeleteObject(hMask);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdc);
    return hIcon;
}

HICON MainWindow::createPlayIcon() { return createIconFromText(L"▶", 24); }
HICON MainWindow::createPauseIcon() { return createIconFromText(L"⏸", 24); }
HICON MainWindow::createNextIcon() { return createIconFromText(L"⏭", 24); }
HICON MainWindow::createPrevIcon() { return createIconFromText(L"⏮", 24); }
#endif

void MainWindow::setupThumbnailToolBar() {
#ifdef Q_OS_WIN
    if (thumbnailToolbarInitialized) return;

    playIcon = createPlayIcon();
    pauseIcon = createPauseIcon();
    nextIcon = createNextIcon();
    prevIcon = createPrevIcon();

    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITaskbarList3, &taskbarList);

    if (SUCCEEDED(hr)) {
        ITaskbarList3* pTaskbarList = (ITaskbarList3*)taskbarList;
        hr = pTaskbarList->HrInit();

        if (SUCCEEDED(hr)) {
            THUMBBUTTON thumbButtons[3];

            thumbButtons[0].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[0].iId = 0;
            thumbButtons[0].hIcon = prevIcon;
            wcscpy(thumbButtons[0].szTip, L"Предыдущий");
            thumbButtons[0].dwFlags = THBF_ENABLED;

            thumbButtons[1].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[1].iId = 1;
            thumbButtons[1].hIcon = (player->playbackState() == QMediaPlayer::PlayingState) ? pauseIcon : playIcon;
            wcscpy(thumbButtons[1].szTip, L"Воспроизведение/Пауза");
            thumbButtons[1].dwFlags = THBF_ENABLED;

            thumbButtons[2].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[2].iId = 2;
            thumbButtons[2].hIcon = nextIcon;
            wcscpy(thumbButtons[2].szTip, L"Следующий");
            thumbButtons[2].dwFlags = THBF_ENABLED;

            hr = pTaskbarList->ThumbBarAddButtons((HWND)winId(), 3, thumbButtons);
            if (SUCCEEDED(hr)) thumbnailToolbarInitialized = true;
        }
    }
#endif
}

void MainWindow::updateThumbnailButtons() {
#ifdef Q_OS_WIN
    if (!thumbnailToolbarInitialized || !taskbarList) return;

    ITaskbarList3* pTaskbarList = (ITaskbarList3*)taskbarList;
    THUMBBUTTON thumbButton;
    thumbButton.dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
    thumbButton.iId = 1;

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        thumbButton.hIcon = pauseIcon;
        wcscpy(thumbButton.szTip, L"Пауза");
    } else {
        thumbButton.hIcon = playIcon;
        wcscpy(thumbButton.szTip, L"Воспроизведение");
    }
    thumbButton.dwFlags = THBF_ENABLED;
    pTaskbarList->ThumbBarUpdateButtons((HWND)winId(), 1, &thumbButton);
#endif
}

void MainWindow::cleanupThumbnailToolBar() {
#ifdef Q_OS_WIN
    if (playIcon) { DestroyIcon(playIcon); playIcon = nullptr; }
    if (pauseIcon) { DestroyIcon(pauseIcon); pauseIcon = nullptr; }
    if (nextIcon) { DestroyIcon(nextIcon); nextIcon = nullptr; }
    if (prevIcon) { DestroyIcon(prevIcon); prevIcon = nullptr; }
    if (taskbarList) { ((ITaskbarList3*)taskbarList)->Release(); taskbarList = nullptr; }
    thumbnailToolbarInitialized = false;
#endif
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_COMMAND) {
            int buttonId = LOWORD(msg->wParam);
            switch (buttonId) {
            case 0: onPrevClicked(); return true;
            case 1: onPlayPauseClicked(); return true;
            case 2: onNextClicked(); return true;
            }
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    if (!thumbnailToolbarInitialized) {
        QTimer::singleShot(100, this, &MainWindow::setupThumbnailToolBar);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    scannerThread->stop();
    scannerThread->wait();
    saveSettings();
    cleanupThumbnailToolBar();
    event->accept();
}

MainWindow::~MainWindow() {
    cleanupThumbnailToolBar();
}
