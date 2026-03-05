#pragma once
#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class BadTrackDialog : public QDialog {
    Q_OBJECT
public:
    explicit BadTrackDialog(QWidget* parent = nullptr);

    void setTrackInfo(const QString& filePath, const QString& error);
    bool skipAlways() const;

private:
    QLabel* trackLabel;
    QLabel* errorLabel;
    QCheckBox* skipCheckBox;
    QPushButton* skipButton;
    QPushButton* stopButton;
};
