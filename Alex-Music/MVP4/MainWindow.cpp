#include "MainWindow.h"

#include <QVBoxLayout>    // Вертикальная компоновка
#include <QHBoxLayout>    // Горизонтальная компоновка
#include <QFileDialog>    // Диалог выбора файлов/папок
#include <QDirIterator>   // Итератор для обхода директорий
#include <QPixmap>        // Растровое изображение
#include <QPushButton>    // Кнопка
#include <QLineEdit>      // Поле ввода
#include <QDir>           // Работа с директориями
#include <QFileInfo>      // Информация о файле
#include <QButtonGroup>   // Группа кнопок
#include <QCoreApplication> // Основной класс приложения
#include <QTimer>         // Таймер
#include <QScrollBar>     // Полоса прокрутки
#include <QShortcut>      // Горячие клавиши
#include <QMessageBox>
#include <QKeyEvent>

#include "HtmlDelegate.h"
#include "TrackValidator.h"
#include "BadTrackDialog.h"

// Windows API headers (только для Windows)
#ifdef Q_OS_WIN
#include <windows.h>      // Основные заголовки Windows API
#include <commctrl.h>     // Common Controls
#include <shobjidl.h>     // Shell интерфейсы
#endif

// Конструктор главного окна
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AlexMusic");  // Установка заголовока окна

    // Инициализация сохраненных состояний
    savedShuffleState_ = false;
    savedRepeatMode_ = Playlist::RepeatMode::None;

    // Инициализация флагов
    alwaysSkipBadTracks_ = false;
    lastWasForward_ = true;

    trackValidator = new TrackValidator(this);

    // Подключаем сигнал валидатора
    connect(trackValidator, &TrackValidator::validationFailed,
            this, &MainWindow::handleInvalidTrack);


    // Инициализация медиаплеера и аудиовыхода
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);  // Связь плеера с выводом
    audioOutput->setVolume(volumeBeforeMute_ / 100.0);  // Установка начальной громкости

    // Создание центрального виджета (основная область окна)
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);  // Установка как центрального

    // Основная вертикальная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);  // Отступы от краев окна
    mainLayout->setSpacing(20);  // Расстояние между элементами

    // ВЕРХНЯЯ ПАНЕЛЬ С КНОПКОЙ ПАПКИ, ПОИСКОМ И СОРТИРОВКОЙ
    QHBoxLayout* topBar = new QHBoxLayout;

    // Кнопка выбора папки
    QPushButton* folderBtn = new QPushButton("📁 Выбрать папку с музыкой");
    topBar->addWidget(folderBtn);

    // Растягиваемое пространство между кнопкой папки и поиском
    topBar->addStretch();

    // Строка поиска
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Поиск треков...");
    searchEdit->setClearButtonEnabled(true);  // Кнопка очистки в поле
    // searchEdit->setFixedWidth(325);  // Фиксированная ширина
    searchEdit->setStyleSheet(
        "QLineEdit { "
        "width: 100%;"
        "background: #222; "           // Темный фон
        "border: 1px solid #444; "     // Серая рамка
        "border-radius: 15px; "        // Закругленные углы
        "padding: 8px 12px; "          // Внутренние отступы
        "color: #fff; "                // Белый текст
        "font-size: 14px; "            // Размер шрифта
        "}"
        "QLineEdit:focus { "           // Стиль при фокусе
        "border: 2px solid #0078d4; "  // Синяя рамка
        "}"
        );
    topBar->addWidget(searchEdit);

    // Кнопки сортировки
    sortStandardBtn = new QPushButton("Станд");
    sortStandardBtn->setFixedSize(50, 35);
    sortStandardBtn->setToolTip("Стандартный порядок (исходная последовательность файлов)");

    // Добавляем кнопки сортировки в верхнюю панель
    topBar->addWidget(sortStandardBtn);

    settingsBtn = new QPushButton("⚙");
    settingsBtn->setFixedSize(35, 35);
    settingsBtn->setToolTip("Настройки");
    topBar->addWidget(settingsBtn);

    // // Подключаем сигнал:
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::showSettingsDialog);

    // // Инициализируем диалог (в конструкторе после setupShortcuts):
    settingsDialog = new SettingsDialog(this);

    // Добавляем верхнюю панель в основную компоновку
    mainLayout->addLayout(topBar);

    // ОСНОВНОЙ КОНТЕНТ - горизонтальная компоновка
    QHBoxLayout* contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(30);  // Расстояние между левой и правой панелью

    // ЛЕВАЯ ПАНЕЛЬ - ОБЛОЖКА И ИНФОРМАЦИЯ О ТРЕКЕ
    QWidget* leftPanel = new QWidget;
    leftPanel->setFixedWidth(400);  // Фиксированная ширина
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);  // Расстояние между элементами

    // Метка для отображения обложки альбома
    coverLabel = new QLabel;
    coverLabel->setFixedSize(250, 250);  // Фиксированный размер
    coverLabel->setStyleSheet("QLabel { background: #222; border: 2px solid #444; border-radius: 10px; color: #fff; }");
    coverLabel->setAlignment(Qt::AlignCenter);  // Выравнивание по центру
    coverLabel->setText("No Cover");  // Текст по умолчанию
    leftLayout->addWidget(coverLabel, 0, Qt::AlignCenter);  // Добавляем по центру

    // Метка для названия альбома/трека
    albumLabel = new QLabel("Выберите папку с музыкой");
    albumLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #000; }");
    albumLabel->setWordWrap(true);  // Перенос длинного текста
    albumLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);  // Можно выделять текст
    albumLabel->setCursor(Qt::IBeamCursor);  // Курсор I-образный для текста

    // Метка для имени исполнителя
    artistLabel = new QLabel();
    artistLabel->setStyleSheet("QLabel { font-size: 16px; color: #000; }");
    artistLabel->setWordWrap(true);
    artistLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    artistLabel->setCursor(Qt::IBeamCursor);

    // ПАНЕЛЬ РЕЙТИНГА - 5 звезд
    QWidget* ratingWidget = new QWidget;
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setSpacing(5);  // Расстояние между звездами
    ratingLayout->setAlignment(Qt::AlignLeft);  // Выравнивание по левому краю

    // Создаем 5 кнопок-звезд
    for (int i = 0; i < 5; ++i) {
        starButtons[i] = new QPushButton("☆");  // Пустая звезда
        starButtons[i]->setFixedSize(30, 30);   // Фиксированный размер
        starButtons[i]->setStyleSheet(
            "QPushButton {"
            "    background: #333;"      // Темный фон
            "    border: 1px solid #555;" // Серая рамка
            "    border-radius: 15px;"   // Круглая кнопка
            "    color: #ffcc00;"        // Желтый цвет звезд
            "    font-size: 16px;"       // Размер символа
            "}"
            "QPushButton:hover {"        // Стиль при наведении
            "    background: #444;"
            "}"
            );
        ratingLayout->addWidget(starButtons[i]);  // Добавляем в компоновку

        // Подключение обработчика клика для каждой звезды
        // Использование лямбда-функции для захвата индекса звезды
        connect(starButtons[i], &QPushButton::clicked, [this, i]() {
            onRatingChanged(i + 1);  // рейтинг от 1 до 5
        });
    }

    ratingLayout->addStretch();  // Растягивающееся пространство справа

    // Добавляем все элементы в левую панель
    leftLayout->addWidget(albumLabel);
    leftLayout->addWidget(artistLabel);
    leftLayout->addWidget(ratingWidget);

    // Добавляем левую панель в основную компоновку контента
    contentLayout->addWidget(leftPanel);

    // Устанавливаем кастомный делегат для HTML
    HtmlDelegate* delegate = new HtmlDelegate(this);

    // ПРАВАЯ ПАНЕЛЬ - ТАБЛИЦА ТРЕКОВ
    trackTable = new QTableWidget;
    setupTrackTable();  // Настраиваем таблицу

    trackTable->setStyleSheet(
        "QTableWidget { "
        "background: #fff; "
        "border: 1px solid #333; "
        "border-radius: 10px; "
        "color: #000; "
        "font-size: 13px; "
        "gridline-color: #eee; "
        "}"
        "QTableWidget::item { "
        "padding: 4px; "
        "border-bottom: 1px solid #f0f0f0; "
        "}"
        "QTableWidget::item:selected { "
        "background: #0078d4; "
        "color: #fff; "
        "border: none; "
        "}"
        "QHeaderView::section { "
        "background: #f0f0f0; "
        "padding: 8px; "
        "border: 1px solid #ddd; "
        "font-weight: bold; "
        "color: #333; "
        "}"
        "QHeaderView::section:hover { "
        "background: #e0e0e0; "
        "cursor: pointer; "
        "}"
        );

    contentLayout->addWidget(trackTable, 1);

    // Подключаем обработчик клика по заголовку
    connect(trackTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onTableHeaderClicked);

    // Подключаем двойной клик по строке
    connect(trackTable, &QTableWidget::cellDoubleClicked,
            this, [this](int row, int column) {
                Q_UNUSED(column);
                playTrackFromTable(row);
            });

    trackTable->installEventFilter(this);

    // Добавляем компоновку контента в основную
    mainLayout->addLayout(contentLayout, 1);  // Растягиваем контент

    // ПАНЕЛЬ УПРАВЛЕНИЯ (внизу окна)
    controls = new PlayerControls;
    controls->setStyleSheet(
        "PlayerControls { background: #111; border: 1px solid #333; border-radius: 10px; }"
        "QPushButton { background: #333; color: #fff; border: 1px solid #444; border-radius: 8px; padding: 8px; font-size: 16px; }"
        "QPushButton:hover { background: #444; }"      // Светлее при наведении
        "QPushButton:pressed { background: #555; }"    // Еще светлее при нажатии
        );
    mainLayout->addWidget(controls);  // Добавляем внизу

    // Устанавливаем общий стиль для главного окна
    setStyleSheet(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #80A6FF, stop:1 #f0fff0); }"  // Градиентный фон от темного к очень темному
        "QMainWindow::title { background: transparent; }"  // Прозрачный заголовок
        );

    resize(1000, 700);  // Начальный размер окна

    // ПОДКЛЮЧЕНИЕ СИГНАЛОВ К СЛОТАМ

    // Обработчик кнопки выбора папки
    connect(folderBtn, &QPushButton::clicked, [this]() {
        // Открываем диалог выбора папки
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с MP3");
        if (!dir.isEmpty()) {
            scanFolder(dir);  // Сканируем если папка выбрана
            // if (shuffled) {

            // }
        }
    });

    // Подключаем сигналы от элементов интерфейса к слотам
    connect(controls, &PlayerControls::playPauseClicked, this, &MainWindow::onPlayPauseClicked);
    connect(controls, &PlayerControls::nextClicked, this, &MainWindow::onNextClicked);
    connect(controls, &PlayerControls::prevClicked, this, &MainWindow::onPrevClicked);
    connect(controls, &PlayerControls::seek, this, &MainWindow::onSeek);
    connect(controls, &PlayerControls::volumeChanged, this, &MainWindow::onVolumeChanged);
    connect(controls, &PlayerControls::repeatClicked, this, &MainWindow::onRepeatClicked);
    connect(controls, &PlayerControls::shuffleClicked, this, &MainWindow::onShuffleClicked);
    connect(controls, &PlayerControls::muteToggled, this, &MainWindow::onMuteToggled);

    // Подключаем сигналы медиаплеера
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onMediaStatusChanged);

    // Подключаем сигналы поиска и сортировки
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(sortStandardBtn, &QPushButton::clicked, this, &MainWindow::onSortStandardClicked);

    // Автоматически сканируем папку Music если она существует
    QString defaultFolder = "C:\\Users\\User\\Music";
    if (QDir(defaultFolder).exists()) {
        scanFolder(defaultFolder);
    }

    // Инициализируем переменные для thumbnail toolbar
    thumbnailToolbarInitialized = false;
    taskbarList = nullptr;

    setupShortcuts();  // Настраиваем горячие клавиши
    loadSettings(); // Загружаем сохранённые настройки
}

