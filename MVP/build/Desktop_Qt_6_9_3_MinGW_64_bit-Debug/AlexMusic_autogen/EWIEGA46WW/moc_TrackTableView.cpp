/****************************************************************************
** Meta object code from reading C++ file 'TrackTableView.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../MVP1/TrackTableView.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrackTableView.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14TrackTableViewE_t {};
} // unnamed namespace

template <> constexpr inline auto TrackTableView::qt_create_metaobjectdata<qt_meta_tag_ZN14TrackTableViewE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TrackTableView",
        "trackDoubleClicked",
        "",
        "size_t",
        "index",
        "trackAddedToQueue",
        "trackRatingChanged",
        "rating",
        "onDoubleClicked",
        "QModelIndex",
        "onCustomContextMenu",
        "pos"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'trackDoubleClicked'
        QtMocHelpers::SignalData<void(size_t)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'trackAddedToQueue'
        QtMocHelpers::SignalData<void(size_t)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'trackRatingChanged'
        QtMocHelpers::SignalData<void(size_t, double)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Double, 7 },
        }}),
        // Slot 'onDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 9, 4 },
        }}),
        // Slot 'onCustomContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TrackTableView, qt_meta_tag_ZN14TrackTableViewE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TrackTableView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTableView::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TrackTableViewE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TrackTableViewE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14TrackTableViewE_t>.metaTypes,
    nullptr
} };

void TrackTableView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrackTableView *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->trackDoubleClicked((*reinterpret_cast< std::add_pointer_t<size_t>>(_a[1]))); break;
        case 1: _t->trackAddedToQueue((*reinterpret_cast< std::add_pointer_t<size_t>>(_a[1]))); break;
        case 2: _t->trackRatingChanged((*reinterpret_cast< std::add_pointer_t<size_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->onDoubleClicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 4: _t->onCustomContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TrackTableView::*)(size_t )>(_a, &TrackTableView::trackDoubleClicked, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrackTableView::*)(size_t )>(_a, &TrackTableView::trackAddedToQueue, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrackTableView::*)(size_t , double )>(_a, &TrackTableView::trackRatingChanged, 2))
            return;
    }
}

const QMetaObject *TrackTableView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TrackTableView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TrackTableViewE_t>.strings))
        return static_cast<void*>(this);
    return QTableView::qt_metacast(_clname);
}

int TrackTableView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void TrackTableView::trackDoubleClicked(size_t _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void TrackTableView::trackAddedToQueue(size_t _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void TrackTableView::trackRatingChanged(size_t _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}
QT_WARNING_POP
