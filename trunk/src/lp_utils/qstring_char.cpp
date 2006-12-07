#include "qstring_char.h"

qstring_char::qstring_char(QString qstr)
{
	pv_cstr = 0;
	pv_cstr = (char*)malloc(qstr.length()+1);
	strcpy(pv_cstr, qstr.toAscii());
}

qstring_char::qstring_char()
{
	pv_cstr = 0;
}

char *qstring_char::to_char()
{
	if(pv_cstr != 0){
		return pv_cstr;
	}
	return 0;
}

int qstring_char::length()
{
	return strlen(pv_cstr);
}