// Настройка горячих клавиш приложения
void MainWindow::setupShortcuts() {
    // 1. Пауза/старт трека - пробел
    QShortcut* playPauseShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(playPauseShortcut, &QShortcut::activated, this, &MainWindow::onPlayPauseClicked);

    // 2. Трек назад - B
    QShortcut* prevShortcut = new QShortcut(QKeySequence("B"), this);
    connect(prevShortcut, &QShortcut::activated, this, &MainWindow::onPrevClicked);

    // 3. Трек вперед - N
    QShortcut* nextShortcut = new QShortcut(QKeySequence("N"), this);
    connect(nextShortcut, &QShortcut::activated, this, &MainWindow::onNextClicked);

    // 4. Поиск - Ctrl+F
    QShortcut* searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(searchShortcut, &QShortcut::activated, [this]() {
        searchEdit->setFocus();    // Устанавливаем фокус на поле поиска
        searchEdit->selectAll();   // Выделяем весь текст для удобства
    });

    // 5. Прибавить громкость - Shift+Right или +
    QShortcut* volumeUpShortcut1 = new QShortcut(QKeySequence("Shift+Right"), this);
    QShortcut* volumeUpShortcut2 = new QShortcut(QKeySequence("+"), this);
    connect(volumeUpShortcut1, &QShortcut::activated, this, [this]() {
        controls->onVolumeUpClicked();  // Увеличиваем громкость
    });
    connect(volumeUpShortcut2, &QShortcut::activated, this, [this]() {
        controls->onVolumeUpClicked();
    });

    // 6. Убавить громкость - Shift+Left или -
    QShortcut* volumeDownShortcut1 = new QShortcut(QKeySequence("Shift+Left"), this);
    QShortcut* volumeDownShortcut2 = new QShortcut(QKeySequence("-"), this);
    connect(volumeDownShortcut1, &QShortcut::activated, this, [this]() {
        controls->onVolumeDownClicked();  // Уменьшаем громкость
    });
    connect(volumeDownShortcut2, &QShortcut::activated, this, [this]() {
        controls->onVolumeDownClicked();
    });

    // 7. Перематывание назад - Left (на 5 секунд)
    QShortcut* seekBackShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(seekBackShortcut, &QShortcut::activated, this, [this]() {
        qint64 currentPosition = player->position();
        qint64 newPosition = qMax(0LL, currentPosition - 5000); // 5 секунд назад
        player->setPosition(newPosition);
        controls->setPosition(newPosition, player->duration()); // Обновляем UI
    });

    // 8. Перематывание вперед - Right (на 5 секунд)
    QShortcut* seekForwardShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(seekForwardShortcut, &QShortcut::activated, this, [this]() {
        qint64 currentPosition = player->position();
        qint64 duration = player->duration();
        qint64 newPosition = qMin(duration, currentPosition + 5000); // 5 секунд вперед
        player->setPosition(newPosition);
        controls->setPosition(newPosition, duration);
    });

    // 9. Выключить звук - M (M(ute))
    QShortcut* muteShortcut = new QShortcut(QKeySequence("M"), this);
    connect(muteShortcut, &QShortcut::activated, this, [this]() {
        controls->onMuteClicked();  // Переключаем звук
    });

    // 10. (Fn) Home - перемотка в начало трека
    QShortcut* seekStartShortcut = new QShortcut(QKeySequence(Qt::Key_Home), this);
    connect(seekStartShortcut, &QShortcut::activated, this, [this]() {
        player->setPosition(0);  // В начало трека
        controls->setPosition(0, player->duration());
    });

    // 11. (Fn) End - перемотка в конец трека
    QShortcut* seekEndShortcut = new QShortcut(QKeySequence(Qt::Key_End), this);
    connect(seekEndShortcut, &QShortcut::activated, this, [this]() {
        qint64 duration = player->duration();
        player->setPosition(duration - 1000); // За 1 секунду до конца
        controls->setPosition(duration - 1000, duration);
    });

    // 12. Включение редима "Повтор трека"
    QShortcut* onRepeatClicked = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(onRepeatClicked, &QShortcut::activated,
            this, &MainWindow::onRepeatClicked);

    // 13. Включение редима "Случайный порядок"
    QShortcut* onShuffleClicked = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(onShuffleClicked, &QShortcut::activated,
            this, &MainWindow::onShuffleClicked);

    // 14. Переход к текущему треку - Ctrl+G
    QShortcut* scrollToCurrentShortcut = new QShortcut(QKeySequence("Ctrl+G"), this);
    connect(scrollToCurrentShortcut, &QShortcut::activated,
            this, &MainWindow::onScrollToCurrentClicked);

    // 15. Enter/Return для воспроизведения выделенного трека в таблице
    if (trackTable) {
        QShortcut* tableEnterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), trackTable);
        connect(tableEnterShortcut, &QShortcut::activated, this, &MainWindow::playSelectedTrack);

        QShortcut* tableEnterShortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), trackTable);
        connect(tableEnterShortcut2, &QShortcut::activated, this, &MainWindow::playSelectedTrack);
    }
}

