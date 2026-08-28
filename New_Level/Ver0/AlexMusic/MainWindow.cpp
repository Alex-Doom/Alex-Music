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
#include <QMenuBar>
#include <QHeaderView>
#include <QProgressDialog>
#include <QApplication>
#include <QWidget>

#include <QFrame>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QMediaDevices>

#include "HtmlDelegate.h"
#include "TrackValidator.h"
#include "BadTrackDialog.h"
// #include "MainWindow.moc"
#include "FastMetadataReader.h"

// Windows API headers (только для Windows)
#ifdef Q_OS_WIN
#include <windows.h>      // Основные заголовки Windows API
#include <commctrl.h>     // Common Controls
#include <shobjidl.h>     // Shell интерфейсы
#endif

// Конструктор главного окна
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AlexMusic");  // Установка заголовока окна

    setupTrayIcon();

    // Попытка поиска и установки иконки несколькими способами
    // QIcon appIcon;
    // // Список возможных путей к иконке
    // QStringList possiblePaths = {
    //     QCoreApplication::applicationDirPath() + "/app_icon.ico",  // Рядом с exe
    //     // "C:\\My_QT\\CPP\\Alex_Music\\work\\untitled\\icons\\app_icon.ico", // Абсолютный путь
    // };

    // bool iconLoaded = false;
    // // Перебираем все возможные пути
    // for (const QString& path : possiblePaths) {
    //     if (QFile::exists(path)) {  // Проверка, существует ли файл
    //         appIcon = QIcon(path);  // Загрузка иконки
    //         if (!appIcon.isNull()) {
    //             setWindowIcon(appIcon);  // Установка иконки окна
    //             qDebug() << "Иконка успешно загружена из:" << path;
    //             iconLoaded = true;
    //             break;
    //         }
    //     }
    // }

    // // Если иконка не загружена - создание временной
    // if (!iconLoaded) {
    //     qDebug() << "Не удалось загрузить иконку. Проверенные пути:";
    //     for (const QString& path : possiblePaths) {
    //         qDebug() << "  " << path << "(exists:" << QFile::exists(path) << ")";
    //     }

    //     // Создание простой иконки программно для теста
    //     QPixmap pixmap(32, 32);
    //     pixmap.fill(Qt::blue);  // Синий квадрат
    //     setWindowIcon(QIcon(pixmap));
    //     qDebug() << "Установлена временная иконка";
    // }


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

    // Запоминаем начальное устройство для отслеживания смены
    currentAudioDevice = QMediaDevices::defaultAudioOutput();

    // === Мониторинг аудиоустройств ===
    setupAudioDeviceMonitoring();

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

    // Открытие одной песни
    QPushButton* trackBtn = new QPushButton("🎶 Выбрать трек");
    topBar->addWidget(trackBtn);

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

    // Кнопка сортировки "Станд" (единственная кнопка сортировки)
    sortStandardBtn = new QPushButton("Станд");
    sortStandardBtn->setFixedSize(50, 35);
    sortStandardBtn->setToolTip("Стандартный порядок / Обратный порядок");
    topBar->addWidget(sortStandardBtn);

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

    // ПРАВАЯ ПАНЕЛЬ - СПИСОК ТРЕКОВ
    trackTable = new QTableWidget;
    trackTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setupTrackTable();
    // Проверяем, что таблица инициализирована
    qDebug() << "Таблица инициализирована, колонок:" << trackTable->columnCount();
    trackTable->setTextElideMode(Qt::ElideRight);

    // Устанавливаем кастомный делегат для HTML
    HtmlDelegate* delegate = new HtmlDelegate(this);
    trackTable->setItemDelegate(delegate);


    // trackTable->setUniformItemSizes(false);
    trackTable->setStyleSheet(
        "QListWidget { "
        "background: #fff; "         // Белый фон
        "border: 1px solid #333; "      // Темно-серая рамка
        "border-radius: 10px; "         // Закругленные углы
        "color: #000; "                 // Черный текст
        "font-size: 13px; "             // Размер шрифта
        "}"
        "QListWidget::item:selected { background: #0078d4; color: #fff; }" // Синий выделенный элемент
        );
    contentLayout->addWidget(trackTable, 1);  // Растягиваем список (коэффициент 1)

    // Способ 2: Через перехват событий (дополнительно)
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

    connect(trackBtn, &QPushButton::clicked, [this]() {
        openSingleFile();
        // Открываем диалог выбора трека
        // QString dir = QFileDialog::getExistingDirectory(this, "Выберите любой трек MP3");
        // if (!dir.isEmpty()) {
        //     openSingleFile();  // Сканируем если папка выбрана
            // if (shuffled) {

            // }
        // }
    });

    // Подключаем сигналы от элементов интерфейса к слотам
    connect(trackTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onTableDoubleClicked);
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
    QDir musicDir(defaultFolder);

    qDebug() << "Проверяем папку по умолчанию:" << defaultFolder;
    qDebug() << "Папка существует:" << musicDir.exists();

    if (musicDir.exists()) {
        // Используем таймер для отложенного сканирования
        QTimer::singleShot(100, [this, defaultFolder]() {
            qDebug() << "Начинаем сканирование папки по умолчанию...";
            scanFolder(defaultFolder);
        });
    } else {
        albumLabel->setText("Выберите папку с музыкой");
        qDebug() << "Папка Music не найдена";
    }

    // Инициализируем переменные для thumbnail toolbar
    thumbnailToolbarInitialized = false;
    taskbarList = nullptr;
    updateSortButtonsStyle();  // Обновляем стили кнопок сортировки

    createMenuBar();
    setupShortcuts();  // Настраиваем горячие клавиши
    loadSettings(); // Загружаем сохранённые настройки
    updateMenuBar();

    if (settingsDialog) {
        settingsDialog->setAlwaysSkipBadTracks(alwaysSkipBadTracks_);
        settingsDialog->setDefaultVolume(volumeBeforeMute_);
    }
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

    // Способ 1: Через шорткаты (самый надежный)
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Return), trackTable);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::playSelectedTrack);

    QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), trackTable);
    connect(shortcut2, &QShortcut::activated, this, &MainWindow::playSelectedTrack);
}

// Метод воспроизведения выделенного трека
void MainWindow::playSelectedTrack() {
    QList<QTableWidgetItem*> selectedItems = trackTable->selectedItems();

    if (selectedItems.isEmpty()) {
        qDebug() << "Нет выделенного трека";
        return;
    }

    // Получаем первую выделенную строку
    QTableWidgetItem* selectedItem = selectedItems.first();
    int row = trackTable->row(selectedItem); // Получаем номер строки

    // Проверяем что трек существует в плейлисте
    if (row >= 0 && row < static_cast<int>(playlist.size())) {
        if (playlist.setCurrent(row, true)) {
            auto current = playlist.current();
            if (current) {
                QString filePath = QString::fromStdString(current->path());

                // Проверяем трек перед воспроизведением
                if (!validateTrack(filePath)) {
                    // Трек битый - обрабатываем в зависимости от настроек
                    if (alwaysSkipBadTracks_) {
                        // Автоматически ищем следующий валидный трек
                        if (!navigateAutoSkip(true)) {
                            qDebug() << "Не удалось найти валидный трек после битого";
                            player->stop();
                            controls->setPlaying(false);
                        }
                    } else {
                        // Показываем диалог
                        showBadTrackDialog(filePath, true);
                    }
                    return;
                }

                // Трек валиден - воспроизводим
                currentPlayingTrack_ = *current;
                hasCurrentPlayingTrack_ = true;
                player->setSource(QUrl::fromLocalFile(filePath));
                player->play();
                controls->setPlaying(true);
                updateThumbnailButtons();
                updateUI();
                highlightCurrentTrack();

                // auto cur = playlist.current();
                // if (cur) showTrackNotification(cur->qArtist(), cur->qTitle());

                auto cur = playlist.current();
                if (cur) showCustomTrackNotification(cur->qArtist(), cur->qTitle(), cur->getCoverImage());

                qDebug() << "Воспроизводится трек:" << QString::fromStdString(current->title());
            }
        }
    }
}

