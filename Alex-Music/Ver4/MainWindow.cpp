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

// Windows API headers
#ifdef Q_OS_WIN
#include <windows.h>
#include <commctrl.h>
#include <shobjidl.h>
#endif

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AlexMusic");

    // Пытаемся найти и установить иконку несколькими способами
    QIcon appIcon;
    QStringList possiblePaths = {
        QCoreApplication::applicationDirPath() + "/app_icon.ico",
        "C:\\My_QT\\CPP\\Alex_Music\\work\\untitled\\icons\\app_icon.ico",
    };

    bool iconLoaded = false;
    for (const QString& path : possiblePaths) {
        if (QFile::exists(path)) {
            appIcon = QIcon(path);
            if (!appIcon.isNull()) {
                setWindowIcon(appIcon);
                qDebug() << "Иконка успешно загружена из:" << path;
                iconLoaded = true;
                break;
            }
        }
    }

    if (!iconLoaded) {
        qDebug() << "Не удалось загрузить иконку. Проверенные пути:";
        for (const QString& path : possiblePaths) {
            qDebug() << "  " << path << "(exists:" << QFile::exists(path) << ")";
        }

        // Создаем простую иконку программно для теста
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::blue);
        setWindowIcon(QIcon(pixmap));
        qDebug() << "Установлена временная иконка";
    }

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(volumeBeforeMute_ / 100.0);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QHBoxLayout* topBar = new QHBoxLayout;
    QPushButton* folderBtn = new QPushButton("📁 Выбрать папку с музыкой");
    topBar->addWidget(folderBtn);
    topBar->addStretch();
    mainLayout->addLayout(topBar);

    QHBoxLayout* contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(30);

    QWidget* leftPanel = new QWidget;
    leftPanel->setFixedWidth(400);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);

    coverLabel = new QLabel;
    coverLabel->setFixedSize(200, 200);
    coverLabel->setStyleSheet("QLabel { background: #222; border: 2px solid #444; border-radius: 10px; color: #fff; }");
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setText("No Cover");
    leftLayout->addWidget(coverLabel, 0, Qt::AlignCenter);

    albumLabel = new QLabel("Выберите папку с музыкой");
    albumLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #fff; }");
    albumLabel->setWordWrap(true);

    artistLabel = new QLabel;
    artistLabel->setStyleSheet("QLabel { font-size: 14px; color: #ccc; }");
    artistLabel->setWordWrap(true);

    // ЗАМЕНЯЕМ genreLabel на панель с рейтингом
    QWidget* ratingWidget = new QWidget;
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setSpacing(5);
    ratingLayout->setAlignment(Qt::AlignLeft);

    // Создаем 5 кнопок-звезд
    for (int i = 0; i < 5; ++i) {
        starButtons[i] = new QPushButton("☆");
        starButtons[i]->setFixedSize(30, 30);
        starButtons[i]->setStyleSheet(
            "QPushButton {"
            "    background: #333;"
            "    border: 1px solid #555;"
            "    border-radius: 15px;"
            "    color: #ffcc00;"
            "    font-size: 16px;"
            "}"
            "QPushButton:hover {"
            "    background: #444;"
            "}"
            );
        ratingLayout->addWidget(starButtons[i]);

        // Подключаем каждую кнопку к слоту рейтинга
        connect(starButtons[i], &QPushButton::clicked, [this, i]() {
            onRatingChanged(i + 1);  // i+1 потому что рейтинг от 1 до 5
        });
    }

    ratingLayout->addStretch();

    leftLayout->addWidget(albumLabel);
    leftLayout->addWidget(artistLabel);
    leftLayout->addWidget(ratingWidget);  // Добавляем панель рейтинга вместо genreLabel

    contentLayout->addWidget(leftPanel);

    trackList = new QListWidget;
    trackList->setStyleSheet(
        "QListWidget { "
        "background: #1a1a1a; "
        "border: 1px solid #333; "
        "border-radius: 10px; "
        "color: #fff; "
        "font-size: 13px; "
        "alternate-background-color: #222; "
        "}"
        "QListWidget::item:selected { background: #0078d4; }"
        );
    contentLayout->addWidget(trackList, 1);

    mainLayout->addLayout(contentLayout, 1);

    controls = new PlayerControls;
    controls->setStyleSheet(
        "PlayerControls { background: #111; border: 1px solid #333; border-radius: 10px; }"
        "QPushButton { background: #333; color: #fff; border: 1px solid #444; border-radius: 8px; padding: 8px; }"
        "QPushButton:hover { background: #444; }"
        "QPushButton:pressed { background: #555; }"
        );
    mainLayout->addWidget(controls);

    setStyleSheet(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 #0a0a0a, stop:1 #1a1a1a); }"
        "QMainWindow::title { background: transparent; }"
        );
    resize(1000, 700);

    connect(folderBtn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с MP3");
        if (!dir.isEmpty()) scanFolder(dir);
    });

    connect(trackList, &QListWidget::itemDoubleClicked, this, &MainWindow::onTrackListDoubleClicked);
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

    QString defaultFolder = "C:\\Users\\User\\Music";
    if (QDir(defaultFolder).exists()) {
        scanFolder(defaultFolder);
    }

    thumbnailToolbarInitialized = false;
    taskbarList = nullptr;
}