// Метод воспроизведения выделенного трека
void MainWindow::playSelectedTrack() {
    QList<QTableWidgetItem*> selectedItems = trackTable->selectedItems();

    if (selectedItems.isEmpty()) {
        qDebug() << "Нет выделенного трека";
        return;
    }

    // Берем первую выделенную строку
    int row = selectedItems.first()->row();
    playTrackFromTable(row);
}

// Сканирование папки и добавление MP3 файлов в плейлист
void MainWindow::scanFolder(const QString& path) {
    // Сохраняем текущие состояния перед очисткой
    savedShuffleState_ = controls->isShuffleEnabled();
    savedRepeatMode_ = static_cast<Playlist::RepeatMode>(controls->getRepeatState());

    playlist.clear();
    trackTable->clearContents();
    trackTable->setRowCount(0);
    originalTracks_.clear();

    connect(trackTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onTrackTableDoubleClicked);


    // Сканирование файлов...
    QDirIterator it(path, {"*.mp3"}, QDir::Files, QDirIterator::Subdirectories);
    int index = 1;

    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.baseName();
        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);
        QString artist = parts.value(0, "Unknown Artist");
        QString title = parts.value(1, baseName);

        // Создаем трек с базовой информацией
        Track track(filePath.toStdString(), artist.toStdString(),
                    title.toStdString(), "", 0.0);

        // Загружаем метаданные из файла
        track.loadMetadata();

        playlist.add(track);
        originalTracks_.push_back(track);
    }

    playlist.loadRatings();

    if (!playlist.all().empty()) {
        playlist.setCurrent(0);

        // Восстанавливаем сохраненные режимы
        playlist.setRepeatMode(savedRepeatMode_);
        playlist.setShuffle(savedShuffleState_);

        // Обновляем кнопки управления
        controls->setRepeatState(static_cast<int>(savedRepeatMode_));
        controls->setShuffleState(savedShuffleState_);

        // Заполняем таблицу
        populateTrackTable();
        updateUI();
    }

    loadSettings();
}

// Метод проверки трека (добавьте после других методов)
bool MainWindow::validateTrack(const QString& filePath) {
    if (!trackValidator) {
        // Создаем валидатор если его нет
        trackValidator = new TrackValidator(this);
    }

    return trackValidator->validateTrack(filePath);
}

// Обработчик битого трека
void MainWindow::handleInvalidTrack(const QString& filePath, const QString& error) {
    Q_UNUSED(filePath);
    Q_UNUSED(error);

    // Показываем диалог с опцией "Всегда пропускать"
    BadTrackDialog dialog(this);
    dialog.setTrackInfo(filePath, error);

    if (dialog.exec() == QDialog::Accepted) {
        alwaysSkipBadTracks_ = dialog.skipAlways();

        // Автоматически ищем следующий валидный трек в том же направлении
        if (!navigateWithSkip(lastWasForward_)) {
            player->stop();
            controls->setPlaying(false);
        }
    } else {
        // Если пользователь отменил - останавливаем воспроизведение
        player->stop();
        controls->setPlaying(false);
    }
}

// Обработчик изменения рейтинга
void MainWindow::onRatingChanged(int rating) {
    // Устанавливаем рейтинг текущему треку и сохраняем в файл
    playlist.setCurrentTrackRating(static_cast<double>(rating));
    updateUI();  // Обновляем отображение звезд
}

// Воспроизведение текущего трека
void MainWindow::playCurrentTrack() {
    auto current = playlist.current();
    if (!current) return;

    QString filePath = QString::fromStdString(current->path());

    // Всегда проверяем трек перед воспроизведением
    if (!validateTrack(filePath)) {
        // Если трек битый - показываем диалог
        showBadTrackDialog(filePath, true);
        return;
    }

    player->setSource(QUrl::fromLocalFile(filePath));
    player->play();
    controls->setPlaying(true);
    updateThumbnailButtons();
    updateUI();
    highlightCurrentTrackInTable();
}

// Перезапуск текущего трека (с начала)
void MainWindow::restartCurrentTrack() {
    player->setPosition(0);  // Перематываем в начало
    player->play();          // Запускаем воспроизведение
}

