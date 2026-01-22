/****************************************************************************
** Meta object code from reading C++ file 'temp_monitor.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "temp_monitor.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'temp_monitor.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TempMonitor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x0a,
      23,   12,   12,   12, 0x08,
      37,   12,   12,   12, 0x08,
      51,   12,   12,   12, 0x08,
      66,   12,   12,   12, 0x08,
      90,   12,   12,   12, 0x08,
     115,   12,   12,   12, 0x08,
     142,   12,   12,   12, 0x08,
     155,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_TempMonitor[] = {
    "TempMonitor\0\0closing()\0togglePause()\0"
    "setBrake(int)\0updateCurves()\0"
    "updateTempScale(QRectF)\0"
    "updatePowerScale(QRectF)\0"
    "updateDensityScale(QRectF)\0updateTemp()\0"
    "updateTab(int)\0"
};

void TempMonitor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TempMonitor *_t = static_cast<TempMonitor *>(_o);
        switch (_id) {
        case 0: _t->closing(); break;
        case 1: _t->togglePause(); break;
        case 2: _t->setBrake((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->updateCurves(); break;
        case 4: _t->updateTempScale((*reinterpret_cast< const QRectF(*)>(_a[1]))); break;
        case 5: _t->updatePowerScale((*reinterpret_cast< const QRectF(*)>(_a[1]))); break;
        case 6: _t->updateDensityScale((*reinterpret_cast< const QRectF(*)>(_a[1]))); break;
        case 7: _t->updateTemp(); break;
        case 8: _t->updateTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TempMonitor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TempMonitor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TempMonitor,
      qt_meta_data_TempMonitor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TempMonitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TempMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TempMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TempMonitor))
        return static_cast<void*>(const_cast< TempMonitor*>(this));
    if (!strcmp(_clname, "TempMonitorIf"))
        return static_cast< TempMonitorIf*>(const_cast< TempMonitor*>(this));
    return QWidget::qt_metacast(_clname);
}

int TempMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