// Сканирование папки и добавление MP3 файлов в плейлист
void MainWindow::scanFolder(const QString& path) {
    lastScannedFolder_ = path;
    qDebug() << "Начинаем сканирование папки:" << path;

    // === СОХРАНЯЕМ ГЕОМЕТРИЮ ОКНА ===
    QByteArray savedGeometry = saveGeometry();
    QByteArray savedState = saveState();
    bool wasMaximized = isMaximized();

    // Сохраняем состояния
    savedShuffleState_ = controls->isShuffleEnabled();
    savedRepeatMode_ = static_cast<Playlist::RepeatMode>(controls->getRepeatState());

    playlist.clear();
    originalTracks_.clear();
    trackTable->clearContents();
    trackTable->setRowCount(0);
    folderOriginalTracks_.clear();

    // Сброс состояния сортировки при загрузке новой папки
    isStandardSortAscending_ = true;
    columnSortOrders_.clear();
    trackTable->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);

    // Отключаем обновление таблицы для ускорения
    trackTable->setUpdatesEnabled(false);
    trackTable->setSortingEnabled(false);

    // Загружаем рейтинги
    playlist.loadRatings();

    // Собираем все MP3 файлы
    QStringList filePaths;
    QDirIterator it(path, {"*.mp3"}, QDir::Files, QDirIterator::Subdirectories);

    // Быстрый подсчет файлов
    while (it.hasNext()) {
        filePaths.append(it.next());
    }

    if (filePaths.isEmpty()) {
        trackTable->setUpdatesEnabled(true);
        albumLabel->setText("В папке нет MP3 файлов");
        return;
    }

    qDebug() << "Найдено MP3 файлов:" << filePaths.size();

    // Создаем диалог прогресса
    QProgressDialog progress("Сканирование MP3 файлов...", "Отмена", 0, filePaths.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);

    // Быстрое добавление треков (только имена файлов)
    trackTable->setRowCount(filePaths.size());

    for (int i = 0; i < filePaths.size(); ++i) {
        if (progress.wasCanceled()) {
            break;
        }

        QString filePath = filePaths[i];
        QFileInfo fileInfo(filePath);

        // Обновляем прогресс каждые 100 файлов
        if (i % 100 == 0) {
            progress.setValue(i);
            progress.setLabelText(QString("Обработано %1 из %2 файлов")
                                      .arg(i)
                                      .arg(filePaths.size()));
            QCoreApplication::processEvents();
        }

        // Создаем трек с быстрой загрузкой метаданных
        Track track(filePath.toStdString());
        track.loadMetadataFast();  // Только из имени файла - быстро!

        // Рейтинг уже загружен в playlist — применяем к треку
        std::string normalizedPath = QDir::cleanPath(QString::fromStdString(track.path())).toStdString();
        auto it = playlist.savedRatings_.find(normalizedPath);
        if (it != playlist.savedRatings_.end()) {
            track.setTrackRating(it->second);
            qDebug() << "  Применён рейтинг" << it->second << "для" << QString::fromStdString(track.path());
        }

        playlist.add(track);
        originalTracks_.push_back(track);

        // Заполняем таблицу
        int row = i;
        QString filePathForRole = filePath;  // Сохраняем полный путь

        // Название трека - СОХРАНЯЕМ ПУТЬ, А НЕ ИНДЕКС!
        QTableWidgetItem* titleItem = new QTableWidgetItem(track.qTitle());
        titleItem->setData(Qt::UserRole, filePathForRole);  // ← ПУТЬ, а не row!
        titleItem->setData(Qt::UserRole + 1, track.qTitle());
        trackTable->setItem(row, COL_TITLE, titleItem);

        // Исполнитель - тоже сохраняем путь
        QTableWidgetItem* artistItem = new QTableWidgetItem(track.qArtist());
        artistItem->setData(Qt::UserRole, filePathForRole);
        trackTable->setItem(row, COL_ARTIST, artistItem);

        // Жанр
        trackTable->setItem(row, COL_GENRE, new QTableWidgetItem(track.qGenre()));

        // Альбом
        trackTable->setItem(row, COL_ALBUM, new QTableWidgetItem(track.qAlbum()));

        // Рейтинг — БЕРЁМ ИЗ ТРЕКА, а не хардкод!
        double rating = track.rating();
        QString ratingText;
        int fullStars = static_cast<int>(rating);
        for (int s = 0; s < 5; ++s) {
            ratingText += (s < fullStars) ? "★" : "☆";
        }
        QTableWidgetItem* ratingItem = new QTableWidgetItem(ratingText);
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setData(Qt::UserRole, filePathForRole);
        ratingItem->setData(Qt::UserRole + 2, rating);  // числовой рейтинг
        trackTable->setItem(row, COL_RATING, ratingItem);

        // Год
        trackTable->setItem(row, COL_YEAR, new QTableWidgetItem(track.qYear()));
    }

    progress.close();

    // Включаем обновление таблицы
    trackTable->setSortingEnabled(false); // ОТКЛЮЧАЕМ встроенную сортировку Qt, чтобы она не расс
    trackTable->setUpdatesEnabled(true);

    // Не сбрасываем трек и не обновляем левую панель, если что-то уже играет ===
    bool isCurrentlyPlaying = (player->playbackState() == QMediaPlayer::PlayingState);

    if (playlist.size() > 0) {
        playlist.setRepeatMode(savedRepeatMode_);
        playlist.setShuffle(savedShuffleState_);
        controls->setRepeatState(static_cast<int>(savedRepeatMode_));
        controls->setShuffleState(savedShuffleState_);
        // НЕ устанавливаем current и НЕ вызываем updateUI() — левая панель остаётся пустой
    }

    updateSortButtonsStyle();
    loadSettings();

    qDebug() << "Загружено треков:" << playlist.size();

    // === ВОССТАНАВЛИВАЕМ ГЕОМЕТРИЮ ОКНА ===
    if (wasMaximized) {
        showMaximized();
    } else {
        restoreGeometry(savedGeometry);
        restoreState(savedState);
    }

    // НОВАЯ ЧАСТЬ: Фоновая загрузка метаданных через taglib
    // ===================================================
    if (!filePaths.isEmpty()) {
        // Создаем обертку для фоновой загрузки
        FastTagReaderWrapper* wrapper = new FastTagReaderWrapper(this);

        // Подключаем сигнал для получения метаданных
        connect(wrapper, &FastTagReaderWrapper::metadataReady,
                this, &MainWindow::onMetadataLoaded);

        // Подключаем сигнал прогресса
        connect(wrapper, &FastTagReaderWrapper::batchProgress,
                [this](int current, int total) {
                    // Обновляем прогресс каждые 10 файлов
                    if (current % 10 == 0 || current == total) {
                        qDebug() << "📥 Загружено метаданных:" << current << "/" << total;

                        // Обновляем статус-бар (если есть)
                        // statusBar()->showMessage(QString("Загрузка метаданных: %1/%2")
                        //                         .arg(current).arg(total));
                    }
                });

        // Подключаем сигнал завершения
        connect(wrapper, &FastTagReaderWrapper::batchFinished,
                [this, wrapper]() {
                    qDebug() << "✅ Фоновая загрузка метаданных завершена";

                    // Обновляем статус-бар
                    // statusBar()->showMessage("Загрузка метаданных завершена", 3000);

                    // Обновляем текущий трек, если он изменился
                    // updateUI();
                    if (!hasCurrentPlayingTrack_) {
                        updateUI();
                    }

                    // Удаляем wrapper после завершения
                    wrapper->deleteLater();
                });

        // Запускаем фоновую загрузку
        qDebug() << "🚀 Запускаем фоновую загрузку метаданных для" << filePaths.size() << "файлов";
        wrapper->readBatchAsync(filePaths);
    }
}

// Новый метод для фоновой загрузки реальных метаданных
void MainWindow::loadRealMetadataInBackground(const QStringList& filePaths) {
    qDebug() << "Начинаем фоновую загрузку реальных метаданных...";

    QProgressDialog progress("Загрузка метаданных...", "Отмена", 0, filePaths.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);

    for (int i = 0; i < filePaths.size(); ++i) {
        if (progress.wasCanceled()) {
            break;
        }

        if (i % 50 == 0) {
            progress.setValue(i);
            progress.setLabelText(QString("Загружено метаданных %1 из %2")
                                      .arg(i)
                                      .arg(filePaths.size()));
            QCoreApplication::processEvents();
        }

        const QString& filePath = filePaths[i];

        // Загружаем реальные метаданные
        TrackMetadata metadata = FastMetadataReader::instance().getMetadata(filePath);

        // Обновляем трек в плейлисте
        if (i < static_cast<int>(playlist.size())) {
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

            // Обновляем таблицу если были изменения
            if (updated) {
                updateTrackTableRow(i);

                // Обновляем строку в таблице
                if (trackTable->item(i, COL_TITLE)) {
                    trackTable->item(i, COL_TITLE)->setText(track.qTitle());
                }
                if (trackTable->item(i, COL_ARTIST)) {
                    trackTable->item(i, COL_ARTIST)->setText(track.qArtist());
                }
                if (trackTable->item(i, COL_ALBUM)) {
                    trackTable->item(i, COL_ALBUM)->setText(track.qAlbum());
                }
                if (trackTable->item(i, COL_GENRE)) {
                    trackTable->item(i, COL_GENRE)->setText(track.qGenre());
                }
                if (trackTable->item(i, COL_YEAR)) {
                    trackTable->item(i, COL_YEAR)->setText(track.qYear());
                }
            }
        }
    }

    progress.close();
    qDebug() << "Фоновая загрузка метаданных завершена";
}

// Метод проверки трека (добавьте после других методов)
bool MainWindow::validateTrack(const QString& filePath) {
    if (!trackValidator) {
        // Создаем валидатор если его нет
        trackValidator = new TrackValidator(this);
    }

    return trackValidator->validateTrack(filePath);
}

// // Метод поиска и воспроизведения валидного трека
// bool MainWindow::findAndPlayValidTrack(bool forward) {
//     if (playlist.size() == 0) return false;

//     int maxAttempts = playlist.size();
//     int attempts = 0;

//     size_t startIndex = playlist.currentIndex();

//     while (attempts < maxAttempts) {
//         // Переходим к следующему/предыдущему треку
//         bool success;
//         if (forward) {
//             success = playlist.next();
//         } else {
//             success = playlist.prev(0, true); // skipThreeSecondRule = true
//         }

//         if (!success) {
//             return false;
//         }

//         auto current = playlist.current();
//         if (current) {
//             QString filePath = QString::fromStdString(current->path());

//             // Проверяем трек
//             if (validateTrack(filePath)) {
//                 // Трек валиден - воспроизводим
//                 player->setSource(QUrl::fromLocalFile(filePath));
//                 player->play();
//                 controls->setPlaying(true);
//                 updateUI();
//                 highlightCurrentTrack();
//                 return true;
//             } else {
//                 qDebug() << "Трек" << playlist.currentIndex() << "невалиден, продолжаем поиск...";
//             }
//         }

//         attempts++;

//         // Защита от бесконечного цикла
//         if (playlist.currentIndex() == startIndex) {
//             qDebug() << "Вернулись к начальному индексу, поиск завершен";
//             break;
//         }
//     }

//     return false;
// }

// Метод пропуска битого трека
// void MainWindow::skipBadTrackAndContinue(const QString& filePath) {
//     Q_UNUSED(filePath);

//     if (alwaysSkipBadTracks_) {
//         // Автоматически ищем следующий валидный трек
//         if (!findAndPlayValidTrack(lastWasForward_)) {
//             player->stop();
//             controls->setPlaying(false);
//         }
//         return;
//     }

//     BadTrackDialog dialog(this);
//     dialog.setTrackInfo(filePath, "Трек поврежден или недоступен");

//     if (dialog.exec() == QDialog::Accepted) {
//         alwaysSkipBadTracks_ = dialog.skipAlways();

//         // Ищем следующий валидный трек
//         if (!findAndPlayValidTrack(lastWasForward_)) {
//             player->stop();
//             controls->setPlaying(false);
//         }
//     } else {
//         player->stop();
//         controls->setPlaying(false);
//     }
// }

// Обработчик битого трека
void MainWindow::handleInvalidTrack(const QString& filePath, const QString& error) {
    Q_UNUSED(error);

    qDebug() << "handleInvalidTrack: файл =" << filePath;

    // Если уже стоит галочка "Всегда пропускать" - автоматически пропускаем
    if (alwaysSkipBadTracks_) {
        qDebug() << "Автоматически пропускаем битый трек (настройка включена)";
        if (navigateAutoSkip(lastWasForward_)) {
            return;
        } else {
            player->stop();
            controls->setPlaying(false);
        }
        return;
    }

    // Показываем диалог
    showBadTrackDialog(filePath, lastWasForward_);
}