// Обновление пользовательского интерфейса
void MainWindow::updateUI() {
    auto current = playlist.current();  // Получаем текущий трек
    if (!current) return;  // Если трека нет - выходим

    // Получаем обложку трека
    QImage coverImage = current->getCoverImage();

    if (!coverImage.isNull()) {
        // Масштабируем обложку под размер метки с сохранением пропорций
        QPixmap coverPixmap = QPixmap::fromImage(coverImage)
                                  .scaled(coverLabel->width(), coverLabel->height(),
                                          Qt::KeepAspectRatio, Qt::SmoothTransformation);
        coverLabel->setPixmap(coverPixmap);  // Устанавливаем обложку
        coverLabel->setText("");             // Убираем текст "No Cover"
    } else {
        // Если обложки нет - создаем серый квадрат
        QPixmap coverPixmap(coverLabel->width(), coverLabel->height());
        coverPixmap.fill(Qt::darkGray);  // Заливаем темно-серым
        coverLabel->setPixmap(coverPixmap);
        coverLabel->setText("No Cover");  // Текст "No Cover"
        coverLabel->setStyleSheet("QLabel { background: #222; border: 2px solid #444; border-radius: 10px; color: #fff; font-size: 12px; }");
    }

    // Устанавливаем информацию о треке
    albumLabel->setText(QString::fromStdString(current->title()));
    artistLabel->setText(QString::fromStdString(current->artist()));

    // Обновляем отображение звезд рейтинга
    double rating = current->rating();
    for (int i = 0; i < 5; ++i) {
        if (i < rating) {
            starButtons[i]->setText("★");  // Заполненная звезда
        } else {
            starButtons[i]->setText("☆");  // Пустая звезда
        }
    }

    // Подсвечиваем текущий трек в таблице
    highlightCurrentTrackInTable();
}

// Обработчик кнопки Play/Pause
void MainWindow::onPlayPauseClicked() {
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();               // Если играет - ставим на паузу
        controls->setPlaying(false);   // Меняем иконку на "play"
    } else {
        // Если не играет
        if (player->source().isEmpty() && !playlist.all().empty()) {
            // Если источник не установлен но есть треки - играем текущий
            playCurrentTrack();
        } else {
            player->play();            // Продолжаем воспроизведение
            controls->setPlaying(true); // Меняем иконку на "pause"
        }
    }
    updateThumbnailButtons();  // Обновляем кнопки в thumbnail toolbar
}

// Обработчик кнопки "Следующий трек"
void MainWindow::onNextClicked() {
    if (playlist.shouldRestartTrack(player->position())) {
        restartCurrentTrack();
        return;
    }

    navigateWithSkip(true);
}

// Обработчик кнопки "Предыдущий трек"
void MainWindow::onPrevClicked() {
    // Сначала проверяем правило 3 секунд
    if (playlist.shouldRestartTrack(player->position())) {
        restartCurrentTrack();
        return;
    }

    navigateWithSkip(false);
}

void MainWindow::onRepeatClicked() {
    Playlist::RepeatMode currentMode = playlist.repeatMode();  // Текущий режим
    Playlist::RepeatMode newMode;

    // Циклическое переключение режимов: None -> One -> All -> None
    switch (currentMode) {
    case Playlist::RepeatMode::None:
        newMode = Playlist::RepeatMode::One;
        break;
    case Playlist::RepeatMode::One:
        // newMode = Playlist::RepeatMode::All; // не нужно ???
        newMode = Playlist::RepeatMode::None;
        break;
        // case Playlist::RepeatMode::All: // не нужно ???
        //     newMode = Playlist::RepeatMode::None;
        //     break;
    }

    playlist.setRepeatMode(newMode);  // Устанавливаем новый режим
    controls->setRepeatState(static_cast<int>(newMode));  // Обновляем UI

    // Сохраняем состояние для будущих папок
    savedRepeatMode_ = newMode;
}

// Обработчик кнопки перемешивания
void MainWindow::onShuffleClicked() {
    bool newShuffleState = !playlist.isShuffled();  // Инвертируем состояние
    playlist.setShuffle(newShuffleState);           // Устанавливаем
    controls->setShuffleState(newShuffleState);     // Обновляем UI

    // Сохраняем состояние для будущих папок
    savedShuffleState_ = newShuffleState;
}

// Обработчик перемотки трека
void MainWindow::onSeek(qint64 position) {
    player->setPosition(position);  // Устанавливаем новую позицию
}

// Обработчик изменения громкости
void MainWindow::onVolumeChanged(int volume) {
    audioOutput->setVolume(volume / 100.0);  // Устанавливаем громкость (0.0 - 1.0)
    volumeBeforeMute_ = volume;              // Сохраняем для восстановления
}

// Обработчик изменения позиции воспроизведения
void MainWindow::onPositionChanged(qint64 position) {
    // Обновляем позицию в элементах управления
    controls->setPosition(position, player->duration());
}

// Обработчик изменения длительности трека
void MainWindow::onDurationChanged(qint64 duration) {
    // Обновляем позицию в элементах управления
    controls->setPosition(player->position(), duration);
}

// Обработчик изменения статуса медиа
void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {  // Если трек закончился
        if (playlist.repeatMode() == Playlist::RepeatMode::One) {
            // Режим повтора одного трека - перезапускаем текущий
            player->setPosition(0);
            player->play();
            return;
        }

        // Безопасный Автоматический переход к следующему треку
        if (!playlist.next()) {
            // Если следующий трек недоступен
            player->stop();
            controls->setPlaying(false);
        } else {
            playCurrentTrack();
        }
    }

    updateThumbnailButtons();  // Обновляем кнопки в thumbnail toolbar
}

// Обработчик двойного клика по треку в списке
void MainWindow::onTrackTableDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    playTrackFromTable(row);
}

// Обработчик включения/выключения звука
void MainWindow::onMuteToggled(bool muted) {
    if (muted) {
        audioOutput->setVolume(0);  // Выключаем звук
    } else {
        // Включаем звук с сохраненной громкостью
        audioOutput->setVolume(volumeBeforeMute_ / 100.0);
    }
}

// Обработчик изменения текста поиска
QString MainWindow::simpleHighlight(const QString& text, const QString& searchText) const {
    if (searchText.isEmpty() || text.isEmpty()) {
        return text;
    }

    QString result;
    QString remaining = text;
    QString searchLower = searchText.toLower();

    while (!remaining.isEmpty()) {
        // Ищем вхождение (регистронезависимо)
        int foundIndex = remaining.toLower().indexOf(searchLower);

        if (foundIndex == -1) {
            // Не нашли больше вхождений
            result += remaining;
            break;
        }

        // Добавляем часть до найденного текста
        result += remaining.left(foundIndex);

        // Добавляем найденный текст с подсветкой
        QString found = remaining.mid(foundIndex, searchText.length());
        result += QString("<span style='background-color:#5ac3ff;color:black;font-weight:bold;'>%1</span>")
                      .arg(found);

        // Продолжаем с оставшегося текста
        remaining = remaining.mid(foundIndex + searchText.length());
    }

    return result;
}

// -----------------------------------------------------------------
// ПРОСТОЙ ОБРАБОТЧИК ПОИСКА
// -----------------------------------------------------------------

void MainWindow::onSearchTextChanged(const QString& text) {
    // Временно отключаем сортировку для фильтрации
    bool wasSortingEnabled = trackTable->isSortingEnabled();
    trackTable->setSortingEnabled(false);

    for (int row = 0; row < trackTable->rowCount(); ++row) {
        bool shouldShow = text.isEmpty();

        if (!shouldShow) {
            // Проверяем все текстовые колонки на совпадение
            for (int col = 1; col <= 5; ++col) { // Колонки 1-5: текстовая информация
                QTableWidgetItem* item = trackTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    shouldShow = true;
                    break;
                }
            }
        }

        // Показываем/скрываем строку
        trackTable->setRowHidden(row, !shouldShow);
    }

    // Восстанавливаем состояние сортировки
    trackTable->setSortingEnabled(wasSortingEnabled);

    // Подсвечиваем текущий трек
    highlightCurrentTrackInTable();
}

