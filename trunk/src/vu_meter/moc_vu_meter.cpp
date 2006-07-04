/****************************************************************************
** vu_meter meta object code from reading C++ file 'vu_meter.h'
**
** Created: Tue Jul 4 22:11:46 2006
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.6   edited Mar 8 17:43 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "vu_meter.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *vu_meter::className() const
{
    return "vu_meter";
}

QMetaObject *vu_meter::metaObj = 0;
static QMetaObjectCleanUp cleanUp_vu_meter( "vu_meter", &vu_meter::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString vu_meter::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vu_meter", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString vu_meter::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "vu_meter", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* vu_meter::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"vu_meter", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_vu_meter.setMetaObject( metaObj );
    return metaObj;
}

void* vu_meter::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "vu_meter" ) )
	return this;
    if ( !qstrcmp( clname, "QThread" ) )
	return (QThread*)this;
    return QWidget::qt_cast( clname );
}

bool vu_meter::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool vu_meter::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool vu_meter::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool vu_meter::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
