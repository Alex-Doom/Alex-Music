#pragma once
#include <QString>
#include <QObject>

class TrackValidator : public QObject {
    Q_OBJECT
public:
    explicit TrackValidator(QObject* parent = nullptr);

    // Проверяет, валиден ли трек
    bool validateTrack(const QString& filePath);

    // Возвращает описание ошибки
    QString lastError() const { return lastError_; }

    // Получает длительность трека в миллисекундах
    qint64 getDuration(const QString& filePath);

signals:
    void validationFailed(const QString& filePath, const QString& error);

private:
    QString lastError_;
};
