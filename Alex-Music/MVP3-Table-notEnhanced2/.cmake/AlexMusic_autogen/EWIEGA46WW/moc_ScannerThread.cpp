/****************************************************************************
** Meta object code from reading C++ file 'ScannerThread.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ScannerThread.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScannerThread.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13ScannerThreadE_t {};
} // unnamed namespace

template <> constexpr inline auto ScannerThread::qt_create_metaobjectdata<qt_meta_tag_ZN13ScannerThreadE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScannerThread",
        "trackFound",
        "",
        "Track",
        "track",
        "scanProgress",
        "current",
        "total",
        "currentFile",
        "scanFinished",
        "totalTracks",
        "scanError",
        "error"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'trackFound'
        QtMocHelpers::SignalData<void(const Track &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'scanProgress'
        QtMocHelpers::SignalData<void(int, int, const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 }, { QMetaType::Int, 7 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'scanFinished'
        QtMocHelpers::SignalData<void(int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Signal 'scanError'
        QtMocHelpers::SignalData<void(const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScannerThread, qt_meta_tag_ZN13ScannerThreadE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScannerThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ScannerThreadE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ScannerThreadE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13ScannerThreadE_t>.metaTypes,
    nullptr
} };

void ScannerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScannerThread *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->trackFound((*reinterpret_cast< std::add_pointer_t<Track>>(_a[1]))); break;
        case 1: _t->scanProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 2: _t->scanFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->scanError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScannerThread::*)(const Track & )>(_a, &ScannerThread::trackFound, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScannerThread::*)(int , int , const QString & )>(_a, &ScannerThread::scanProgress, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScannerThread::*)(int )>(_a, &ScannerThread::scanFinished, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ScannerThread::*)(const QString & )>(_a, &ScannerThread::scanError, 3))
            return;
    }
}

const QMetaObject *ScannerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScannerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ScannerThreadE_t>.strings))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int ScannerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ScannerThread::trackFound(const Track & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ScannerThread::scanProgress(int _t1, int _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3);
}

// SIGNAL 2
void ScannerThread::scanFinished(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ScannerThread::scanError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
