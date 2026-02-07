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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
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

    QWidget* ratingWidget = new QWidget;
    QHBoxLayout* ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    ratingLayout->setSpacing(5);

    star1 = new QPushButton("☆");
    star2 = new QPushButton("☆");
    star3 = new QPushButton("☆");
    star4 = new QPushButton("☆");
    star5 = new QPushButton("☆");

    QString starStyle = "QPushButton { "
                        "background: transperant; "
                        "border: none; "
                        "font-size: 24px; "
                        "color: #ffcc00; "
                        "min-width: 30px; "
                        "min-height: 30px; "
                        "}"
                        "QPushButton:hover { color: #ffdd44; } ";

    star1->setStyleSheet(starStyle);
    star2->setStyleSheet(starStyle);
    star3->setStyleSheet(starStyle);
    star4->setStyleSheet(starStyle);
    star5->setStyleSheet(starStyle);

    ratingLayout->addWidget(star1);
    ratingLayout->addWidget(star2);
    ratingLayout->addWidget(star3);
    ratingLayout->addWidget(star4);
    ratingLayout->addWidget(star5);
    ratingLayout->addStretch();

    leftLayout->addWidget(albumLabel);
    leftLayout->addWidget(artistLabel);
    leftLayout->addWidget(ratingWidget);

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

    connect(star1, &QPushButton::clicked, this, &MainWindow::onOneStarsClicked);
    connect(star2, &QPushButton::clicked, this, &MainWindow::onTwoStarsClicked);
    connect(star3, &QPushButton::clicked, this, &MainWindow::onThreeStarsClicked);
    connect(star4, &QPushButton::clicked, this, &MainWindow::onFourStarsClicked);
    connect(star5, &QPushButton::clicked, this, &MainWindow::onFiveStarsClicked);

    QString defaultFolder = "C:\\Users\\User\\Music";
    if (QDir(defaultFolder).exists()) {
        scanFolder(defaultFolder);
    }
}


void MainWindow::onOneStarsClicked() {
    if (playlist.setCurrentTrackRating(1.0)) {
        updateStarsRating();
    }
}

void MainWindow::onTwoStarsClicked() {
    if (playlist.setCurrentTrackRating(2.0)) {
        updateStarsRating();
    }
}

void MainWindow::onThreeStarsClicked() {
    if (playlist.setCurrentTrackRating(3.0)) {
        updateStarsRating();
    }
}

void MainWindow::onFourStarsClicked() {
    if (playlist.setCurrentTrackRating(4.0)) {
        updateStarsRating();
    }
}

void MainWindow::onFiveStarsClicked() {
    if (playlist.setCurrentTrackRating(5.0)) {
        updateStarsRating();
    }
}

void MainWindow::updateStarsRating() {
    auto current = playlist.current();
    if (!current) return;

    double rating = current->rating();

    star1->setText(rating >=1 ? "★" : "☆");
    star2->setText(rating >=2 ? "★" : "☆");
    star3->setText(rating >=3 ? "★" : "☆");
    star4->setText(rating >=4 ? "★" : "☆");
    star5->setText(rating >=5 ? "★" : "☆");
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
                           title.toStdString(), "Music for imaginary movies", 0.0));

        trackList->addItem(QString("%1. %2 - %3").arg(index++).arg(artist).arg(title));
    }

    if (!playlist.all().empty()) {
        playlist.setCurrent(0);
        updateUI();
    }
}

void MainWindow::playCurrentTrack() {
    auto current = playlist.current();
    if (!current) return;

    player->setSource(QUrl::fromLocalFile(QString::fromStdString(current->path())));
    player->play();
    controls->setPlaying(true);
    updateUI();
}

void MainWindow::restartCurrentTrack() {
    player->setPosition(0);
    player->play();
}

void MainWindow::updateUI() {
    auto current = playlist.current();
    if (!current) return;

    // Получаем обложку трека (только 2 варианта: из MP3 или default.jpg)
    QImage coverImage = current->getCoverImage();

    if (!coverImage.isNull()) {
        // Обложка найдена - масштабируем и отображаем
        QPixmap coverPixmap = QPixmap::fromImage(coverImage)
                                  .scaled(coverLabel->width(), coverLabel->height(),
                                          Qt::KeepAspectRatio, Qt::SmoothTransformation);
        coverLabel->setPixmap(coverPixmap);
        coverLabel->setText(""); // Убираем текст
    } else {
        // Обложка не найдена - показываем заглушку с текстом
        QPixmap coverPixmap(coverLabel->width(), coverLabel->height());
        coverPixmap.fill(Qt::darkGray);
        coverLabel->setPixmap(coverPixmap);
        coverLabel->setText("No Cover");
        coverLabel->setStyleSheet("QLabel { background: #222; border: 2px solid #444; border-radius: 10px; color: #fff; font-size: 12px; }");
    }

    // Обновляем информацию о треке
    albumLabel->setText(QString::fromStdString(current->title()));
    artistLabel->setText(QString::fromStdString(current->artist()));

    updateStarsRating();

    // Выделение текущего трека
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