// Обработчик изменения рейтинга
void MainWindow::onRatingChanged(int rating) {
    double dRating = static_cast<double>(rating);
    playlist.setCurrentTrackRating(dRating);

    // Обновляем копию текущего трека на левой панели
    if (hasCurrentPlayingTrack_) {
        currentPlayingTrack_.setTrackRating(dRating);
    }

    updateUI();

    int currentRow = static_cast<int>(playlist.currentIndex());
    updateTrackTableRow(currentRow);
}

// Воспроизведение текущего трека
void MainWindow::playCurrentTrack() {
    // Если нет текущего трека, но есть треки в плейлисте — берём первый
    if (!playlist.current().has_value() && !playlist.all().empty()) {
        playlist.setCurrent(0);
    }

    auto current = playlist.current();
    if (!current) return;

    QString filePath = QString::fromStdString(current->path());

    // Всегда проверяем трек перед воспроизведением
    if (!validateTrack(filePath)) {
        // Если трек битый - обрабатываем в зависимости от настроек
        if (alwaysSkipBadTracks_) {
            // Автоматически пропускаем и ищем следующий валидный трек
            if (!navigateAutoSkip(true)) {
                qDebug() << "Не удалось найти валидный трек после битого";
                player->stop();
                controls->setPlaying(false);
            }
        } else {
            // Показываем диалог только если настройка выключена
            showBadTrackDialog(filePath, true);
        }
        return;
    }

    // === СОХРАНЯЕМ КОПИЮ ТЕКУЩЕГО ТРЕКА ===
    currentPlayingTrack_ = *current;
    hasCurrentPlayingTrack_ = true;

    // Трек валиден - воспроизводим
    player->setSource(QUrl::fromLocalFile(filePath));
    player->play();
    controls->setPlaying(true);
    updateThumbnailButtons();
    updateUI();
    highlightCurrentTrack();

    // showTrackNotification(current->qArtist(), current->qTitle());
    // void showCustomTrackNotification(const QString& artist, const QString& title, const QImage& cover);

    auto cur = playlist.current();
    if (cur) showCustomTrackNotification(cur->qArtist(), cur->qTitle(), cur->getCoverImage());
}

// Перезапуск текущего трека (с начала)
void MainWindow::restartCurrentTrack() {
    player->setPosition(0);  // Перематываем в начало
    player->play();          // Запускаем воспроизведение
}

// Обновление пользовательского интерфейса
void MainWindow::updateUI() {
    const Track* trackToShow = nullptr;

    // Если в плеере есть источник — показываем реально играющий трек
    if (hasCurrentPlayingTrack_ && !player->source().isEmpty()) {
        trackToShow = &currentPlayingTrack_;
    }
    // else {
    //     auto current = playlist.current();
    //     if (current.has_value()) {
    //         trackToShow = &*current;  // ← адрес Track внутри optional
    //     }
    // }

    if (!trackToShow) {
        albumLabel->setText("Выберите папку с музыкой");
        artistLabel->setText("");
        QPixmap coverPixmap(coverLabel->width(), coverLabel->height());
        coverPixmap.fill(Qt::darkGray);
        coverLabel->setPixmap(coverPixmap);
        coverLabel->setText("No Cover");
        return;
    }

    // --- Обложка ---
    QImage coverImage = trackToShow->getCoverImage();
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

    // --- Текст ---
    albumLabel->setText(trackToShow->qTitle());
    artistLabel->setText(trackToShow->qArtist());

    // --- Рейтинг ---
    double rating = trackToShow->rating();
    for (int i = 0; i < 5; ++i) {
        starButtons[i]->setText((i < rating) ? "★" : "☆");
    }

    // Обновляем строку в таблице только если нет "висящего" трека в плеере
    if (!hasCurrentPlayingTrack_ || player->source().isEmpty()) {
        int currentRow = static_cast<int>(playlist.currentIndex());
        updateTrackTableRow(currentRow);
    }

    highlightCurrentTrack();
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

    updateThumbnailButtons();  // ← Обновляем иконки в тулбаре
}

// Обработчик кнопки перемешивания
void MainWindow::onShuffleClicked() {
    bool newShuffleState = !playlist.isShuffled();  // Инвертируем состояние
    playlist.setShuffle(newShuffleState);           // Устанавливаем
    controls->setShuffleState(newShuffleState);     // Обновляем UI

    // Сохраняем состояние для будущих папок
    savedShuffleState_ = newShuffleState;

    updateThumbnailButtons();  // ← Обновляем иконки в тулбаре
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
            // ВМЕСТО вызова playCurrentTrack() используем логику с пропуском
            auto current = playlist.current();
            if (current) {
                QString filePath = QString::fromStdString(current->path());

                // Проверяем трек
                if (!validateTrack(filePath)) {
                    // Трек битый - обрабатываем в зависимости от настроек
                    if (alwaysSkipBadTracks_) {
                        // Автоматически пропускаем и ищем следующий валидный
                        if (!navigateAutoSkip(true)) {
                            player->stop();
                            controls->setPlaying(false);
                        }
                    } else {
                        // Показываем диалог
                        showBadTrackDialog(filePath, true);
                    }
                    return;
                }

                // Трек валиден - воспроизводим
                currentPlayingTrack_ = *current;
                hasCurrentPlayingTrack_ = true;
                player->setSource(QUrl::fromLocalFile(filePath));
                player->play();
                controls->setPlaying(true);
                updateUI();
                highlightCurrentTrack();

                // auto cur = playlist.current();
                // if (cur) showTrackNotification(cur->qArtist(), cur->qTitle());
                auto cur = playlist.current();
                if (cur) showCustomTrackNotification(cur->qArtist(), cur->qTitle(), cur->getCoverImage());
            } else {
                player->stop();
                controls->setPlaying(false);
            }
        }
    }

    updateThumbnailButtons();  // Обновляем кнопки в thumbnail toolbar
}

// Обработчик двойного клика по треку в списке
void MainWindow::onTableDoubleClicked(int row, int column) {
    Q_UNUSED(column);

    if (row >= 0 && row < static_cast<int>(playlist.size())) {
        playlist.setCurrent(row, true);
        playCurrentTrack();
    }
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
    // Отключаем сортировку на время фильтрации
    trackTable->setSortingEnabled(false);

    for (int i = 0; i < trackTable->rowCount(); ++i) {
        // Получаем элемент в первой колонке (название трека)
        QTableWidgetItem* titleItem = trackTable->item(i, COL_TITLE);
        if (!titleItem) continue;

        // Получаем оригинальный текст
        QString original = titleItem->data(Qt::UserRole + 1).toString();
        if (original.isEmpty()) {
            original = titleItem->text();
            titleItem->setData(Qt::UserRole + 1, original);
        }

        // Проверяем совпадение
        bool shouldShow = text.isEmpty();
        if (!shouldShow) {
            // Проверяем название
            shouldShow = original.contains(text, Qt::CaseInsensitive);

            // Если не нашли в названии, проверяем другие колонки
            if (!shouldShow) {
                for (int col = 1; col < COL_COUNT; col++) {
                    if (col == COL_RATING) continue; // Пропускаем рейтинг

                    QTableWidgetItem* item = trackTable->item(i, col);
                    if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                        shouldShow = true;
                        break;
                    }
                }
            }
        }

        // Показываем/скрываем строку
        trackTable->setRowHidden(i, !shouldShow);

        // Подсвечиваем если нужно
        if (shouldShow && !text.isEmpty()) {
            QString htmlText = simpleHighlight(original, text);
            titleItem->setText(htmlText);
        } else if (shouldShow) {
            titleItem->setText(original);
        }
    }

    // Включаем сортировку обратно
    trackTable->setSortingEnabled(true);

    // После фильтрации сохраняем выделение текущего трека
    highlightCurrentTrack();
}

// Обработчик стандартной сортировки (исходный порядок)
void MainWindow::onSortStandardClicked() {
    if (folderOriginalTracks_.empty()) return;

    // Если была сортировка по заголовкам — всегда сбрасываем к исходному порядку
    if (!columnSortOrders_.isEmpty()) {
        originalTracks_ = folderOriginalTracks_;
        applySorting(originalTracks_, "Стандарт");
        columnSortOrders_.clear();
        trackTable->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
        isStandardSortAscending_ = true;
    }
    // Иначе работаем как переключатель: исходный <-> реверс
    else if (isStandardSortAscending_) {
        originalTracks_ = folderOriginalTracks_;
        applySorting(originalTracks_, "Стандарт");
        isStandardSortAscending_ = false;
    } else {
        std::vector<Track> reversed = folderOriginalTracks_;
        std::reverse(reversed.begin(), reversed.end());
        originalTracks_ = reversed;
        applySorting(reversed, "Реверс");
        isStandardSortAscending_ = true;
    }

    updateSortButtonsStyle();
}

