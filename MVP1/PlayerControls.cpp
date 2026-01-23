#include "PlayerControls.h"
#include <QHBoxLayout>   // Горизонтальная компоновка
#include <QVBoxLayout>   // Вертикальная компоновка
#include <QMouseEvent>   // События мыши
#include <QStyle>        // Стили Qt

// Реализация обработчика мыши для ClickableSlider
void ClickableSlider::mousePressEvent(QMouseEvent* event) {
    // Проверка, что нажата левая кнопка мыши
    if (event->button() == Qt::LeftButton) {
        // Вычисление значения слайдера на основе позиции клика
        int value = QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                    event->position().x(), width());
        setValue(value);  // установка вычисленного значения
        emit sliderReleased();  // Испускание сигнала, что слайдер отпущен
    }
    // Вызов базовой реализации для стандартного поведения
    QSlider::mousePressEvent(event);
}

// Конструктор PlayerControls
PlayerControls::PlayerControls(QWidget* parent) : QWidget(parent) {
    // Создание кнопок с иконками-эмодзи
    repeatBtn = new QPushButton("🔁");
    shuffleBtn = new QPushButton("🔀");
    prevBtn = new QPushButton("⏮");
    playBtn = new QPushButton("▶");
    // Установление стиля для кнопки play
    playBtn->setStyleSheet("QPushButton { font-size: 24px; min-width: 60px; "
                           "min-height: 60px; border-radius: 30px; }");
    nextBtn = new QPushButton("⏭");

    // Создание слайдера прогресса
    progressSlider = new ClickableSlider(Qt::Horizontal);
    progressSlider->setRange(0, 1000);  // Диапазон 0-1000

    // Создание слайдера громкости
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);    // Диапазон 0-100%
    volumeSlider->setValue(70);        // Начальное значение 70%
    volumeSlider->setFixedWidth(80);   // Фиксированная ширина

    // Создание кнопки управления громкостью
    volumeDownBtn = new QPushButton("−");
    volumeDownBtn->setFixedSize(30, 30);  // Фиксированный размер
    volumeDownBtn->setStyleSheet("QPushButton { border-radius: 15px; font-weight: bold;"
                                 "background: #333; color: #fff; }");

    volumeUpBtn = new QPushButton("+");
    volumeUpBtn->setFixedSize(30, 30);
    volumeUpBtn->setStyleSheet("QPushButton { border-radius: 15px; font-weight: bold; background: #333; color: #fff; }");

    muteBtn = new QPushButton("🔊");
    muteBtn->setFixedSize(30, 30);
    muteBtn->setStyleSheet("QPushButton { border-radius: 15px; background: #333; color: #fff; }");

    // Создание метки для отображения времени
    timeLabel = new QLabel("00:00 / 00:00");
    timeLabel->setStyleSheet("QLabel { color: #000; font-size: 15px; }");

    // Основная вертикальная компоновка
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 10);  // Отступы: лево, верх, право, низ
    mainLayout->setSpacing(5);  // Расстояние между элементами

    // Горизонтальная компоновка для прогресса и времени
    auto* progressLayout = new QHBoxLayout;
    progressLayout->addWidget(progressSlider, 1);  // Растягиваем слайдер
    progressLayout->addWidget(timeLabel);          // Метка времени справа

    // Горизонтальная компоновка для элементов управления
    auto* controlsLayout = new QHBoxLayout;
    controlsLayout->setSpacing(15);  // Расстояние между кнопками

    // Добавляем кнопки в порядке слева направо
    controlsLayout->addWidget(repeatBtn);
    controlsLayout->addWidget(shuffleBtn);
    controlsLayout->addWidget(prevBtn);
    controlsLayout->addWidget(playBtn, 0, Qt::AlignCenter);  // Центрируем кнопку play
    controlsLayout->addWidget(nextBtn);
    controlsLayout->addStretch();  // Растягивающееся пространство
    controlsLayout->addWidget(volumeDownBtn);
    controlsLayout->addWidget(volumeSlider);
    controlsLayout->addWidget(volumeUpBtn);
    controlsLayout->addWidget(muteBtn);

    // Добавляем компоновки в основную
    mainLayout->addLayout(progressLayout);
    mainLayout->addLayout(controlsLayout);

    // Установка стилей для слайдера прогресса
    progressSlider->setStyleSheet(
        "QSlider::groove:horizontal {"      // Стиль желоба слайдера
        "    background: #333;"             // Темно-серый фон
        "    height: 6px;"                  // Высота желоба
        "    border-radius: 3px;"           // Закругленные углы
        "}"
        "QSlider::handle:horizontal {"      // Стиль ручки слайдера
        "    background: #0078d4;"          // Синий цвет
        "    width: 16px;"                  // Ширина ручки
        "    height: 16px;"                 // Высота ручки
        "    border-radius: 8px;"           // Круглая ручка
        "    margin: -5px 0;"               // Отступы чтобы ручка выходила за желоб
        "}"
        "QSlider::handle:horizontal:hover {" // Стиль при наведении
        "    background: #0086f0;"          // Более светлый синий
        "    width: 18px;"                  // Увеличиваем при наведении
        "    height: 18px;"
        "}"
        );

    // Стили для слайдера громкости
    volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    background: #333;"
        "    height: 5px;"
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #0078d4;"
        "    width: 12px;"
        "    height: 12px;"
        "    border-radius: 6px;"
        "    margin: -4px 0;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "    background: #0086f0;"
        "}"
        );

    // ПОДКЛЮЧЕНИЕ СИГНАЛОВ К СЛОТАМ

    // Кнопки управления воспроизведением
    connect(playBtn, &QPushButton::clicked, this, &PlayerControls::playPauseClicked);
    connect(prevBtn, &QPushButton::clicked, this, &PlayerControls::prevClicked);
    connect(nextBtn, &QPushButton::clicked, this, &PlayerControls::nextClicked);
    connect(repeatBtn, &QPushButton::clicked, this, &PlayerControls::repeatClicked);
    connect(shuffleBtn, &QPushButton::clicked, this, &PlayerControls::shuffleClicked);

    // Обработка перемотки слайдером прогресса
    connect(progressSlider, &QSlider::sliderReleased, this, [this]() {
        // Вычисляем позицию в миллисекундах на основе значения слайдера (0-1000)
        qint64 position = static_cast<qint64>(progressSlider->value() / 1000.0 * duration_);
        emit seek(position);  // Испускаем сигнал с новой позицией
    });

    // Изменение громкости
    connect(volumeSlider, &QSlider::valueChanged, this, &PlayerControls::volumeChanged);

    // Кнопки управления громкостью
    connect(volumeDownBtn, &QPushButton::clicked, this, &PlayerControls::onVolumeDownClicked);
    connect(volumeUpBtn, &QPushButton::clicked, this, &PlayerControls::onVolumeUpClicked);
    connect(muteBtn, &QPushButton::clicked, this, &PlayerControls::onMuteClicked);
}

