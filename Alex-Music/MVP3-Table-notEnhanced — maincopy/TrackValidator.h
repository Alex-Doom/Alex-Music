#pragma once
#include <QString>
#include <QObject>

class TrackValidator : public QObject {
    Q_OBJECT
public:
    explicit TrackValidator(QObject* parent = nullptr);
    bool validate(const QString& path);
    QString lastError() const { return error_; }

private:
    QString error_;
    bool checkDuration(const QString& path, qint64& duration);
};