// Применение сортировки к плейлисту и UI
void MainWindow::applySorting(const std::vector<Track>& tracks, const QString& sortName) {
    std::unordered_map<std::string, double> savedRatingsCopy = playlist.savedRatings_;
    auto currentTrack = playlist.current();
    std::string currentPath = currentTrack ? currentTrack->path() : "";

    // === БЛОКИРУЕМ ПЕРЕРИСОВКУ ===
    trackTable->setUpdatesEnabled(false);
    trackTable->blockSignals(true);
    trackTable->setSortingEnabled(false);

    playlist.clear();
    trackTable->clearContents();
    trackTable->setRowCount(0);
    playlist.savedRatings_ = savedRatingsCopy;

    for (const auto& track : tracks) {
        Track trackWithRating = track;
        std::string normalizedPath = QDir::cleanPath(QString::fromStdString(track.path())).toStdString();
        auto it = playlist.savedRatings_.find(normalizedPath);
        if (it != playlist.savedRatings_.end())
            trackWithRating.setTrackRating(it->second);
        playlist.add(trackWithRating);
    }

    int rowCount = static_cast<int>(tracks.size());
    trackTable->setRowCount(rowCount);

    for (int i = 0; i < rowCount; ++i) {
        const Track& track = tracks[i];
        int row = i;
        QString filePath = QString::fromStdString(track.path());

        QTableWidgetItem* titleItem = new QTableWidgetItem(track.qTitle());
        titleItem->setData(Qt::UserRole, filePath);
        titleItem->setData(Qt::UserRole + 1, track.qTitle());
        trackTable->setItem(row, COL_TITLE, titleItem);

        QTableWidgetItem* artistItem = new QTableWidgetItem(track.qArtist());
        artistItem->setData(Qt::UserRole, filePath);
        trackTable->setItem(row, COL_ARTIST, artistItem);

        trackTable->setItem(row, COL_GENRE, new QTableWidgetItem(track.qGenre()));

        QTableWidgetItem* albumItem = new QTableWidgetItem(track.qAlbum());
        albumItem->setData(Qt::UserRole, filePath);
        trackTable->setItem(row, COL_ALBUM, albumItem);

        double rating = track.rating();
        QString ratingText;
        int fullStars = static_cast<int>(rating);
        for (int s = 0; s < 5; ++s)
            ratingText += (s < fullStars) ? "★" : "☆";
        QTableWidgetItem* ratingItem = new QTableWidgetItem(ratingText);
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setData(Qt::UserRole, filePath);
        ratingItem->setData(Qt::UserRole + 2, rating);
        trackTable->setItem(row, COL_RATING, ratingItem);

        QTableWidgetItem* yearItem = new QTableWidgetItem(track.qYear());
        yearItem->setData(Qt::UserRole, filePath);
        trackTable->setItem(row, COL_YEAR, yearItem);

        if (track.path() == currentPath)
            playlist.setCurrent(i);
    }

    // === РАЗБЛОКИРУЕМ ===
    trackTable->blockSignals(false);
    trackTable->setUpdatesEnabled(true);

    suppressNextScroll_ = true;
    trackTable->scrollToTop();   // ← сразу в начало
    updateUI();
    onSearchTextChanged(searchEdit->text());
}

// Обновление стилей кнопок сортировки
void MainWindow::updateSortButtonsStyle() {
    QString activeStyle =
        "QPushButton { "
        "background: #0078d4; "
        "border: 1px solid #0078d4; "
        "border-radius: 8px; "
        "color: #fff; "
        "font-size: 12px; "
        "}";

    QString inactiveStyle =
        "QPushButton { "
        "background: #333; "
        "border: 1px solid #444; "
        "border-radius: 8px; "
        "color: #fff; "
        "font-size: 12px; "
        "}"
        "QPushButton:hover { "
        "background: #444; "
        "}";

    // Кнопка активна, если выбран стандартный порядок (не реверс)
    sortStandardBtn->setStyleSheet(isStandardSortAscending_ ? inactiveStyle : activeStyle);

    if (isStandardSortAscending_) {
        sortStandardBtn->setToolTip("Нажмите для стандартного порядка");
    } else {
        sortStandardBtn->setToolTip("Нажмите для обратного порядка");
    }
}

// Деструктор главного окна - вызывается при уничтожении объекта MainWindow
MainWindow::~MainWindow() {
    cleanupThumbnailToolBar();  // Очищаем ресурсы thumbnail toolbar при закрытии приложения
}

// Метод инициализации thumbnail toolbar (панель предпросмотра в Windows)
void MainWindow::setupThumbnailToolBar() {
#ifdef Q_OS_WIN
    if (thumbnailToolbarInitialized) return;

    // Создаем все иконки
    playIcon = createPlayIcon();
    pauseIcon = createPauseIcon();
    nextIcon = createNextIcon();
    prevIcon = createPrevIcon();
    repeatIcon = createRepeatIcon();
    shuffleIcon = createShuffleIcon();
    repeatActiveIcon = createRepeatActiveIcon();
    shuffleActiveIcon = createShuffleActiveIcon();

    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                                  IID_ITaskbarList3, &taskbarList);

    if (SUCCEEDED(hr)) {
        ITaskbarList3* pTaskbarList = (ITaskbarList3*)taskbarList;
        hr = pTaskbarList->HrInit();

        if (SUCCEEDED(hr)) {
            THUMBBUTTON thumbButtons[5];

            // Кнопка 0: Repeat (слева)
            thumbButtons[0].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[0].iId = 0;
            thumbButtons[0].hIcon = (playlist.repeatMode() != Playlist::RepeatMode::None)
                                        ? repeatActiveIcon : repeatIcon;
            wcscpy(thumbButtons[0].szTip, L"Повтор");
            thumbButtons[0].dwFlags = THBF_ENABLED;

            // Кнопка 1: Previous
            thumbButtons[1].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[1].iId = 1;
            thumbButtons[1].hIcon = prevIcon;
            wcscpy(thumbButtons[1].szTip, L"Предыдущий");
            thumbButtons[1].dwFlags = THBF_ENABLED;

            // Кнопка 2: Play/Pause
            thumbButtons[2].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[2].iId = 2;
            thumbButtons[2].hIcon = (player->playbackState() == QMediaPlayer::PlayingState)
                                        ? pauseIcon : playIcon;
            wcscpy(thumbButtons[2].szTip, L"Воспроизведение/Пауза");
            thumbButtons[2].dwFlags = THBF_ENABLED;

            // Кнопка 3: Next
            thumbButtons[3].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[3].iId = 3;
            thumbButtons[3].hIcon = nextIcon;
            wcscpy(thumbButtons[3].szTip, L"Следующий");
            thumbButtons[3].dwFlags = THBF_ENABLED;

            // Кнопка 4: Shuffle (справа)
            thumbButtons[4].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[4].iId = 4;
            thumbButtons[4].hIcon = playlist.isShuffled() ? shuffleActiveIcon : shuffleIcon;
            wcscpy(thumbButtons[4].szTip, L"Случайный порядок");
            thumbButtons[4].dwFlags = THBF_ENABLED;

            hr = pTaskbarList->ThumbBarAddButtons((HWND)winId(), 5, thumbButtons);

            if (SUCCEEDED(hr)) {
                thumbnailToolbarInitialized = true;
            }
        }
    }
#endif
}

// Метод обновления состояния кнопок thumbnail toolbar
void MainWindow::updateThumbnailButtons() {
#ifdef Q_OS_WIN
    if (!thumbnailToolbarInitialized || !taskbarList) return;

    ITaskbarList3* pTaskbarList = (ITaskbarList3*)taskbarList;

    // Обновляем Play/Pause (ID 2)
    THUMBBUTTON thumbButton;
    thumbButton.dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
    thumbButton.iId = 2;
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        thumbButton.hIcon = pauseIcon;
        wcscpy(thumbButton.szTip, L"Пауза");
    } else {
        thumbButton.hIcon = playIcon;
        wcscpy(thumbButton.szTip, L"Воспроизведение");
    }
    thumbButton.dwFlags = THBF_ENABLED;
    pTaskbarList->ThumbBarUpdateButtons((HWND)winId(), 1, &thumbButton);

    // Обновляем Repeat (ID 0)
    THUMBBUTTON repeatBtn;
    repeatBtn.dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
    repeatBtn.iId = 0;
    repeatBtn.hIcon = (playlist.repeatMode() != Playlist::RepeatMode::None)
                          ? repeatActiveIcon : repeatIcon;
    wcscpy(repeatBtn.szTip, (playlist.repeatMode() != Playlist::RepeatMode::None)
                                ? L"Повтор включен" : L"Повтор");
    repeatBtn.dwFlags = THBF_ENABLED;
    pTaskbarList->ThumbBarUpdateButtons((HWND)winId(), 1, &repeatBtn);

    // Обновляем Shuffle (ID 4)
    THUMBBUTTON shuffleBtn;
    shuffleBtn.dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
    shuffleBtn.iId = 4;
    shuffleBtn.hIcon = playlist.isShuffled() ? shuffleActiveIcon : shuffleIcon;
    wcscpy(shuffleBtn.szTip, playlist.isShuffled() ? L"Shuffle включен" : L"Случайный порядок");
    shuffleBtn.dwFlags = THBF_ENABLED;
    pTaskbarList->ThumbBarUpdateButtons((HWND)winId(), 1, &shuffleBtn);
#endif
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
    if (repeatIcon) { DestroyIcon(repeatIcon); repeatIcon = nullptr; }
    if (shuffleIcon) { DestroyIcon(shuffleIcon); shuffleIcon = nullptr; }
    if (repeatActiveIcon) { DestroyIcon(repeatActiveIcon); repeatActiveIcon = nullptr; }
    if (shuffleActiveIcon) { DestroyIcon(shuffleActiveIcon); shuffleActiveIcon = nullptr; }

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
            case 0: // Repeat
                onRepeatClicked();
                return true;
            case 1: // Previous button
                onPrevClicked();    // Вызываем обработчик предыдущего трека
                return true;        // Сообщаем что событие обработано
            case 2: // Play/Pause button
                onPlayPauseClicked();  // Вызываем обработчик воспроизведения/паузы
                return true;           // Сообщаем что событие обработано
            case 3: // Next button
                onNextClicked();    // Вызываем обработчик следующего трека
                return true;        // Сообщаем что событие обработано
            case 4: // Shuffle
                onShuffleClicked();
                return true;
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

HICON MainWindow::createRepeatIcon() {
    return createIconFromText(L"🔁", 24);
}

HICON MainWindow::createShuffleIcon() {
    return createIconFromText(L"🔀", 24);
}

HICON MainWindow::createRepeatActiveIcon() {
#ifdef Q_OS_WIN
    HDC hdc = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdc);
    int size = 24;
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
    SelectObject(hdcMem, hBitmap);

    // Синий фон
    RECT rect = {0, 0, size, size};
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 212));
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);

    // Белый текст
    HFONT hFont = CreateFont(size-4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
    SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(255, 255, 255));
    SetBkMode(hdcMem, TRANSPARENT);
    DrawText(hdcMem, L"🔁", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    HBITMAP hMask = CreateBitmap(size, size, 1, 1, nullptr);
    ICONINFO iconInfo = {TRUE, 0, 0, hMask, hBitmap};
    HICON hIcon = CreateIconIndirect(&iconInfo);

    DeleteObject(hFont);
    DeleteObject(hBitmap);
    DeleteObject(hMask);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdc);
    return hIcon;
#endif
    return nullptr;
}