// Деструктор главного окна - вызывается при уничтожении объекта MainWindow
MainWindow::~MainWindow() {
    cleanupThumbnailToolBar();  // Очищаем ресурсы thumbnail toolbar при закрытии приложения
}

// Метод инициализации thumbnail toolbar (панель предпросмотра в Windows)
void MainWindow::setupThumbnailToolBar() {
#ifdef Q_OS_WIN  // Этот код компилируется только на Windows
    if (thumbnailToolbarInitialized) return;  // Если уже инициализирован - выходим

    // Создаем иконки для кнопок toolbar используя Windows API
    playIcon = createPlayIcon();      // для кнопки Play
    pauseIcon = createPauseIcon();    // для кнопки Pause
    nextIcon = createNextIcon();      // для кнопки Next
    prevIcon = createPrevIcon();      // для кнопки Previous

    // Создаем COM объект ITaskbarList3 для работы с панелью задач Windows
    // CLSID_TaskbarList - идентификатор класса TaskbarList
    // NULL - нет агрегирования
    // CLSCTX_INPROC_SERVER - сервер в процессе
    // IID_ITaskbarList3 - идентификатор интерфейса ITaskbarList3
    // &taskbarList - указатель для сохранения созданного объекта
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITaskbarList3, &taskbarList);

    // Проверяем успешность создания COM объекта
    if (SUCCEEDED(hr)) {
        // Приводим указатель к правильному типу ITaskbarList3*
        ITaskbarList3* pTaskbarList = (ITaskbarList3*)taskbarList;
        // Инициализируем COM объект
        hr = pTaskbarList->HrInit();

        // Проверяем успешность инициализации
        if (SUCCEEDED(hr)) {
            // Создаем массив из 3 кнопок для thumbnail toolbar
            THUMBBUTTON thumbButtons[3];

            // Левая кнопка: ⏮ Предыдущий трек
            thumbButtons[0].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;  // Указываем какие поля структуры используем
            thumbButtons[0].iId = 0;              // Уникальный идентификатор кнопки
            thumbButtons[0].hIcon = prevIcon;     // Дескриптор иконки
            wcscpy(thumbButtons[0].szTip, L"Предыдущий");  // Текст подсказки (wide char)
            thumbButtons[0].dwFlags = THBF_ENABLED;  // Флаги - кнопка активна

            // Центральная кнопка: ▶/⏸ Play/Pause
            thumbButtons[1].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[1].iId = 1;
            // Динамически выбираем иконку в зависимости от состояния воспроизведения
            thumbButtons[1].hIcon = (player->playbackState() == QMediaPlayer::PlayingState) ? pauseIcon : playIcon;
            wcscpy(thumbButtons[1].szTip, L"Воспроизведение/Пауза");
            thumbButtons[1].dwFlags = THBF_ENABLED;

            // Правая кнопка: ⏭ Следующий трек
            thumbButtons[2].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[2].iId = 2;
            thumbButtons[2].hIcon = nextIcon;
            wcscpy(thumbButtons[2].szTip, L"Следующий");
            thumbButtons[2].dwFlags = THBF_ENABLED;

            // Добавляем кнопки в thumbnail toolbar окна
            // (HWND)winId() - получаем handle окна Windows из QWidget
            // 3 - количество кнопок
            // thumbButtons - массив кнопок
            hr = pTaskbarList->ThumbBarAddButtons((HWND)winId(), 3, thumbButtons);

            // Если кнопки успешно добавлены - устанавливаем флаг инициализации
            if (SUCCEEDED(hr)) {
                thumbnailToolbarInitialized = true;  // Помечаем что toolbar инициализирован
            }
        }
    }
#endif  // Конец блока #ifdef Q_OS_WIN
}

// Метод обновления состояния кнопок thumbnail toolbar
void MainWindow::updateThumbnailButtons() {
#ifdef Q_OS_WIN  // Только для Windows
    // Проверяем что toolbar инициализирован и COM объект существует
    if (!thumbnailToolbarInitialized || !taskbarList) return;

    // Приводим указатель к правильному типу
    ITaskbarList3* pTaskbarList = (ITaskbarList3*)taskbarList;

    // Создаем структуру для обновления кнопки
    THUMBBUTTON thumbButton;
    thumbButton.dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;  // Обновляем иконку, подсказку и флаги
    thumbButton.iId = 1;  // ID кнопки Play/Pause (центральная кнопка)

    // В зависимости от состояния воспроизведения обновляем иконку и подсказку
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        thumbButton.hIcon = pauseIcon;              // Устанавливаем иконку паузы
        wcscpy(thumbButton.szTip, L"Пауза");        // Обновляем подсказку
    } else {
        thumbButton.hIcon = playIcon;               // Устанавливаем иконку воспроизведения
        wcscpy(thumbButton.szTip, L"Воспроизведение");  // Обновляем подсказку
    }
    thumbButton.dwFlags = THBF_ENABLED;  // Кнопка активна

    // Обновляем только одну кнопку (Play/Pause) в toolbar
    // (HWND)winId() - handle окна
    // 1 - количество обновляемых кнопок
    // &thumbButton - указатель на структуру с данными кнопки
    pTaskbarList->ThumbBarUpdateButtons((HWND)winId(), 1, &thumbButton);
#endif  // Конец блока #ifdef Q_OS_WIN
}

// Метод очистки ресурсов thumbnail toolbar
void MainWindow::cleanupThumbnailToolBar() {
#ifdef Q_OS_WIN  // Только для Windows
    // Уничтожаем созданные иконки и освобождаем ресурсы
    if (playIcon) {
        DestroyIcon(playIcon);    // Уничтожаем иконку Play
        playIcon = nullptr;       // Обнуляем указатель
    }
    if (pauseIcon) {
        DestroyIcon(pauseIcon);   // Уничтожаем иконку Pause
        pauseIcon = nullptr;      // Обнуляем указатель
    }
    if (nextIcon) {
        DestroyIcon(nextIcon);    // Уничтожаем иконку Next
        nextIcon = nullptr;       // Обнуляем указатель
    }
    if (prevIcon) {
        DestroyIcon(prevIcon);    // Уничтожаем иконку Previous
        prevIcon = nullptr;       // Обнуляем указатель
    }

    // Освобождаем COM объект
    if (taskbarList) {
        ((ITaskbarList3*)taskbarList)->Release();  // Вызываем Release() для освобождения COM объекта
        taskbarList = nullptr;     // Обнуляем указатель
    }
    thumbnailToolbarInitialized = false;  // Сбрасываем флаг инициализации
#endif  // Конец блока #ifdef Q_OS_WIN
}

// Обработчик нативных событий Windows (переопределенный метод QWidget)
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN  // Только для Windows
    // Проверяем тип события - должно быть Windows сообщение
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        // Приводим сообщение к структуре MSG Windows API
        MSG* msg = static_cast<MSG*>(message);

        // Проверяем что это команда (нажатие кнопки)
        if (msg->message == WM_COMMAND) {
            // Извлекаем ID кнопки из параметра сообщения
            int buttonId = LOWORD(msg->wParam);

            // Обрабатываем нажатие в зависимости от ID кнопки
            switch (buttonId) {
            case 0: // Previous button
                onPrevClicked();    // Вызываем обработчик предыдущего трека
                return true;        // Сообщаем что событие обработано
            case 1: // Play/Pause button
                onPlayPauseClicked();  // Вызываем обработчик воспроизведения/паузы
                return true;           // Сообщаем что событие обработано
            case 2: // Next button
                onNextClicked();    // Вызываем обработчик следующего трека
                return true;        // Сообщаем что событие обработано
            }
        }
    }
