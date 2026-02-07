#include "BadTrackDialog.h"
#include <QFileInfo>

BadTrackDialog::BadTrackDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Повреждённый трек");
    setModal(true);
    resize(400, 200);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // Заголовок
    QLabel* titleLabel = new QLabel("⚠ Обнаружен повреждённый трек");
    titleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #ff4444; }");
    mainLayout->addWidget(titleLabel);

    // Информация о треке
    trackLabel = new QLabel("Трек:");
    trackLabel->setWordWrap(true);
    trackLabel->setStyleSheet("QLabel { font-size: 12px; padding: 5px; }");
    mainLayout->addWidget(trackLabel);

    // Информация об ошибке
    errorLabel = new QLabel("Ошибка:");
    errorLabel->setWordWrap(true);
    errorLabel->setStyleSheet("QLabel { color: #ff6666; font-size: 12px; padding: 5px; background: #ffeeee; border-radius: 5px; }");
    mainLayout->addWidget(errorLabel);

    // Чекбокс для запоминания выбора
    skipCheckBox = new QCheckBox("Всегда пропускать повреждённые треки (можно изменить в Настройках)");
    skipCheckBox->setChecked(false); // По умолчанию не выбрано
    mainLayout->addWidget(skipCheckBox);

    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    skipButton = new QPushButton("Пропустить и продолжить");
    skipButton->setDefault(true);
    skipButton->setStyleSheet("QPushButton { padding: 8px 15px; font-weight: bold; }");
    buttonLayout->addWidget(skipButton);

    stopButton = new QPushButton("Остановить");
    stopButton->setStyleSheet("QPushButton { padding: 8px 15px; }");
    buttonLayout->addWidget(stopButton);

    mainLayout->addLayout(buttonLayout);

    connect(skipButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(stopButton, &QPushButton::clicked, this, &QDialog::reject);
}

void BadTrackDialog::setTrackInfo(const QString& filePath, const QString& error) {
    QFileInfo fileInfo(filePath);
    QString displayName = fileInfo.fileName();

    trackLabel->setText(QString("Трек: %1").arg(displayName));
    errorLabel->setText(QString("Ошибка: %1").arg(error));
}

bool BadTrackDialog::skipAlways() const {
    return skipCheckBox->isChecked();
}