HICON MainWindow::createShuffleActiveIcon() {
#ifdef Q_OS_WIN
    // Аналогично createRepeatActiveIcon, но с текстом L"🔀"
    HDC hdc = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdc);
    int size = 24;
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
    SelectObject(hdcMem, hBitmap);

    RECT rect = {0, 0, size, size};
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 212));
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);

    HFONT hFont = CreateFont(size-4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol");
    SelectObject(hdcMem, hFont);
    SetTextColor(hdcMem, RGB(255, 255, 255));
    SetBkMode(hdcMem, TRANSPARENT);
    DrawText(hdcMem, L"🔀", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    HBITMAP hMask = CreateBitmap(size, size, 1, 1, nullptr);
    ICONINFO iconInfo = {TRUE, 0, 0, hMask, hBitmap};
    HICON hIcon = CreateIconIndirect(&iconInfo);

    DeleteObject(hFont);
    DeleteObject(hBitmap);
    DeleteObject(hMask);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdc);
    return hIcon;
#endif
    return nullptr;
}
#endif  // Конец блока #ifdef Q_OS_WIN

// Метод подсветки текущего трека в списке
void MainWindow::highlightCurrentTrack() {
    int currentRow = static_cast<int>(playlist.currentIndex());

    // Снимаем выделение со всех строк
    trackTable->clearSelection();

    if (currentRow >= 0 && currentRow < trackTable->rowCount()) {
        // Выделяем строку
        trackTable->selectRow(currentRow);

        // Прокручиваем к строке если нужно
        if (!suppressNextScroll_) {
            QRect itemRect = trackTable->visualItemRect(trackTable->item(currentRow, 0));
            QRect viewportRect = trackTable->viewport()->rect();


            if (!viewportRect.contains(itemRect)) {
                trackTable->scrollToItem(trackTable->item(currentRow, 0),
                                         QAbstractItemView::EnsureVisible);
            }
        }
    }

    suppressNextScroll_ = false;  // сбрасываем флаг
}

// Обработчик кнопки прокрутки к текущему треку
void MainWindow::onScrollToCurrentClicked() {
    int currentRow = static_cast<int>(playlist.currentIndex());

    if (currentRow >= 0 && currentRow < trackTable->rowCount()) {
        QTableWidgetItem* item = trackTable->item(currentRow, 0);
        if (item) {
            // Принудительно прокручиваем к треку по центру viewport
            trackTable->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            // Снимаем предыдущее выделение
            trackTable->clearSelection();
            // Выделяем строку
            trackTable->selectRow(currentRow);
        }
    }
}

// Основной метод навигации с пропуском битых треков
bool MainWindow::navigateWithSkip(bool forward) {
    lastWasForward_ = forward;

    if (playlist.size() == 0) {
        qDebug() << "Плейлист пуст";
        return false;
    }

    qDebug() << "navigateWithSkip: forward =" << forward
             << ", alwaysSkipBadTracks =" << alwaysSkipBadTracks_;

    // Просто пытаемся найти следующий валидный трек
    if (alwaysSkipBadTracks_) {
        qDebug() << "Используем автоматический пропуск";
        return navigateAutoSkip(forward);
    } else {
        qDebug() << "Используем навигацию с диалогом";
        // Сначала пытаемся перейти один раз
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
            currentPlayingTrack_ = *current;
            hasCurrentPlayingTrack_ = true;
            player->setSource(QUrl::fromLocalFile(filePath));
            player->play();
            controls->setPlaying(true);
            updateUI();
            highlightCurrentTrack();
            return true;
        } else {
            // Трек битый - показываем диалог
            showBadTrackDialog(filePath, forward);
            return false; // Диалог сам решит
        }
    }
}

// Навигация с автоматическим пропуском битых треков
bool MainWindow::navigateAutoSkip(bool forward) {
    size_t startIndex = playlist.currentIndex();
    int attempts = 0;
    const int maxAttempts = playlist.size() * 2; // Увеличиваем количество попыток

    qDebug() << "navigateAutoSkip: forward =" << forward << ", startIndex =" << startIndex;

    while (attempts < maxAttempts) {
        // Пытаемся перейти
        bool navigationSuccess;
        if (forward) {
            navigationSuccess = playlist.next();
        } else {
            navigationSuccess = playlist.prev(0, true);
        }

        qDebug() << "  Попытка" << attempts << ": navigationSuccess =" << navigationSuccess;

        if (!navigationSuccess) {
            qDebug() << "  Навигация не удалась";
            return false;
        }

        auto current = playlist.current();
        if (!current) {
            qDebug() << "  Нет текущего трека";
            return false;
        }

        QString filePath = QString::fromStdString(current->path());
        qDebug() << "  Проверяем трек:" << filePath;

        // ПРОВЕРЯЕМ ТРЕК - ЭТО ГЛАВНОЕ ИСПРАВЛЕНИЕ
        if (validateTrack(filePath)) {
            // Трек валиден - воспроизводим
            qDebug() << "  Трек валиден, воспроизводим";
            currentPlayingTrack_ = *current;
            hasCurrentPlayingTrack_ = true;
            player->setSource(QUrl::fromLocalFile(filePath));
            player->play();
            controls->setPlaying(true);
            updateUI();
            highlightCurrentTrack();

            // auto cur = playlist.current();
            // if (cur) showTrackNotification(cur->qArtist(), cur->qTitle());

            auto cur = playlist.current();
            if (cur) showCustomTrackNotification(cur->qArtist(), cur->qTitle(), cur->getCoverImage());

            return true;
        } else {
            // Трек битый - логируем и продолжаем поиск
            qDebug() << "  Трек битый, пропускаем";
            attempts++;
        }

        // Защита от цикла - если вернулись к начальному индексу
        if (playlist.currentIndex() == startIndex) {
            qDebug() << "  Вернулись к начальному индексу, все треки битые";
            // Возвращаемся на стартовую позицию
            playlist.setCurrent(startIndex);
            break;
        }
    }

    qDebug() << "  Не найдено валидных треков после" << attempts << "попыток";
    return false;
}

// Навигация с показом диалога для битых треков
bool MainWindow::navigateWithDialog(bool forward) {
    // Просто используем ту же логику, что и в navigateAutoSkip,
    // но перед этим показываем диалог для первого битого трека

    size_t startIndex = playlist.currentIndex();
    int attempts = 0;
    const int maxAttempts = playlist.size() * 2;

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

        // Проверяем трек
        if (validateTrack(filePath)) {
            // Трек валиден - воспроизводим
            player->setSource(QUrl::fromLocalFile(filePath));
            player->play();
            controls->setPlaying(true);
            updateUI();
            highlightCurrentTrack();
            return true;
        } else {
            // Трек битый - показываем диалог ТОЛЬКО ПРИ ПЕРВОМ БИТОМ ТРЕКЕ
            if (attempts == 0) {
                showBadTrackDialog(filePath, forward);
                return false; // Диалог сам решит что делать
            }
            // Если уже показали диалог для первого трека, просто продолжаем
            attempts++;
        }

        // Защита от цикла
        if (playlist.currentIndex() == startIndex) {
            break;
        }
    }

    return false;
}

// Показать диалог для битого трека
void MainWindow::showBadTrackDialog(const QString& filePath, bool wasForward) {
    BadTrackDialog dialog(this);
    dialog.setTrackInfo(filePath, "Трек поврежден или недоступен");

    if (dialog.exec() == QDialog::Accepted) {
        // Если пользователь поставил галочку "Всегда пропускать"
        if (dialog.skipAlways()) {
            alwaysSkipBadTracks_ = true;
            saveSettings(); // Сохраняем настройку
            updateMenuBar(); // Обновляем галочку в меню

            // Также обновляем настройки в диалоге настроек
            if (settingsDialog) {
                settingsDialog->setAlwaysSkipBadTracks(true);
            }

            qDebug() << "Пользователь выбрал 'Всегда пропускать', обновляем все галочки";
        }

        // Ищем следующий валидный трек
        if (navigateAutoSkip(wasForward)) {
            return;
        }
    }

    // Если диалог отменен или не нашли валидный трек
    qDebug() << "Диалог отменен или не найден валидный трек";
    player->stop();
    controls->setPlaying(false);
}

// Проверка наличия валидного трека в направлении
bool MainWindow::hasValidTrackInDirection(bool forward, int maxAttempts) {
    if (playlist.size() == 0) return false;

    // Сохраняем текущее состояние
    size_t originalIndex = playlist.currentIndex();
    auto originalTrack = playlist.current();
    bool found = false;
    int attempts = 0;

    while (attempts < maxAttempts) {
        // Пытаемся перейти
        bool navigationSuccess;
        if (forward) {
            navigationSuccess = playlist.next();
        } else {
            navigationSuccess = playlist.prev(0, true);
        }

        if (!navigationSuccess) {
            break;
        }

        auto current = playlist.current();
        if (!current) {
            break;
        }

        // Проверяем трек
        if (validateTrack(QString::fromStdString(current->path()))) {
            found = true;
            break;
        }

        attempts++;

        // Защита от цикла
        if (playlist.currentIndex() == originalIndex) {
            break;
        }
    }

    // Восстанавливаем исходное состояние
    playlist.setCurrent(originalIndex);
    return found;
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

    // Сохраняем настройки диалога
    if (settingsDialog) {
        settings.setValue("autoSkipBadTracks", alwaysSkipBadTracks_);
        settings.setValue("defaultVolume", volumeBeforeMute_);
    }
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

    // Загружаем настройки в диалог
    if (settingsDialog) {
        settingsDialog->setAlwaysSkipBadTracks(alwaysSkipBadTracks_);
        settingsDialog->setDefaultVolume(volumeBeforeMute_);
    }

    // Обновляем галочку в меню (ВЫЗЫВАЕМ ПОСЛЕ ЗАГРУЗКИ НАСТРОЕК!)
    // Этот вызов должен быть в конструкторе MainWindow после createMenuBar()

    qDebug() << "Загружены настройки: alwaysSkipBadTracks =" << alwaysSkipBadTracks_;
}