void MainWindow::scanFolder(const QString& path) {
    playlist.clear();
    trackList->clear();

    QDirIterator it(path, {"*.mp3"}, QDir::Files, QDirIterator::Subdirectories);
    int index = 1;

    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.baseName();

        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);
        QString artist = parts.value(0, "Unknown Artist");
        QString title = parts.value(1, baseName);

        playlist.add(Track(filePath.toStdString(), artist.toStdString(),
                           title.toStdString(), "Music for imaginary movies", 0.0));  // Рейтинг 0.0 по умолчанию

        trackList->addItem(QString("%1. %2 - %3").arg(index++).arg(artist).arg(title));
    }

    // ЗАГРУЖАЕМ СОХРАНЕННЫЕ РЕЙТИНГИ после сканирования папки
    playlist.loadRatings();

    if (!playlist.all().empty()) {
        playlist.setCurrent(0);
        updateUI();
    }
}

// НОВЫЙ СЛОТ: обработка изменения рейтинга
void MainWindow::onRatingChanged(int rating) {
    // rating от 1 до 5
    playlist.setCurrentTrackRating(static_cast<double>(rating));
    updateUI(); // Обновляем отображение звезд
}

void MainWindow::playCurrentTrack() {
    auto current = playlist.current();
    if (!current) return;

    player->setSource(QUrl::fromLocalFile(QString::fromStdString(current->path())));
    player->play();
    controls->setPlaying(true);
    updateThumbnailButtons();
    updateUI();
}

void MainWindow::restartCurrentTrack() {
    player->setPosition(0);
    player->play();
}

void MainWindow::updateUI() {
    auto current = playlist.current();
    if (!current) return;

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
        coverLabel->setStyleSheet("QLabel { background: #222; border: 2px solid #444; border-radius: 10px; color: #fff; font-size: 12px; }");
    }

    albumLabel->setText(QString::fromStdString(current->title()));
    artistLabel->setText(QString::fromStdString(current->artist()));

    // ОБНОВЛЯЕМ ОТОБРАЖЕНИЕ ЗВЕЗД РЕЙТИНГА
    double rating = current->rating();
    for (int i = 0; i < 5; ++i) {
        if (i < rating) {
            starButtons[i]->setText("★");  // Заполненная звезда
        } else {
            starButtons[i]->setText("☆");  // Пустая звезда
        }
    }

    trackList->setCurrentRow(static_cast<int>(playlist.currentIndex()));
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
    if (playlist.next()) {
        playCurrentTrack();
    }
}

void MainWindow::onPrevClicked() {
    if (playlist.shouldRestartTrack(player->position())) {
        restartCurrentTrack();
    } else {
        if (playlist.prev(player->position())) {
            playCurrentTrack();
        }
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
        newMode = Playlist::RepeatMode::All;
        break;
    case Playlist::RepeatMode::All:
        newMode = Playlist::RepeatMode::None;
        break;
    }

    playlist.setRepeatMode(newMode);
    controls->setRepeatState(static_cast<int>(newMode));
}

void MainWindow::onShuffleClicked() {
    bool newShuffleState = !playlist.isShuffled();
    playlist.setShuffle(newShuffleState);
    controls->setShuffleState(newShuffleState);
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
        } else {
            onNextClicked();
        }
    }
    updateThumbnailButtons();
}

void MainWindow::onTrackListDoubleClicked(QListWidgetItem* item) {
    int row = trackList->row(item);
    if (playlist.setCurrent(row)) {
        playCurrentTrack();
    }
}

void MainWindow::onMuteToggled(bool muted) {
    if (muted) {
        audioOutput->setVolume(0);
    } else {
        audioOutput->setVolume(volumeBeforeMute_ / 100.0);
    }
}

