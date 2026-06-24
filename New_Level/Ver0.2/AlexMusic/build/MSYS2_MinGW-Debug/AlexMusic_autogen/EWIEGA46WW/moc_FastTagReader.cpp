/****************************************************************************
** Meta object code from reading C++ file 'FastTagReader.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../FastTagReader.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FastTagReader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN20FastTagReaderWrapperE_t {};
} // unnamed namespace

template <> constexpr inline auto FastTagReaderWrapper::qt_create_metaobjectdata<qt_meta_tag_ZN20FastTagReaderWrapperE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "FastTagReaderWrapper",
        "metadataReady",
        "",
        "filePath",
        "TrackMetadata",
        "metadata",
        "batchProgress",
        "current",
        "total",
        "batchFinished"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'metadataReady'
        QtMocHelpers::SignalData<void(const QString &, const TrackMetadata &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'batchProgress'
        QtMocHelpers::SignalData<void(int, int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { QMetaType::Int, 8 },
        }}),
        // Signal 'batchFinished'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FastTagReaderWrapper, qt_meta_tag_ZN20FastTagReaderWrapperE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject FastTagReaderWrapper::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20FastTagReaderWrapperE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20FastTagReaderWrapperE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN20FastTagReaderWrapperE_t>.metaTypes,
    nullptr
} };

void FastTagReaderWrapper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FastTagReaderWrapper *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->metadataReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<TrackMetadata>>(_a[2]))); break;
        case 1: _t->batchProgress((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->batchFinished(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FastTagReaderWrapper::*)(const QString & , const TrackMetadata & )>(_a, &FastTagReaderWrapper::metadataReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FastTagReaderWrapper::*)(int , int )>(_a, &FastTagReaderWrapper::batchProgress, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FastTagReaderWrapper::*)()>(_a, &FastTagReaderWrapper::batchFinished, 2))
            return;
    }
}

const QMetaObject *FastTagReaderWrapper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FastTagReaderWrapper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20FastTagReaderWrapperE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FastTagReaderWrapper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void FastTagReaderWrapper::metadataReady(const QString & _t1, const TrackMetadata & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void FastTagReaderWrapper::batchProgress(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void FastTagReaderWrapper::batchFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