// Фильтр событий для обработки клавиш
// Фильтр событий для обработки клавиш
bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == trackTable && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        // Обработка клавиши Enter/Return
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            playSelectedTrack();
            return true; // Событие обработано
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
        bool newSkipSetting = settingsDialog->alwaysSkipBadTracks();
        int newVolume = settingsDialog->defaultVolume();

        // Если настройка изменилась - обновляем ВСЕ места
        if (alwaysSkipBadTracks_ != newSkipSetting) {
            alwaysSkipBadTracks_ = newSkipSetting;
            updateMenuBar(); // Обновляем галочку в меню

            QMessageBox::information(this, "Настройки",
                                     alwaysSkipBadTracks_
                                         ? "✅ Теперь повреждённые треки будут пропускаться автоматически\n"
                                           "(при встрече битого трека плеер перейдет к следующему рабочему)"
                                         : "❌ Пропуск повреждённых треков отключен\n"
                                           "(при встрече битого трека будет показан диалог)");
        }

        volumeBeforeMute_ = newVolume;

        // Применяем настройки
        audioOutput->setVolume(volumeBeforeMute_ / 100.0);
        controls->setVolume(volumeBeforeMute_);

        // Сохраняем в файл
        saveSettings();

        qDebug() << "Настройки сохранены: alwaysSkipBadTracks =" << alwaysSkipBadTracks_
                 << ", volume =" << volumeBeforeMute_;
    }
}

void MainWindow::createMenuBar() {
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // Меню "Файл"
    fileMenu = menuBar->addMenu("Файл");

    QAction* openFolderAction = new QAction("📁 Открыть папку с музыкой", this);
    openFolderAction->setShortcut(QKeySequence("Ctrl+O"));
    connect(openFolderAction, &QAction::triggered, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с MP3");
        if (!dir.isEmpty()) {
            scanFolder(dir);
        }
    });
    fileMenu->addAction(openFolderAction);
    fileMenu->addSeparator();

    QAction* openFileAction = new QAction("🎵 Открыть один файл", this);
    openFileAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(openFileAction, &QAction::triggered, this, &MainWindow::openSingleFile);
    fileMenu->addAction(openFileAction);
    fileMenu->addSeparator();

    QAction* exitAction = new QAction("🚪 Выход", this);
    exitAction->setShortcut(QKeySequence("Alt+F4"));
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(exitAction);

    // Меню "Настройки"
    settingsMenu = menuBar->addMenu("Настройки");

    QAction* settingsAction = new QAction("⚙ Настройки", this);
    settingsAction->setShortcut(QKeySequence("Ctrl+P"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    settingsMenu->addAction(settingsAction);

    settingsMenu->addSeparator();

    // Действие для включения/выключения пропуска битых треков
    // В методе createMenuBar() исправляем создание action:
    QAction* autoSkipAction = settingsMenu->addAction("Всегда пропускать повреждённые треки");
    autoSkipAction->setCheckable(true);
    autoSkipAction->setChecked(alwaysSkipBadTracks_); // Устанавливаем начальное значение

    // Исправляем connect - нужно обновлять alwaysSkipBadTracks_ при изменении
    connect(autoSkipAction, &QAction::triggered, [this, autoSkipAction]() {
        bool newState = autoSkipAction->isChecked();
        alwaysSkipBadTracks_ = newState;
        saveSettings();

        // Также обновляем настройки в диалоге настроек
        if (settingsDialog) {
            settingsDialog->setAlwaysSkipBadTracks(newState);
        }

        QMessageBox::information(this, "Настройки",
                                 newState
                                     ? "✅ Теперь повреждённые треки будут пропускаться автоматически\n"
                                       "(при встрече битого трека плеер перейдет к следующему рабочему)"
                                     : "❌ Пропуск повреждённых треков отключен\n"
                                       "(при встрече битого трека будет показан диалог)");
    });
    settingsMenu->addAction(autoSkipAction);

    // Меню "Справка"
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

// Методы для отображения справки:
void MainWindow::showHelpDialog() {
    QString helpText = R"(
<b>AlexMusic - Простой музыкальный плеер</b>

<b>Основные возможности:</b><br>
• Воспроизведение MP3 файлов<br>
• Управление плейлистами<br>
• Поиск и сортировка треков<br>
• Рейтинг треков (звездочки)<br>
• Поддержка обложек альбомов<br>

<b>Установка папки с музыкой:</b><br>
1. Нажмите кнопку "Выбрать папку с музыкой"<br>
2. Выберите папку с MP3 файлами<br>
3. Плеер автоматически отсканирует все треки<br>

<b>Обработка повреждённых треков:</b><br>
• При обнаружении битого трека показывается диалог<br>
• Можно выбрать "Всегда пропускать повреждённые треки"<br>
• Настройку можно изменить в меню Настройки<br>
    )";

    QMessageBox::information(this, "Справка", helpText);
}

void MainWindow::showHotkeysDialog() {
    QString hotkeys = R"(
<b>Горячие клавиши AlexMusic</b><br>

<b>Управление воспроизведением:</b><br>
• Пробел — Play/Pause<br>
• B — Предыдущий трек<br>
• N — Следующий трек<br>
• Ctrl+R — Повтор трека<br>
• Ctrl+S — Случайный порядок<br>

<b>Управление громкостью:</b><br>
• M — Вкл/выкл звук<br>
• + или Shift+Right — Увеличить громкость<br>
• - или Shift+Left — Уменьшить громкость<br>

<b>Навигация по треку:</b><br>
• ← — Назад 5 секунд<br>
• → — Вперед 5 секунд<br>
• Home — В начало трека<br>
• End — В конец трека<br>

<b>Общие:</b><br>
• Ctrl+F — Поиск<br>
• Ctrl+G — К текущему треку<br>
• Ctrl+O — Открыть папку<br>
• Ctrl+P — Настройки<br>
• F1 — Горячие клавиши<br>
• Alt+F4 — Выход<br>
    )";

    QMessageBox::information(this, "Горячие клавиши", hotkeys);
}

void MainWindow::showAboutDialog() {
    QString aboutText = R"(
<b>AlexMusic v1.0</b>

Простой и удобный музыкальный плеер для Windows.<br>

<b>Основные функции:</b><br>
• Поддержка MP3 файлов<br>
• Управление плейлистами<br>
• Рейтинг треков<br>
• Поиск и фильтрация<br>
• Сортировка по разным критериям<br>
• Поддержка обложек альбомов<br>

<b>Разработчик:</b> AlexMusic Team<br>
<b>Лицензия:</b> MIT License<br>

<b>Обратная связь:</b><br>
Для сообщений об ошибках и предложений
обратитесь к разработчику.<br>
    )";

    QMessageBox::about(this, "О программе", aboutText);
}

void MainWindow::updateMenuBar() {
    // Находим action "Всегда пропускать повреждённые треки" в меню Настройки
    QList<QAction*> actions = settingsMenu->actions();
    for (QAction* action : actions) {
        if (action->text() == "Всегда пропускать повреждённые треки") {
            action->setChecked(alwaysSkipBadTracks_);
            qDebug() << "updateMenuBar: установлена галочка в меню =" << alwaysSkipBadTracks_;
            break;
        }
    }
}

// Новый метод для сортировки по заголовкам
void MainWindow::populateTrackTable() {
    qDebug() << "populateTrackTable: заполнение таблицы";

    trackTable->setSortingEnabled(false);
    trackTable->blockSignals(true);
    trackTable->clearContents();

    const auto& tracks = playlist.all();
    int trackCount = static_cast<int>(tracks.size());

    trackTable->setRowCount(trackCount);

    for (int row = 0; row < trackCount; ++row) {
        const Track& track = tracks[row];

        // Название трека
        QString title = track.qTitle();
        if (title.isEmpty()) title = "Unknown Title";
        QTableWidgetItem* titleItem = new QTableWidgetItem(title);
        titleItem->setData(Qt::UserRole, QVariant::fromValue(row));
        titleItem->setData(Qt::UserRole + 1, title);
        trackTable->setItem(row, COL_TITLE, titleItem);

        // Исполнитель
        QString artist = track.qArtist();
        if (artist.isEmpty()) artist = "Unknown Artist";
        trackTable->setItem(row, COL_ARTIST, new QTableWidgetItem(artist));

        // Жанр
        QString genre = track.qGenre();
        if (genre.isEmpty()) genre = "Unknown";
        trackTable->setItem(row, COL_GENRE, new QTableWidgetItem(genre));

        // Альбом
        QString album = track.qAlbum();
        if (album.isEmpty()) album = "Unknown Album";
        trackTable->setItem(row, COL_ALBUM, new QTableWidgetItem(album));

        // Рейтинг
        double rating = track.rating();
        QString ratingText;
        int fullStars = static_cast<int>(rating);
        for (int s = 0; s < 5; ++s) {
            ratingText += (s < fullStars) ? "★" : "☆";
        }
        QTableWidgetItem* ratingItem = new QTableWidgetItem(ratingText);
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setData(Qt::UserRole, QVariant::fromValue(row));
        ratingItem->setData(Qt::UserRole + 2, rating);
        trackTable->setItem(row, COL_RATING, ratingItem);

        // Год
        QString year = track.qYear();
        if (year.isEmpty()) year = "Unknown";
        trackTable->setItem(row, COL_YEAR, new QTableWidgetItem(year));

        // Обновляем прогресс
        if (row % 50 == 0) {
            QCoreApplication::processEvents();
        }
    }

    trackTable->setSortingEnabled(true);
    trackTable->blockSignals(false);

    qDebug() << "Таблица заполнена, строк:" << trackTable->rowCount();

    highlightCurrentTrack();
}

void MainWindow::onRatingChangedInTable(int row, int rating) {
    if (row < 0 || row >= trackTable->rowCount()) return;

    // Получаем путь трека из таблицы
    QTableWidgetItem* titleItem = trackTable->item(row, COL_TITLE);
    if (!titleItem) return;

    QString filePath = titleItem->data(Qt::UserRole).toString();
    if (filePath.isEmpty()) return;

    // === УСТАНАВЛИВАЕМ РЕЙТИНГ ПО ПУТИ, А НЕ ТЕКУЩЕМУ ТРЕКУ ===
    playlist.setTrackRatingByPath(filePath.toStdString(), static_cast<double>(rating));

    // Если это текущий трек — обновляем UI
    auto current = playlist.current();
    if (current && current->path() == filePath.toStdString()) {
        updateUI();  // Обновляем звёзды в левой панели
    }

    // Обновляем отображение рейтинга в таблице
    QString ratingText;
    for (int s = 0; s < 5; ++s) {
        ratingText += (s < rating) ? "★" : "☆";
    }

    QTableWidgetItem* ratingItem = trackTable->item(row, COL_RATING);
    if (ratingItem) {
        ratingItem->setText(ratingText);
        ratingItem->setData(Qt::UserRole + 2, static_cast<double>(rating));
    }
}