// Установка состояния воспроизведения (play/pause)
void PlayerControls::setPlaying(bool playing) {
    playBtn->setText(playing ? "⏸" : "▶");  // Меняем иконку
}

// Установка позиции трека и обновление слайдера
void PlayerControls::setPosition(qint64 position, qint64 duration) {
    // Проверяем валидность длительности
    if (duration <= 0) {
        qDebug() << "Предупреждение: трек с нулевой длительностью";
        // Не обновляем UI для невалидных треков
        return;
    }

    duration_ = duration;

    progressSlider->blockSignals(true);
    if (duration > 0) {
        int value = static_cast<int>((position * 1000.0) / duration);
        progressSlider->setValue(value);
    } else {
        progressSlider->setValue(0);
    }
    progressSlider->blockSignals(false);

    timeLabel->setText(formatTime(position) + " / " + formatTime(duration));
}

// Установка громкости
void PlayerControls::setVolume(int volume) {
    volumeSlider->blockSignals(true);  // Блокировка сигналов
    volumeSlider->setValue(volume);    // Установка значений
    volumeSlider->blockSignals(false); // Разблокировка
}

// Установка состояния повтора
void PlayerControls::setRepeatState(int state) {
    repeatState_ = state;  // Сохраняем состояние
    // Смена иконки в зависимости от состояния
    switch (state) {
    case 0: repeatBtn->setText("🔁"); break;  // Нет повтора (NONE)
        // case 1: repeatBtn->setText("🔂"); break;  // Повтор одного (ONE)
        // case 2: repeatBtn->setText("🔁"); break;  // Повтор всех (ALL)
    }

    // Стили для активного/неактивного состояния
    QString activeStyle = "QPushButton { background: #0078d4; color: #fff; }";
    QString normalStyle = "QPushButton { background: #333; color: #fff; }";
    repeatBtn->setStyleSheet(state > 0 ? activeStyle : normalStyle);
}

// Установка состояния перемешивания
void PlayerControls::setShuffleState(bool shuffled) {
    isShuffled_ = shuffled;  // Сохранение состояния
    QString activeStyle = "QPushButton { background: #0078d4; color: #fff; }";
    QString normalStyle = "QPushButton { background: #333; color: #fff; }";
    shuffleBtn->setStyleSheet(shuffled ? activeStyle : normalStyle);
}

// Форматирование времени из миллисекунд в строку "мм:сс"
QString PlayerControls::formatTime(qint64 milliseconds) {
    qint64 seconds = milliseconds / 1000;  // Перевод в секунды
    qint64 minutes = seconds / 60;         // Получение минут
    seconds = seconds % 60;                // Остаток - секунды

    // Форматирование в "мм:сс" с ведущими нулями
    return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

// Увеличение громкости на 10%
void PlayerControls::onVolumeUpClicked() {
    int newVolume = volumeSlider->value() + 10;
    if (newVolume > 100) newVolume = 100;  // Не больше 100%
    volumeSlider->setValue(newVolume);
    emit volumeChanged(newVolume);  // Испускание сигнала
}

// Уменьшение громкости на 10%
void PlayerControls::onVolumeDownClicked() {
    int newVolume = volumeSlider->value() - 10;
    if (newVolume < 0) newVolume = 0;  // Не меньше 0%
    volumeSlider->setValue(newVolume);
    emit volumeChanged(newVolume);
}

// Переключение звука (приватный метод)
void PlayerControls::toggleMute() {
    isMuted_ = !isMuted_;  // Инвертирование состояния
    if (isMuted_) {
        volumeBeforeMute_ = volumeSlider->value();  // Сохранение громкости
        muteBtn->setText("🔇");  // Смена на иконку выключенного звука
        emit muteToggled(true);  // Сигнал о выключении
    } else {
        muteBtn->setText("🔊");  // Меняем на иконку включенного звука
        emit muteToggled(false); // Сигнализируем о включении
    }
}

// Обработчик клика по кнопке mute (публичный слот)
void PlayerControls::onMuteClicked() {
    isMuted_ = !isMuted_;
    if (isMuted_) {
        volumeBeforeMute_ = volumeSlider->value();
        muteBtn->setText("🔇");
        emit muteToggled(true);
    } else {
        muteBtn->setText("🔊");
        emit muteToggled(false);
    }
}
