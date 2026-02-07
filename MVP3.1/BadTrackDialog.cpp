#include "BadTrackDialog.h"
#include <QFileInfo>

BadTrackDialog::BadTrackDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Повреждённый трек");
    setModal(true);
    resize(400, 200);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    trackLabel = new QLabel("Трек:");
    trackLabel->setWordWrap(true);
    mainLayout->addWidget(trackLabel);

    errorLabel = new QLabel("Ошибка:");
    errorLabel->setWordWrap(true);
    errorLabel->setStyleSheet("color: red;");
    mainLayout->addWidget(errorLabel);

    skipCheckBox = new QCheckBox("Всегда пропускать повреждённые треки");
    mainLayout->addWidget(skipCheckBox);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    skipButton = new QPushButton("Пропустить и продолжить");
    skipButton->setDefault(true);
    buttonLayout->addWidget(skipButton);

    stopButton = new QPushButton("Остановить");
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