void MainWindow::updateTrackTableRow(int row) {
    if (row < 0 || row >= trackTable->rowCount()) return;

    // Находим соответствующий трек в плейлисте по пути из таблицы
    QTableWidgetItem* titleItem = trackTable->item(row, COL_TITLE);
    if (!titleItem) return;

    QString filePath = titleItem->data(Qt::UserRole).toString();
    if (filePath.isEmpty()) return;

    // Ищем трек в плейлисте
    const Track* trackPtr = nullptr;
    for (const auto& t : playlist.all()) {
        if (t.path() == filePath.toStdString()) {
            trackPtr = &t;
            break;
        }
    }

    if (!trackPtr) return;

    // Обновляем ВСЕ колонки
    if (trackTable->item(row, COL_TITLE)) {
        trackTable->item(row, COL_TITLE)->setText(trackPtr->qTitle());
        trackTable->item(row, COL_TITLE)->setData(Qt::UserRole + 1, trackPtr->qTitle());
    }
    if (trackTable->item(row, COL_ARTIST)) {
        trackTable->item(row, COL_ARTIST)->setText(trackPtr->qArtist());
    }
    if (trackTable->item(row, COL_ALBUM)) {
        trackTable->item(row, COL_ALBUM)->setText(trackPtr->qAlbum());
    }
    if (trackTable->item(row, COL_GENRE)) {
        trackTable->item(row, COL_GENRE)->setText(trackPtr->qGenre());
    }
    if (trackTable->item(row, COL_YEAR)) {
        trackTable->item(row, COL_YEAR)->setText(trackPtr->qYear());
    }

    // Рейтинг
    QString ratingText;
    int fullStars = static_cast<int>(trackPtr->rating());
    for (int s = 0; s < 5; ++s) {
        ratingText += (s < fullStars) ? "★" : "☆";
    }
    QTableWidgetItem* ratingItem = trackTable->item(row, COL_RATING);
    if (ratingItem) {
        ratingItem->setText(ratingText);
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setData(Qt::UserRole + 2, trackPtr->rating());
    }
}

void MainWindow::setupTrackTable() {
    trackTable->setColumnCount(COL_COUNT);

    QStringList headers;
    headers << "Название трека" << "Исполнитель" << "Жанр" << "Альбом" << "Рейтинг" << "Год";
    trackTable->setHorizontalHeaderLabels(headers);

    // Настройка внешнего вида
    trackTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    trackTable->setSelectionMode(QAbstractItemView::SingleSelection);
    trackTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    trackTable->setAlternatingRowColors(true);

    trackTable->setSortingEnabled(false); // Включаем сортировку по клику на заголовки

    // Настройка ширины колонок - разрешаем изменять
    trackTable->horizontalHeader()->setSectionResizeMode(COL_TITLE, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_ARTIST, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_GENRE, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_ALBUM, QHeaderView::Interactive);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_RATING, QHeaderView::Fixed);
    trackTable->horizontalHeader()->setSectionResizeMode(COL_YEAR, QHeaderView::Fixed);


    trackTable->horizontalHeader()->setStretchLastSection(true);

    // Запрещаем появление пустого пространства справа
    trackTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // Таблица занимает всё доступное пространство
    trackTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Убираем лишние отступы
    trackTable->setFrameStyle(QFrame::NoFrame);
    trackTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    trackTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);


    // Устанавливаем начальные размеры
    trackTable->horizontalHeader()->resizeSection(COL_TITLE, 250);
    trackTable->horizontalHeader()->resizeSection(COL_ARTIST, 150);
    trackTable->horizontalHeader()->resizeSection(COL_GENRE, 100);
    trackTable->horizontalHeader()->resizeSection(COL_ALBUM, 150);
    trackTable->horizontalHeader()->resizeSection(COL_RATING, 120); // Немного шире для звезд
    trackTable->horizontalHeader()->resizeSection(COL_YEAR, 80);

    // Создаем делегаты
    HtmlDelegate* htmlDelegate = new HtmlDelegate(this);
    trackTable->setItemDelegate(htmlDelegate);

    // Создаем делегат для рейтинга
    RatingTableDelegate* ratingDelegate = new RatingTableDelegate(this);
    trackTable->setItemDelegateForColumn(COL_RATING, ratingDelegate);

    // Подключаем сигнал от делегата рейтинга
    connect(ratingDelegate, &RatingTableDelegate::ratingChanged,
            this, &MainWindow::onRatingChangedInTable);

    // Подключаем сигналы
    connect(trackTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onTableDoubleClicked);

    // Подключаем сортировку по заголовкам
    connect(trackTable->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onHeaderClicked);
}