#endif
    // Если событие не обработано - передаем его базовому классу
    return QMainWindow::nativeEvent(eventType, message, result);
}

// Обработчик события показа окна (переопределенный метод QWidget)
void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);  // Вызываем реализацию базового класса

    // Инициализируем thumbnail toolbar после показа окна
    // Используем таймер чтобы дать окну полностью отобразиться
    if (!thumbnailToolbarInitialized) {
        QTimer::singleShot(100, this, &MainWindow::setupThumbnailToolBar);  // Задержка 100 мс
    }
}

#ifdef Q_OS_WIN  // Следующие методы только для Windows
// Метод создания иконки из текстового символа
HICON MainWindow::createIconFromText(const wchar_t* text, int size) {
    // Получаем device context (контекст устройства) для экрана
    HDC hdc = GetDC(nullptr);
    // Создаем совместимый memory device context для рисования
    HDC hdcMem = CreateCompatibleDC(hdc);

    // Создаем bitmap для иконки заданного размера
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
    // Выбираем bitmap в memory DC для рисования
    SelectObject(hdcMem, hBitmap);

    // Настраиваем фон иконки
    RECT rect = {0, 0, size, size};  // Прямоугольник размером с иконку
    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240)); // Создаем светло-серую кисть
    FillRect(hdcMem, &rect, hBrush);  // Заливаем прямоугольник цветом
    DeleteObject(hBrush);             // Удаляем кисть

    // Настраиваем шрифт для текста
    HFONT hFont = CreateFont(
        size - 4,    // Высота шрифта (немного меньше размера иконки)
        0,           // Ширина (0 = auto)
        0,           // Angle of escapement
        0,           // Orientation angle
        FW_NORMAL,   // Weight (нормальный)
        FALSE,       // Italic
        FALSE,       // Underline
        FALSE,       // Strikeout
        DEFAULT_CHARSET,  // Character set
        OUT_DEFAULT_PRECIS,  // Output precision
        CLIP_DEFAULT_PRECIS, // Clipping precision
        DEFAULT_QUALITY,     // Quality
        DEFAULT_PITCH,       // Pitch and family
        L"Segoe UI Symbol"   // Имя шрифта (содержит символы эмодзи)
        );
    SelectObject(hdcMem, hFont);  // Выбираем шрифт в DC

    // Настраиваем цвет текста
    SetTextColor(hdcMem, RGB(0, 0, 0));  // Черный текст
    SetBkMode(hdcMem, TRANSPARENT);       // Прозрачный фон текста

    // Рисуем текст по центру прямоугольника
    DrawText(hdcMem, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Создаем маску для иконки (1-bit bitmap)
    HBITMAP hMask = CreateBitmap(size, size, 1, 1, nullptr);

    // Создаем структуру для информации об иконке
    ICONINFO iconInfo;
    iconInfo.fIcon = TRUE;        // Это иконка (не курсор)
    iconInfo.hbmMask = hMask;     // Маска иконки
    iconInfo.hbmColor = hBitmap;  // Цветной bitmap
    // Создаем иконку из информации
    HICON hIcon = CreateIconIndirect(&iconInfo);

    // Очищаем ресурсы
    DeleteObject(hFont);    // Удаляем шрифт
    DeleteObject(hBitmap);  // Удаляем bitmap
    DeleteObject(hMask);    // Удаляем маску
    DeleteDC(hdcMem);       // Удаляем memory DC
    ReleaseDC(nullptr, hdc); // Освобождаем screen DC

    return hIcon;  // Возвращаем созданную иконку
}

// Создание иконки Play
HICON MainWindow::createPlayIcon() {
    return createIconFromText(L"▶", 24);  // Символ Play, размер 16x16
}

// Создание иконки Pause
HICON MainWindow::createPauseIcon() {
    return createIconFromText(L"⏸", 24);  // Символ Pause, размер 16x16
}

// Создание иконки Next
HICON MainWindow::createNextIcon() {
    return createIconFromText(L"⏭", 24);  // Символ Next, размер 16x16
}

// Создание иконки Previous
HICON MainWindow::createPrevIcon() {
    return createIconFromText(L"⏮", 24);  // Символ Previous, размер 16x16
}
#endif  // Конец блока #ifdef Q_OS_WIN

// Обработчик кнопки прокрутки к текущему треку
void MainWindow::onScrollToCurrentClicked() {
    int currentRowInPlaylist = static_cast<int>(playlist.currentIndex());

    if (currentRowInPlaylist < 0 || !trackTable) return;

    // Ищем строку с текущим треком в таблице
    for (int row = 0; row < trackTable->rowCount(); ++row) {
        QTableWidgetItem* item = trackTable->item(row, 0);
        if (item) {
            int originalIndex = item->data(Qt::UserRole).toInt();
            if (originalIndex == currentRowInPlaylist) {
                // Выделяем строку
                trackTable->selectRow(row);

                // Прокручиваем к видимой области
                QTableWidgetItem* firstItem = trackTable->item(row, 0);
                if (firstItem) {
                    trackTable->scrollToItem(firstItem, QAbstractItemView::PositionAtCenter);
                }
                break;
            }
        }
    }
}


// Основной метод навигации с пропуском битых треков
bool MainWindow::navigateWithSkip(bool forward) {
    lastWasForward_ = forward;

    if (playlist.size() == 0) return false;

    // Если установлен флаг "всегда пропускать"
    if (alwaysSkipBadTracks_) {
        return navigateAutoSkip(forward);
    }

    // Иначе используем обычную навигацию с диалогом
    return navigateWithDialog(forward);
}

// Навигация с автоматическим пропуском битых треков
bool MainWindow::navigateAutoSkip(bool forward) {
    size_t startIndex = playlist.currentIndex();
    int attempts = 0;
    const int maxAttempts = playlist.size();

    while (attempts < maxAttempts) {
        // Пытаемся перейти
        bool navigationSuccess;
        if (forward) {
            navigationSuccess = playlist.next();
        } else {
            navigationSuccess = playlist.prev(0, true);
        }

        if (!navigationSuccess) {
            return false;
        }

        auto current = playlist.current();
        if (!current) {
            return false;
        }

        QString filePath = QString::fromStdString(current->path());

        // Если трек валиден - воспроизводим
        if (validateTrack(filePath)) {
            player->setSource(QUrl::fromLocalFile(filePath));
            player->play();
            controls->setPlaying(true);
            updateUI();
            highlightCurrentTrackInTable();
            return true;
        }

        // Трек битый - логируем и продолжаем поиск
        qDebug() << "Автоматически пропускаем битый трек:" << filePath;
        attempts++;

        // Защита от цикла
        if (playlist.currentIndex() == startIndex) {
            qDebug() << "Вернулись к началу, все треки битые";
            playlist.setCurrent(startIndex);
            return false;
        }
    }

    return false;
}