MainWindow::~MainWindow() {
    cleanupThumbnailToolBar();
}

void MainWindow::setupThumbnailToolBar() {
#ifdef Q_OS_WIN
    if (thumbnailToolbarInitialized) return;

    // Создаем иконки
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

            // Левая кнопка: ⏮ Предыдущий
            thumbButtons[0].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[0].iId = 0;
            thumbButtons[0].hIcon = prevIcon;
            wcscpy(thumbButtons[0].szTip, L"Предыдущий");
            thumbButtons[0].dwFlags = THBF_ENABLED;

            // Центральная кнопка: ▶/⏸ Play/Pause
            thumbButtons[1].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[1].iId = 1;
            thumbButtons[1].hIcon = (player->playbackState() == QMediaPlayer::PlayingState) ? pauseIcon : playIcon;
            wcscpy(thumbButtons[1].szTip, L"Воспроизведение/Пауза");
            thumbButtons[1].dwFlags = THBF_ENABLED;

            // Правая кнопка: ⏭ Следующий
            thumbButtons[2].dwMask = THB_TOOLTIP | THB_FLAGS | THB_ICON;
            thumbButtons[2].iId = 2;
            thumbButtons[2].hIcon = nextIcon;
            wcscpy(thumbButtons[2].szTip, L"Следующий");
            thumbButtons[2].dwFlags = THBF_ENABLED;

            hr = pTaskbarList->ThumbBarAddButtons((HWND)winId(), 3, thumbButtons);

            if (SUCCEEDED(hr)) {
                thumbnailToolbarInitialized = true;
            }
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
    thumbButton.iId = 1;  // Кнопка Play/Pause

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
    if (playIcon) {
        DestroyIcon(playIcon);
        playIcon = nullptr;
    }
    if (pauseIcon) {
        DestroyIcon(pauseIcon);
        pauseIcon = nullptr;
    }
    if (nextIcon) {
        DestroyIcon(nextIcon);
        nextIcon = nullptr;
    }
    if (prevIcon) {
        DestroyIcon(prevIcon);
        prevIcon = nullptr;
    }

    if (taskbarList) {
        ((ITaskbarList3*)taskbarList)->Release();
        taskbarList = nullptr;
    }
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
            case 0: // Previous
                onPrevClicked();
                return true;
            case 1: // Play/Pause
                onPlayPauseClicked();
                return true;
            case 2: // Next
                onNextClicked();
                return true;
            }
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    // Инициализируем thumbnail toolbar после показа окна
    if (!thumbnailToolbarInitialized) {
        QTimer::singleShot(100, this, &MainWindow::setupThumbnailToolBar);
    }
}

#ifdef Q_OS_WIN
HICON MainWindow::createIconFromText(const wchar_t* text, int size) {
    HDC hdc = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdc);

    // Создаем bitmap для иконки
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, size, size);
    SelectObject(hdcMem, hBitmap);

    // Настраиваем фон
    RECT rect = {0, 0, size, size};
    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240)); // Светло-серый фон
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);

    // Настраиваем шрифт
    HFONT hFont = CreateFont(
        size - 4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI Symbol"
        );
    SelectObject(hdcMem, hFont);

    // Настраиваем цвет текста
    SetTextColor(hdcMem, RGB(0, 0, 0)); // Черный текст
    SetBkMode(hdcMem, TRANSPARENT);

    // Рисуем текст по центру
    DrawText(hdcMem, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Создаем маску для иконки
    HBITMAP hMask = CreateBitmap(size, size, 1, 1, nullptr);

    // Создаем иконку из bitmap
    ICONINFO iconInfo;
    iconInfo.fIcon = TRUE;
    iconInfo.hbmMask = hMask;
    iconInfo.hbmColor = hBitmap;
    HICON hIcon = CreateIconIndirect(&iconInfo);

    // Очищаем ресурсы
    DeleteObject(hFont);
    DeleteObject(hBitmap);
    DeleteObject(hMask);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdc);

    return hIcon;
}

HICON MainWindow::createPlayIcon() {
    return createIconFromText(L"▶", 16);
}

HICON MainWindow::createPauseIcon() {
    return createIconFromText(L"⏸", 16);
}

HICON MainWindow::createNextIcon() {
    return createIconFromText(L"⏭", 16);
}

HICON MainWindow::createPrevIcon() {
    return createIconFromText(L"⏮", 16);
}
#endif
