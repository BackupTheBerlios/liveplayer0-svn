/***************************************************************************
 *   Copyright (C) 2006 by Philippe   *
 *   nel230@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef QSTRING_CHAR_H
#define QSTRING_CHAR_H

#include <QString>

/// Pass the QString to constructor
/// A allocated poiter is returned with
/// qstring_char::to_char(),
/// but you must delete it yourself with free()
/// Use a tmp pointer if you pass the char* to a function
/// Exemple:
/// QString qs("TEST");
/// qstring_char qsc(qs);
/// char *tmp = qsc.to_char();
/// std::cout << tmp << std::endl;
/// free(tmp);

class qstring_char
{
public:
	qstring_char(QString qstr);
	qstring_char();
	char *to_char();
 	int length();
private:
	char *pv_cstr;
}; 

#endif
