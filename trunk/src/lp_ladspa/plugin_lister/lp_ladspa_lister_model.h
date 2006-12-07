/***************************************************************************
 *   Copyright (C) 2006 by Philippe   *
 *   nel230@gmail.ch   *
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

#ifndef LP_LADSPA_LISTER_MODEL_H
#define LP_LADSPA_LISTER_MODEL_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <ladspa.h>
#include "../../lp_utils/treemodel/treemodel.h"
#include "../../lp_utils/treemodel/treeitem.h"

class lp_ladspa_lister_model : public TreeModel
{
Q_OBJECT
public:
	lp_ladspa_lister_model(const QList<QVariant> &data, QObject *parent = 0);
	void tests();
	void add_base(QString base, QString new_base);
	void add_plugin(QString base, QString plugin_name, const unsigned long plugin_uid);
	void add_plugin(QString plugin_name, const unsigned long plugin_uid, const LADSPA_Descriptor *ladspa_descriptor = 0);

private:

	TreeItem *unknowBase;
	QString unknowLabel;
};

#endif