// Новый метод для сортировки по заголовкам
void MainWindow::onHeaderClicked(int column) {
    if (originalTracks_.empty()) return;

    // Определяем текущий порядок сортировки для этой колонки
    Qt::SortOrder currentOrder = columnSortOrders_.value(column, Qt::AscendingOrder);
    Qt::SortOrder newOrder = (currentOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    columnSortOrders_[column] = newOrder;

    bool ascending = (newOrder == Qt::AscendingOrder);

    // Сортируем originalTracks_ с "человеческим" сравнением
    std::vector<Track> sorted = originalTracks_;
    std::sort(sorted.begin(), sorted.end(), [column, ascending](const Track& a, const Track& b) {
        QString strA, strB;

        switch (column) {
        case COL_TITLE:
            strA = a.qTitle();
            strB = b.qTitle();
            break;
        case COL_ARTIST:
            strA = a.qArtist();
            strB = b.qArtist();
            break;
        case COL_ALBUM:
            strA = a.qAlbum();
            strB = b.qAlbum();
            break;
        case COL_GENRE:
            strA = a.qGenre();
            strB = b.qGenre();
            break;
        case COL_YEAR:
            strA = a.qYear();
            strB = b.qYear();
            break;
        case COL_RATING:
            return ascending ? a.rating() < b.rating() : a.rating() > b.rating();
        default:
            return false;
        }

        // "Человеческая" сортировка с учётом локали
        int cmp = QString::localeAwareCompare(strA, strB);
        return ascending ? (cmp < 0) : (cmp > 0);
    });

    originalTracks_ = sorted;
    applySorting(sorted, "Сортировка");

    // Обновляем индикатор сортировки
    trackTable->horizontalHeader()->setSortIndicator(column, newOrder);

    isStandardSortAscending_ = true;
}

// метод для обновления UI после загрузки метаданных
void MainWindow::onMetadataLoaded(const QString& filePath, const TrackMetadata& metadata) {
    // Находим трек в плейлисте
    int playlistIndex = -1;
    for (int i = 0; i < playlist.size(); ++i) {
        if (playlist.all()[i].path() == filePath.toStdString()) {
            playlistIndex = i;
            break;
        }
    }

    if (playlistIndex == -1) return;

    // Обновляем данные в плейлисте
    // Обновляем Track в плейлисте
    Track& track = const_cast<Track&>(playlist.all()[playlistIndex]);
    bool changed = false;

    if (!metadata.title.isEmpty() && track.qTitle() != metadata.title) {
        track.setTitle(metadata.title.toStdString());
        changed = true;
    }
    if (!metadata.artist.isEmpty() && track.qArtist() != metadata.artist) {
        track.setArtist(metadata.artist.toStdString());
        changed = true;
    }
    if (!metadata.album.isEmpty() && track.qAlbum() != metadata.album) {
        track.setAlbum(metadata.album.toStdString());
        changed = true;
    }
    if (!metadata.genre.isEmpty() && track.qGenre() != metadata.genre) {
        track.setGenre(metadata.genre.toStdString());
        changed = true;
    }
    if (!metadata.year.isEmpty() && track.qYear() != metadata.year) {
        track.setYear(metadata.year.toStdString());
        changed = true;
    }

    // синхронизируем originalTracks_, чтобы сортировка видела актуальные данные
    for (auto& origTrack : originalTracks_) {
        if (origTrack.path() == filePath.toStdString()) {
            if (!metadata.title.isEmpty() && origTrack.qTitle() != metadata.title)
                origTrack.setTitle(metadata.title.toStdString());
            if (!metadata.artist.isEmpty() && origTrack.qArtist() != metadata.artist)
                origTrack.setArtist(metadata.artist.toStdString());
            if (!metadata.album.isEmpty() && origTrack.qAlbum() != metadata.album)
                origTrack.setAlbum(metadata.album.toStdString());
            if (!metadata.genre.isEmpty() && origTrack.qGenre() != metadata.genre)
                origTrack.setGenre(metadata.genre.toStdString());
            if (!metadata.year.isEmpty() && origTrack.qYear() != metadata.year)
                origTrack.setYear(metadata.year.toStdString());
            break;
        }
    }

    if (!changed) return;

    // Обновляем таблицу (по пути, не по индексу!)
    for (int r = 0; r < trackTable->rowCount(); ++r) {
        QTableWidgetItem* item = trackTable->item(r, COL_TITLE);
        if (item && item->data(Qt::UserRole).toString() == filePath) {
            // Обновляем все колонки
            item->setText(track.qTitle());
            item->setData(Qt::UserRole + 1, track.qTitle());

            if (trackTable->item(r, COL_ARTIST))
                trackTable->item(r, COL_ARTIST)->setText(track.qArtist());
            if (trackTable->item(r, COL_ALBUM))
                trackTable->item(r, COL_ALBUM)->setText(track.qAlbum());
            if (trackTable->item(r, COL_GENRE))
                trackTable->item(r, COL_GENRE)->setText(track.qGenre());
            if (trackTable->item(r, COL_YEAR))
                trackTable->item(r, COL_YEAR)->setText(track.qYear());
            break;
        }
    }

    // === КЛЮЧЕВОЕ: если это текущий трек — обновляем левую панель ===
    // if (playlistIndex == static_cast<int>(playlist.currentIndex())) {
    //     updateUI();  // Теперь данные в Track уже обновлены — отобразятся корректно!
    // }

    // === Обновляем левую панель ТОЛЬКО если это трек, что сейчас в плеере ===
    if (hasCurrentPlayingTrack_ && filePath.toStdString() == currentPlayingTrack_.path()) {
        if (!metadata.title.isEmpty())
            currentPlayingTrack_.setTitle(metadata.title.toStdString());
        if (!metadata.artist.isEmpty())
            currentPlayingTrack_.setArtist(metadata.artist.toStdString());
        if (!metadata.album.isEmpty())
            currentPlayingTrack_.setAlbum(metadata.album.toStdString());
        if (!metadata.genre.isEmpty())
            currentPlayingTrack_.setGenre(metadata.genre.toStdString());
        if (!metadata.year.isEmpty())
            currentPlayingTrack_.setYear(metadata.year.toStdString());

        updateUI();
    }
    // Если плеер пустой, обновляем по старой логике
    else if (!hasCurrentPlayingTrack_ && playlistIndex == static_cast<int>(playlist.currentIndex())) {
        updateUI();
    }
}

// Инициализация иконки в системном трее
void MainWindow::setupTrayIcon() {
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QApplication::windowIcon());  // используем иконку приложения
    trayIcon->setToolTip("AlexMusic");

    // Контекстное меню по ПКМ на иконке трея
    QMenu* trayMenu = new QMenu(this);
    QAction* showAction = trayMenu->addAction("Показать AlexMusic");
    connect(showAction, &QAction::triggered, this, &QWidget::showNormal);

    QAction* quitAction = trayMenu->addAction("Выход");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

// Показ уведомления с именем исполнителя и трека
void MainWindow::showTrackNotification(const QString& artist, const QString& title) {
    // Не показываем, если пользователь сейчас работает в нашем приложении
    if (isActiveWindow()) {
        return;
    }

    if (trayIcon && trayIcon->isSystemTrayAvailable()) {
        trayIcon->showMessage(
            "Сейчас играет",           // заголовок уведомления
            artist + " — " + title,    // текст уведомления
            QSystemTrayIcon::Information,
            4000                       // время показа в мс (4 сек)
            );
    }
}

void MainWindow::showCustomTrackNotification(const QString& artist, const QString& title, const QImage& cover) {
    // Не показываем, если пользователь сейчас работает в нашем приложении
    if (isActiveWindow()) return;

    // Создаём окно-уведомление
    QWidget* toast = new QWidget(nullptr);
    toast->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    toast->setAttribute(Qt::WA_TranslucentBackground);
    toast->setAttribute(Qt::WA_DeleteOnClose);
    toast->setFixedSize(340, 90);

    // Контейнер с градиентом и закруглением
    QFrame* container = new QFrame(toast);
    container->setObjectName("toastContainer");
    container->setStyleSheet(R"(
        #toastContainer {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #3d3d3d, stop:1 #202020);
            border: 1px solid #555;
            border-radius: 16px;
        }
    )");
    container->setGeometry(0, 0, 340, 90);

    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(14);

    // --- Обложка альбома (закруглённая) ---
    QLabel* coverLabel = new QLabel;
    coverLabel->setFixedSize(64, 64);
    if (!cover.isNull()) {
        QPixmap orig = QPixmap::fromImage(cover).scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap rounded(64, 64);
        rounded.fill(Qt::transparent);
        QPainter p(&rounded);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(0, 0, 64, 64, 10, 10);
        p.setClipPath(path);
        p.drawPixmap(0, 0, orig);
        p.end();
        coverLabel->setPixmap(rounded);
    } else {
        coverLabel->setStyleSheet("background: #444; border-radius: 10px; color: #ffcc00; font-size: 24px;");
        coverLabel->setAlignment(Qt::AlignCenter);
        coverLabel->setText("♪");
    }
    layout->addWidget(coverLabel);

    // --- Текст ---
    QVBoxLayout* textLayout = new QVBoxLayout;
    textLayout->setSpacing(4);
    textLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* header = new QLabel("СЕЙЧАС ИГРАЕТ");
    header->setStyleSheet("color: #888; font-size: 9px; font-weight: bold; letter-spacing: 1px;");
    textLayout->addWidget(header);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #ffffff; font-size: 15px; font-weight: bold;");
    QFontMetrics fm(titleLabel->font());
    titleLabel->setText(fm.elidedText(title, Qt::ElideRight, 220));
    textLayout->addWidget(titleLabel);

    QLabel* artistLabel = new QLabel(artist);
    artistLabel->setStyleSheet("color: #bbbbbb; font-size: 13px;");
    QFontMetrics fm2(artistLabel->font());
    artistLabel->setText(fm2.elidedText(artist, Qt::ElideRight, 220));
    textLayout->addWidget(artistLabel);

    textLayout->addStretch();
    layout->addLayout(textLayout, 1);

    // --- Кнопка закрытия ---
    QPushButton* closeBtn = new QPushButton("×");
    closeBtn->setFixedSize(22, 22);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #777;
            font-size: 18px;
            border: none;
            padding-bottom: 4px;
        }
        QPushButton:hover { color: #fff; }
    )");
    connect(closeBtn, &QPushButton::clicked, toast, &QWidget::close);
    layout->addWidget(closeBtn, 0, Qt::AlignTop);

    // --- Позиция: правый нижний угол экрана ---
    QScreen* scr = QApplication::primaryScreen();
    QRect ag = scr->availableGeometry();
    int x = ag.right() - 340 - 18;
    int y = ag.bottom() - 90 - 18;
    toast->move(x, y);
    toast->show();
    toast->raise();

    // --- Плавное появление (fade in) ---
    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(eff);
    QPropertyAnimation* fadeIn = new QPropertyAnimation(eff, "opacity");
    fadeIn->setDuration(250);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutQuad);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    // --- Автозакрытие через 4.5 сек с fade out ---
    QTimer::singleShot(4500, toast, [toast, eff]() {
        QPropertyAnimation* fadeOut = new QPropertyAnimation(eff, "opacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setEasingCurve(QEasingCurve::InQuad);
        connect(fadeOut, &QPropertyAnimation::finished, toast, &QWidget::close);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void MainWindow::setupAudioDeviceMonitoring() {
    // Запоминаем текущее устройство (default output)
    currentAudioDevice = QMediaDevices::defaultAudioOutput();

    // Создаём объект QMediaDevices для подключения к сигналу
    QMediaDevices* mediaDevices = new QMediaDevices(this);  // ← создаём объект, parent = this

    // Подключаемся к сигналу изменения списка аудиоустройств
    connect(mediaDevices, &QMediaDevices::audioOutputsChanged,
            this, &MainWindow::onAudioDevicesChanged);

    qDebug() << "Аудиоустройство по умолчанию:" << currentAudioDevice.description();
}

void MainWindow::onAudioDevicesChanged() {
    QAudioDevice newDefault = QMediaDevices::defaultAudioOutput();

    qDebug() << "Смена аудиоустройств!";
    qDebug() << "  Было:" << currentAudioDevice.description();
    qDebug() << "  Стало:" << newDefault.description();

    // Проверяем, действительно ли изменилось устройство по умолчанию
    if (newDefault.id() != currentAudioDevice.id()) {
        qDebug() << "  → Устройство изменилось, переподключаем...";

        // Сохраняем позицию и состояние воспроизведения
        qint64 savedPosition = player->position();
        bool wasPlaying = (player->playbackState() == QMediaPlayer::PlayingState);

        // Переподключаем аудиовыход
        reconnectAudioOutput();

        // Восстанавливаем позицию и состояние
        player->setPosition(savedPosition);
        if (wasPlaying) {
            player->play();
        }

        // Обновляем текущее устройство
        currentAudioDevice = newDefault;

        // Показываем уведомление (опционально)
        // showTrackNotification("Аудио", "Переключено на: " + newDefault.description());
    }
}

void MainWindow::reconnectAudioOutput() {
    // Удаляем старый audioOutput
    if (audioOutput) {
        // Сохраняем громкость
        float savedVolume = audioOutput->volume();

        // Отключаем от плеера
        player->setAudioOutput(nullptr);

        // Удаляем старый объект
        audioOutput->deleteLater();
        audioOutput = nullptr;

        // Создаём новый на новом default-устройстве
        audioOutput = new QAudioOutput(this);
        audioOutput->setVolume(savedVolume);

        // Подключаем к плееру
        player->setAudioOutput(audioOutput);

        qDebug() << "  AudioOutput переподключен, громкость:" << savedVolume;
    }
}

// запуск одного выбранного трека
void MainWindow::openSingleFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "Выберите MP3 файл", QString(),
        "MP3 файлы (*.mp3);;Все файлы (*)");

    if (filePath.isEmpty()) return;

    // Проверяем валидность
    if (!validateTrack(filePath)) {
        QMessageBox::warning(this, "Ошибка",
                             "Трек повреждён: " + trackValidator->lastError());
        return;
    }

    // Очищаем текущий плейлист
    playlist.clear();
    originalTracks_.clear();
    trackTable->clearContents();
    trackTable->setRowCount(0);

    // Создаем трек
    Track track(filePath.toStdString());
    track.loadMetadata();  // Полная загрузка метаданных

    int duration = FastTagReader::getDuration(filePath);
    // track.setDuration(duration);

    // Добавляем в плейлист
    playlist.add(track);
    originalTracks_.push_back(track);
    playlist.setCurrent(0);

    // === СОХРАНЯЕМ ===
    currentPlayingTrack_ = track;
    hasCurrentPlayingTrack_ = true;

    // Заполняем таблицу
    trackTable->setRowCount(1);

    QTableWidgetItem* titleItem = new QTableWidgetItem(track.qTitle());
    titleItem->setData(Qt::UserRole, filePath);
    titleItem->setData(Qt::UserRole + 1, track.qTitle());
    trackTable->setItem(0, COL_TITLE, titleItem);

    trackTable->setItem(0, COL_ARTIST, new QTableWidgetItem(track.qArtist()));
    trackTable->setItem(0, COL_GENRE, new QTableWidgetItem(track.qGenre()));
    trackTable->setItem(0, COL_ALBUM, new QTableWidgetItem(track.qAlbum()));
    trackTable->setItem(0, COL_YEAR, new QTableWidgetItem(track.qYear()));

    QString durStr = duration > 0
                         ? QString("%1:%2").arg(duration/60).arg(duration%60,2,10,QChar('0'))
                         : "--:--";
    trackTable->setItem(0, COL_DURATION, new QTableWidgetItem(durStr));
    trackTable->setItem(0, COL_PATH, new QTableWidgetItem(filePath));

    // Играем
    playCurrentTrack();
    // updateUI();
}