// Навигация с показом диалога для битых треков
bool MainWindow::navigateWithDialog(bool forward) {
    // Пытаемся перейти один раз
    bool navigationSuccess;
    if (forward) {
        navigationSuccess = playlist.next();
    } else {
        navigationSuccess = playlist.prev(0, true);
    }

    if (!navigationSuccess) {
        return false;
    }

    auto current = playlist.current();
    if (!current) {
        return false;
    }

    QString filePath = QString::fromStdString(current->path());

    // Проверяем трек
    if (validateTrack(filePath)) {
        // Трек валиден - воспроизводим
        player->setSource(QUrl::fromLocalFile(filePath));
        player->play();
        controls->setPlaying(true);
        updateUI();
        highlightCurrentTrackInTable();
        return true;
    } else {
        // Трек битый - показываем диалог
        showBadTrackDialog(filePath, forward);
        return false; // Диалог сам решит, что делать дальше
    }
}

// Показать диалог для битого трека
void MainWindow::showBadTrackDialog(const QString& filePath, bool wasForward) {
    BadTrackDialog dialog(this);
    dialog.setTrackInfo(filePath, "Трек поврежден или недоступен");

    if (dialog.exec() == QDialog::Accepted) {
        alwaysSkipBadTracks_ = dialog.skipAlways();

        // Если поставили галочку "всегда пропускать"
        if (alwaysSkipBadTracks_) {
            // Автоматически ищем следующий валидный трек
            if (navigateAutoSkip(wasForward)) {
                return;
            }
        } else {
            // Без галочки - просто продолжаем поиск с диалогом
            if (navigateWithDialog(wasForward)) {
                return;
            }
        }
    }

    // Если диалог отменен или не нашли валидный трек
    // Возвращаемся к предыдущему валидному треку
    player->stop();
    controls->setPlaying(false);
}

// Сохранение настроек в файл
void MainWindow::saveSettings() {
    QSettings settings("AlexMusic", "Player");
    settings.setValue("shuffleState", savedShuffleState_);
    settings.setValue("repeatMode", static_cast<int>(savedRepeatMode_));
    settings.setValue("alwaysSkipBadTracks", alwaysSkipBadTracks_);
    settings.setValue("volumeBeforeMute", volumeBeforeMute_);
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

// Загрузка настроек из файла
void MainWindow::loadSettings() {
    QSettings settings("AlexMusic", "Player");
    savedShuffleState_ = settings.value("shuffleState", false).toBool();
    savedRepeatMode_ = static_cast<Playlist::RepeatMode>(
        settings.value("repeatMode", 0).toInt());
    alwaysSkipBadTracks_ = settings.value("alwaysSkipBadTracks", false).toBool();
    volumeBeforeMute_ = settings.value("volumeBeforeMute", 70).toInt();

    // Восстанавливаем геометрию окна
    if (settings.contains("windowGeometry")) {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
    }
    if (settings.contains("windowState")) {
        restoreState(settings.value("windowState").toByteArray());
    }

    // Применяем настройки громкости
    audioOutput->setVolume(volumeBeforeMute_ / 100.0);
    controls->setVolume(volumeBeforeMute_);
}

// Фильтр событий для обработки клавиш
bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    // Обработка для таблицы треков
    if (watched == trackTable && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        // Обработка клавиши Enter/Return
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            playSelectedTrack();
            return true;
        }
    }

    // Для остальных событий используем обработку базового класса
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showSettingsDialog() {
    // Загружаем текущие настройки в диалог
    settingsDialog->setAlwaysSkipBadTracks(alwaysSkipBadTracks_);
    settingsDialog->setDefaultVolume(volumeBeforeMute_);

    if (settingsDialog->exec() == QDialog::Accepted) {
        // Сохраняем новые настройки
        alwaysSkipBadTracks_ = settingsDialog->alwaysSkipBadTracks();
        volumeBeforeMute_ = settingsDialog->defaultVolume();

        // Применяем настройки
        audioOutput->setVolume(volumeBeforeMute_ / 100.0);
        controls->setVolume(volumeBeforeMute_);

        // Сохраняем в файл
        saveSettings();

        qDebug() << "Настройки сохранены: alwaysSkipBadTracks =" << alwaysSkipBadTracks_
                 << ", volume =" << volumeBeforeMute_;
    }
}


void MainWindow::setupTrackTable() {
    // Устанавливаем количество колонок
    trackTable->setColumnCount(7);

    // Устанавливаем заголовки колонок
    QStringList headers;
    headers << "#" << "Название трека" << "Исполнитель" << "Альбом"
            << "Год" << "Жанр" << "Рейтинг";
    trackTable->setHorizontalHeaderLabels(headers);

    // Настройка заголовков
    QHeaderView* header = trackTable->horizontalHeader();
    header->setSectionsClickable(true);
    header->setSortIndicatorShown(true);
    header->setSortIndicator(1, Qt::AscendingOrder); // По умолчанию сортировка по названию

    // Настройка поведения таблицы
    trackTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    trackTable->setSelectionMode(QAbstractItemView::SingleSelection);
    trackTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trackTable->verticalHeader()->setVisible(false);
    trackTable->setAlternatingRowColors(true);

    // Настройка ширины колонок
    trackTable->setColumnWidth(0, 40);   // #
    trackTable->setColumnWidth(1, 200);  // Название
    trackTable->setColumnWidth(2, 150);  // Исполнитель
    trackTable->setColumnWidth(3, 150);  // Альбом
    trackTable->setColumnWidth(4, 60);   // Год
    trackTable->setColumnWidth(5, 100);  // Жанр
    trackTable->setColumnWidth(6, 80);   // Рейтинг

    // Устанавливаем делегат для колонки рейтинга
    RatingDelegate* ratingDelegate = new RatingDelegate(this);
    trackTable->setItemDelegateForColumn(6, ratingDelegate);

    // Включаем сортировку
    trackTable->setSortingEnabled(true);
}

