#pragma once
#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include "Playlist.h"
#include "TrackValidator.h"

class PlayerControls;
class SettingsDialog;
class RatingDelegate;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onPlayPause();
    void onNext();
    void onPrev();
    void onSeek(qint64 pos);
    void onVolumeChanged(int vol);
    void onMute(bool muted);
    void onRepeat();
    void onShuffle();
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onTableDoubleClick(int row, int col);
    void onSearch(const QString& text);
    void onRatingInTable(int row, int rating);
    void onSort(int column);
    void scrollToCurrent();
    void openFolder();
    void showSettings();
    void loadFolder(const QString& path);

private:
    void setupUI();
    void setupShortcuts();
    void createMenu();
    void updateUI();
    void highlightCurrentRow();
    void playTrackAt(int row);
    bool validateAndPlay(const QString& path, bool forward);

    // Thumbnail toolbar (Windows)
    void setupThumbnailToolbar();
    void updateThumbnailButtons();

    Playlist playlist_;
    QMediaPlayer* player_;
    QAudioOutput* audio_;
    TrackValidator validator_;

    // UI
    PlayerControls* controls_;
    QTableWidget* table_;
    QLineEdit* searchEdit_;
    QLabel* coverLabel_;
    QLabel* titleLabel_;
    QLabel* artistLabel_;
    SettingsDialog* settingsDlg_;
    RatingDelegate* ratingDelegate_;

    // Состояние
    int volumeBeforeMute_ = 70;
    bool autoSkipBad_ = false;
    bool isScanning_ = false;

#ifdef Q_OS_WIN
    void* taskbarList_ = nullptr;
    bool thumbBarReady_ = false;
#endif
};
