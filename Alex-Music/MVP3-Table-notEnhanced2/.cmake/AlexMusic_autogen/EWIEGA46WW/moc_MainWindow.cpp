/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../MainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onPlayPauseClicked",
        "",
        "onNextClicked",
        "onPrevClicked",
        "onRepeatClicked",
        "onShuffleClicked",
        "onSeek",
        "position",
        "onVolumeChanged",
        "volume",
        "onPositionChanged",
        "onDurationChanged",
        "duration",
        "onMediaStatusChanged",
        "QMediaPlayer::MediaStatus",
        "status",
        "onMuteToggled",
        "muted",
        "onRatingChanged",
        "rating",
        "onTableDoubleClicked",
        "row",
        "column",
        "onRatingChangedInTable",
        "onHeaderClicked",
        "playSelectedTrack",
        "onScrollToCurrentClicked",
        "onSearchTextChanged",
        "text",
        "onSortAlphabeticalClicked",
        "onSortStandardClicked",
        "onSortReverseClicked",
        "scanFolder",
        "path",
        "onTrackFound",
        "Track",
        "track",
        "onScanProgress",
        "current",
        "total",
        "currentFile",
        "onScanFinished",
        "totalTracks",
        "showSettingsDialog",
        "showHelpDialog",
        "showHotkeysDialog",
        "showAboutDialog",
        "handleInvalidTrack",
        "filePath",
        "error"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onPlayPauseClicked'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onNextClicked'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPrevClicked'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRepeatClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShuffleClicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSeek'
        QtMocHelpers::SlotData<void(qint64)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 8 },
        }}),
        // Slot 'onVolumeChanged'
        QtMocHelpers::SlotData<void(int)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Slot 'onPositionChanged'
        QtMocHelpers::SlotData<void(qint64)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 8 },
        }}),
        // Slot 'onDurationChanged'
        QtMocHelpers::SlotData<void(qint64)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 13 },
        }}),
        // Slot 'onMediaStatusChanged'
        QtMocHelpers::SlotData<void(QMediaPlayer::MediaStatus)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 15, 16 },
        }}),
        // Slot 'onMuteToggled'
        QtMocHelpers::SlotData<void(bool)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Slot 'onRatingChanged'
        QtMocHelpers::SlotData<void(int)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 20 },
        }}),
        // Slot 'onTableDoubleClicked'
        QtMocHelpers::SlotData<void(int, int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 }, { QMetaType::Int, 23 },
        }}),
        // Slot 'onRatingChangedInTable'
        QtMocHelpers::SlotData<void(int, int)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 22 }, { QMetaType::Int, 20 },
        }}),
        // Slot 'onHeaderClicked'
        QtMocHelpers::SlotData<void(int)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Slot 'playSelectedTrack'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScrollToCurrentClicked'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void(const QString &)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 29 },
        }}),
        // Slot 'onSortAlphabeticalClicked'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSortStandardClicked'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSortReverseClicked'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'scanFolder'
        QtMocHelpers::SlotData<void(const QString &)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 34 },
        }}),
        // Slot 'onTrackFound'
        QtMocHelpers::SlotData<void(const Track &)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 36, 37 },
        }}),
        // Slot 'onScanProgress'
        QtMocHelpers::SlotData<void(int, int, const QString &)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 39 }, { QMetaType::Int, 40 }, { QMetaType::QString, 41 },
        }}),
        // Slot 'onScanFinished'
        QtMocHelpers::SlotData<void(int)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 43 },
        }}),
        // Slot 'showSettingsDialog'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showHelpDialog'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showHotkeysDialog'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showAboutDialog'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleInvalidTrack'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 49 }, { QMetaType::QString, 50 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onPlayPauseClicked(); break;
        case 1: _t->onNextClicked(); break;
        case 2: _t->onPrevClicked(); break;
        case 3: _t->onRepeatClicked(); break;
        case 4: _t->onShuffleClicked(); break;
        case 5: _t->onSeek((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 6: _t->onVolumeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->onPositionChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 8: _t->onDurationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 9: _t->onMediaStatusChanged((*reinterpret_cast< std::add_pointer_t<QMediaPlayer::MediaStatus>>(_a[1]))); break;
        case 10: _t->onMuteToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->onRatingChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->onTableDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 13: _t->onRatingChangedInTable((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->onHeaderClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->playSelectedTrack(); break;
        case 16: _t->onScrollToCurrentClicked(); break;
        case 17: _t->onSearchTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 18: _t->onSortAlphabeticalClicked(); break;
        case 19: _t->onSortStandardClicked(); break;
        case 20: _t->onSortReverseClicked(); break;
        case 21: _t->scanFolder((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->onTrackFound((*reinterpret_cast< std::add_pointer_t<Track>>(_a[1]))); break;
        case 23: _t->onScanProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 24: _t->onScanFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->showSettingsDialog(); break;
        case 26: _t->showHelpDialog(); break;
        case 27: _t->showHotkeysDialog(); break;
        case 28: _t->showAboutDialog(); break;
        case 29: _t->handleInvalidTrack((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 30)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 30;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 30)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 30;
    }
    return _id;
}
QT_WARNING_POP
