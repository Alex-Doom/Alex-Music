#pragma once
#include <QWidget>      // Базовый класс для виджетов
#include <QPushButton>  // Кнопка
#include <QSlider>      // Ползунок
#include <QLabel>       // Текстовая метка
#include <QtGlobal>     // Основные определения Qt

// Предварительное объявление класса для избежания циклических включений
class ClickableSlider;

// панель управления плеером
class PlayerControls : public QWidget {
    Q_OBJECT // Макрос для поддержки сигналов и слотов Qt

public:
    // Конструктор с родительским виджетом
    explicit PlayerControls(QWidget* parent = nullptr);

    // Сетторы для установки состояния элементов управления
    void setPlaying(bool playing);      // состояние воспроизведения
    void setPosition(qint64 position, qint64 duration); // позицию трека
    void setVolume(int volume);         // громкость
    void setRepeatState(int state);     // состояние повтора
    void setShuffleState(bool shuffled); // состояние перемешивания

    // Форматирование времени из миллисекунд в строку "мм:сс"
    QString formatTime(qint64 milliseconds);

// Сигналы - события, которые испускает этот класс
signals:
    void playPauseClicked();  // Нажата кнопка play/pause
    void nextClicked();       // Нажата кнопка следующего трека
    void prevClicked();       // Нажата кнопка предыдущего трека
    void seek(qint64 position); // Пользователь перемотал трек
    void volumeChanged(int volume); // Изменена громкость
    void repeatClicked();     // Нажата кнопка повтора
    void shuffleClicked();    // Нажата кнопка перемешивания
    void muteToggled(bool muted); // Включен/выключен звук

// Публичные слоты - методы, которые можно вызывать извне
public slots:
    void onVolumeDownClicked(); // Уменьшить громкость
    void onVolumeUpClicked();   // Увеличить громкость
    void onMuteClicked();       // Переключить звук (полностью включить/выключить)

private:
    // Указатели на элементы управления
    QPushButton* repeatBtn;    // Кнопки: повтора
    QPushButton* shuffleBtn;   // перемешивания
    QPushButton* prevBtn;      // предыдущего трека
    QPushButton* playBtn;      // воспроизведения/паузы
    QPushButton* nextBtn;      // следующего трека
    ClickableSlider* progressSlider; // Ползунки: прогресса трека
    QSlider* volumeSlider;     // громкости
    QLabel* timeLabel;         // Метка с временем

    // кнопки управления громкостью
    QPushButton* volumeDownBtn; // уменьшения громкости
    QPushButton* volumeUpBtn;   // увеличения громкости
    QPushButton* muteBtn;       // отключения звука

    // Внутренние состояния
    int repeatState_ = 0;      // Текущее состояние повтора (0-2)
    bool isShuffled_ = false;  // Флаг перемешивания
    qint64 duration_ = 0;      // Длительность текущего трека в мс
    bool isMuted_ = false;     // Флаг отключения звука
    int volumeBeforeMute_ = 70; // Громкость до отключения звука

    void toggleMute(); // Приватный метод переключения звука
};

// класс слайдера с поддержкой клика в любом месте
class ClickableSlider : public QSlider {
    Q_OBJECT

public:
    using QSlider::QSlider;  // Наследуем конструкторы базового класса

protected:
    // Переопределяем обработчик события нажатия мыши
    void mousePressEvent(QMouseEvent* event) override;
};
