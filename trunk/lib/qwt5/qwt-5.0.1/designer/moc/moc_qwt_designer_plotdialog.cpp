/****************************************************************************
** Meta object code from reading C++ file 'qwt_designer_plotdialog.h'
**
** Created: Sat May 5 19:00:04 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qwt_designer_plotdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_designer_plotdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_QwtDesignerPlugin__PlotDialog[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      31,   30,   30,   30, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QwtDesignerPlugin__PlotDialog[] = {
    "QwtDesignerPlugin::PlotDialog\0\0edited(QString)\0"
};

const QMetaObject QwtDesignerPlugin::PlotDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_QwtDesignerPlugin__PlotDialog,
      qt_meta_data_QwtDesignerPlugin__PlotDialog, 0 }
};

const QMetaObject *QwtDesignerPlugin::PlotDialog::metaObject() const
{
    return &staticMetaObject;
}

void *QwtDesignerPlugin::PlotDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QwtDesignerPlugin__PlotDialog))
	return static_cast<void*>(const_cast<PlotDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int QwtDesignerPlugin::PlotDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: edited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void QwtDesignerPlugin::PlotDialog::edited(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
