#pragma once
#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    bool autoSkipBadTracks() const;
    int defaultVolume() const;

    void setAutoSkip(bool skip);
    void setVolume(int vol);

private:
    QCheckBox* skipCheck_;
    QSpinBox* volumeSpin_;
};

class BadTrackDialog : public QDialog {
    Q_OBJECT
public:
    explicit BadTrackDialog(QWidget* parent = nullptr);
    void setTrack(const QString& path, const QString& error);
    bool skipAlways() const;

private:
    QCheckBox* alwaysSkip_;
};
