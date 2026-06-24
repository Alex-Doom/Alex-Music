/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../MainWindow.h"
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
        "onTrackTableDoubleClicked",
        "row",
        "column",
        "onMuteToggled",
        "muted",
        "onRatingChanged",
        "rating",
        "onSearchTextChanged",
        "text",
        "simpleHighlight",
        "searchText",
        "onTableHeaderClicked",
        "onSortStandardClicked",
        "onScrollToCurrentClicked",
        "playSelectedTrack",
        "showSettingsDialog"
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
        // Slot 'onTrackTableDoubleClicked'
        QtMocHelpers::SlotData<void(int, int)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 18 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'onMuteToggled'
        QtMocHelpers::SlotData<void(bool)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 21 },
        }}),
        // Slot 'onRatingChanged'
        QtMocHelpers::SlotData<void(int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Slot 'onSearchTextChanged'
        QtMocHelpers::SlotData<void(const QString &)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 25 },
        }}),
        // Slot 'simpleHighlight'
        QtMocHelpers::SlotData<QString(const QString &, const QString &) const>(26, 2, QMC::AccessPrivate, QMetaType::QString, {{
            { QMetaType::QString, 25 }, { QMetaType::QString, 27 },
        }}),
        // Slot 'onTableHeaderClicked'
        QtMocHelpers::SlotData<void(int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'onSortStandardClicked'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScrollToCurrentClicked'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'playSelectedTrack'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showSettingsDialog'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
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
        case 10: _t->onTrackTableDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->onMuteToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->onRatingChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->onSearchTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: { QString _r = _t->simpleHighlight((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->onTableHeaderClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->onSortStandardClicked(); break;
        case 17: _t->onScrollToCurrentClicked(); break;
        case 18: _t->playSelectedTrack(); break;
        case 19: _t->showSettingsDialog(); break;
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
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 20;
    }
    return _id;
}
QT_WARNING_POP