void MainWindow::populateTrackTable() {
    // Отключаем сортировку временно, чтобы сохранить порядок
    trackTable->setSortingEnabled(false);

    trackTable->setRowCount(0);
    originalOrder_.clear();

    const auto& tracks = playlist.all();
    trackTable->setRowCount(static_cast<int>(tracks.size()));

    for (size_t i = 0; i < tracks.size(); ++i) {
        const Track& track = tracks[i];
        originalOrder_.push_back(i);

        // Колонка 0: Порядковый номер
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(i + 1));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setData(Qt::UserRole, static_cast<int>(i)); // Сохраняем оригинальный индекс
        trackTable->setItem(static_cast<int>(i), 0, numItem);

        // Колонка 1: Название трека
        QTableWidgetItem* titleItem = new QTableWidgetItem(
            QString::fromStdString(track.title()));
        titleItem->setToolTip(QString::fromStdString(track.title()));
        trackTable->setItem(static_cast<int>(i), 1, titleItem);

        // Колонка 2: Исполнитель
        QTableWidgetItem* artistItem = new QTableWidgetItem(
            QString::fromStdString(track.artist()));
        artistItem->setToolTip(QString::fromStdString(track.artist()));
        trackTable->setItem(static_cast<int>(i), 2, artistItem);

        // Колонка 3: Альбом
        QString albumText = QString::fromStdString(track.album());
        if (albumText.isEmpty()) albumText = "Неизвестный альбом";
        QTableWidgetItem* albumItem = new QTableWidgetItem(albumText);
        albumItem->setToolTip(albumText);
        trackTable->setItem(static_cast<int>(i), 3, albumItem);

        // Колонка 4: Год
        QString yearText = (track.year() > 0) ? QString::number(track.year()) : "";
        QTableWidgetItem* yearItem = new QTableWidgetItem(yearText);
        yearItem->setTextAlignment(Qt::AlignCenter);
        yearItem->setData(Qt::UserRole, track.year()); // Для сортировки по числу
        trackTable->setItem(static_cast<int>(i), 4, yearItem);

        // Колонка 5: Жанр
        QString genreText = QString::fromStdString(track.genre());
        QTableWidgetItem* genreItem = new QTableWidgetItem(genreText);
        genreItem->setToolTip(genreText);
        trackTable->setItem(static_cast<int>(i), 5, genreItem);

        // Колонка 6: Рейтинг
        QTableWidgetItem* ratingItem = new QTableWidgetItem();
        ratingItem->setData(Qt::DisplayRole, QString()); // Пустой текст, рисуем через делегат
        ratingItem->setData(Qt::UserRole, track.rating()); // Числовое значение для сортировки
        ratingItem->setTextAlignment(Qt::AlignCenter);
        trackTable->setItem(static_cast<int>(i), 6, ratingItem);

        // Сохраняем путь к файлу для быстрого доступа
        QTableWidgetItem* pathItem = new QTableWidgetItem(QString::fromStdString(track.path()));
        trackTable->setItem(static_cast<int>(i), 7, pathItem); // Скрытая колонка
    }

    // Скрываем колонку с путем
    trackTable->setColumnHidden(7, true);

    // Включаем сортировку обратно
    trackTable->setSortingEnabled(true);

    // Сортировка по умолчанию (по названию трека)
    trackTable->sortItems(1, Qt::AscendingOrder);
    currentSortColumn_ = 1;
    currentSortOrder_ = Qt::AscendingOrder;

    // Обновляем индикатор сортировки
    trackTable->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
}

void MainWindow::highlightCurrentTrackInTable() {
    int currentRowInPlaylist = static_cast<int>(playlist.currentIndex());

    // Снимаем выделение со всех строк
    trackTable->clearSelection();

    // Ищем строку с текущим треком в таблице
    for (int row = 0; row < trackTable->rowCount(); ++row) {
        QTableWidgetItem* item = trackTable->item(row, 0); // Колонка с номером
        if (item) {
            // Получаем оригинальный индекс из UserRole
            int originalIndex = item->data(Qt::UserRole).toInt();
            if (originalIndex == currentRowInPlaylist) {
                // Выделяем строку
                trackTable->selectRow(row);

                // Прокручиваем к видимой области если нужно
                QTableWidgetItem* firstItem = trackTable->item(row, 0);
                if (firstItem) {
                    trackTable->scrollToItem(firstItem, QAbstractItemView::EnsureVisible);
                }
                break;
            }
        }
    }
}

void MainWindow::playTrackFromTable(int row) {
    QTableWidgetItem* item = trackTable->item(row, 0); // Колонка с номером
    if (item) {
        // Получаем оригинальный индекс из UserRole
        int trackIndex = item->data(Qt::UserRole).toInt();

        if (playlist.setCurrent(trackIndex, true)) {
            auto current = playlist.current();
            if (current) {
                QString filePath = QString::fromStdString(current->path());

                // Проверяем трек перед воспроизведением
                if (!validateTrack(filePath)) {
                    showBadTrackDialog(filePath, true);
                    return;
                }

                // Воспроизводим
                player->setSource(QUrl::fromLocalFile(filePath));
                player->play();
                controls->setPlaying(true);
                updateThumbnailButtons();
                updateUI();

                qDebug() << "Воспроизводится трек:" << QString::fromStdString(current->title());
            }
        }
    }
}

void MainWindow::onTableHeaderClicked(int column) {
    if (column >= 0 && column < trackTable->columnCount()) {
        Qt::SortOrder newOrder = Qt::AscendingOrder;

        // Если кликнули по той же колонке - меняем порядок сортировки
        if (currentSortColumn_ == column) {
            newOrder = (currentSortOrder_ == Qt::AscendingOrder) ?
                           Qt::DescendingOrder : Qt::AscendingOrder;
        }

        currentSortColumn_ = column;
        currentSortOrder_ = newOrder;

        // Устанавливаем индикатор сортировки
        trackTable->horizontalHeader()->setSortIndicator(column, newOrder);

        // Для числовых колонок устанавливаем специальную сортировку
        if (column == 0 || column == 4 || column == 6) { // #, Год, Рейтинг
            // Для числовых колонок сортируем по числовому значению в UserRole
            trackTable->sortByColumn(column, newOrder);
        } else {
            // Для текстовых колонок стандартная сортировка
            trackTable->sortItems(column, newOrder);
        }

        // Подсвечиваем текущий трек после сортировки
        highlightCurrentTrackInTable();
    }
}

void MainWindow::onSortStandardClicked() {
    if (originalTracks_.empty()) return;

    // Отключаем сортировку
    trackTable->setSortingEnabled(false);

    // Восстанавливаем оригинальный порядок
    QList<QPair<int, int>> rowOrder; // <оригинальный индекс, текущая строка>

    for (int row = 0; row < trackTable->rowCount(); ++row) {
        QTableWidgetItem* item = trackTable->item(row, 0);
        if (item) {
            int originalIndex = item->data(Qt::UserRole).toInt();
            rowOrder.append(qMakePair(originalIndex, row));
        }
    }

    // Сортируем по оригинальному индексу
    std::sort(rowOrder.begin(), rowOrder.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.first < b.first;
              });

    // Перемещаем строки в оригинальном порядке
    for (int i = 0; i < rowOrder.size(); ++i) {
        if (rowOrder[i].second != i) {
            // Вставляем строку на правильную позицию
            trackTable->insertRow(i);

            // Копируем все ячейки
            for (int col = 0; col < trackTable->columnCount(); ++col) {
                QTableWidgetItem* sourceItem = trackTable->item(rowOrder[i].second + 1, col);
                if (sourceItem) {
                    QTableWidgetItem* newItem = sourceItem->clone();
                    trackTable->setItem(i, col, newItem);
                }
            }

            // Удаляем старую строку
            trackTable->removeRow(rowOrder[i].second + 1);
        }
    }

    // Сбрасываем индикатор сортировки
    trackTable->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    currentSortColumn_ = 1;
    currentSortOrder_ = Qt::AscendingOrder;

    // Включаем сортировку обратно
    trackTable->setSortingEnabled(true);

    // Обновляем подсветку
    highlightCurrentTrackInTable();
}

void MainWindow::updateTrackTable() {
    // Сохраняем текущую сортировку
    int sortColumn = trackTable->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder sortOrder = trackTable->horizontalHeader()->sortIndicatorOrder();

    // Заполняем таблицу заново
    populateTrackTable();

    // Восстанавливаем сортировку
    if (sortColumn >= 0) {
        trackTable->sortItems(sortColumn, sortOrder);
    }

    // Подсвечиваем текущий трек
    highlightCurrentTrackInTable();
}

void MainWindow::applyTableSorting(int column, Qt::SortOrder order) {
    // Для числовых колонок используем специальную сортировку
    if (column == 0 || column == 4 || column == 6) {
        // Сортируем по числовому значению в UserRole
        trackTable->sortByColumn(column, order);
    } else {
        // Стандартная текстовая сортировка
        trackTable->sortItems(column, order);
    }
}
