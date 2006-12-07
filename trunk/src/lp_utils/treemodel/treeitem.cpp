/****************************************************************************
    **
    ** Copyright (C) 2005-2006 Trolltech ASA. All rights reserved.
    **
    ** This file is part of the documentation of the Qt Toolkit.
    **
    ** This file may be used under the terms of the GNU General Public
    ** License version 2.0 as published by the Free Software Foundation
    ** and appearing in the file LICENSE.GPL included in the packaging of
    ** this file.  Please review the following information to ensure GNU
    ** General Public Licensing requirements will be met:
    ** http://www.trolltech.com/products/qt/opensource.html
    **
    ** If you are unsure which license is appropriate for your use, please
    ** review the following information:
    ** http://www.trolltech.com/products/qt/licensing.html or contact the
    ** sales department at sales@trolltech.com.
    **
    ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
    ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
    **
    ****************************************************************************/

/*** This model has little modifications for adaptation, it stay under GPL licence **/

/*
        treeitem.cpp

        A container for items of data supplied by the simple tree model.
*/

#include <QStringList>

#include <iostream>

#include "treeitem.h"

#include "../qstring_char.h"
#include "../lp_custom_types/lp_custom_types.h"

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent)
{
	parentItem = parent;
	itemData = data;
}

// Constructor used by lp_plugin_lister class
TreeItem::TreeItem(const QString _base,
		 const QString _plugin_name,
		 const unsigned long _plugin_uid,
		 const LADSPA_Descriptor *_ladspa_descriptor,
		 TreeItem *parent)
{
        parentItem = parent;

	base = _base;
	plugin_name = _plugin_name;
	plugin_uid = _plugin_uid;
	ladspa_descriptor = _ladspa_descriptor;

        itemData << base;
	itemData << plugin_name;
	if(plugin_uid > 0){
		itemData << (qlonglong)plugin_uid;
	}else{
		itemData << "";
	}
	lp_custom_types custom_var(ladspa_descriptor);
	QVariant tmp_var;
	tmp_var.setValue(custom_var);
	itemData << tmp_var;
}

TreeItem::~TreeItem()
{
        qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
        childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
        return childItems.value(row);
}

int TreeItem::childCount() const
{
        return childItems.count();
}

int TreeItem::columnCount() const
{
        return itemData.count();
}

QVariant TreeItem::data(int column) const
{
        return itemData.value(column);
}

TreeItem *TreeItem::parent()
{
        return parentItem;
}

/*int TreeItem::row() const
{
        if (parentItem)
            return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

        return 0;
}
*/
int TreeItem::row() const
{
        if (parentItem)
            return parentItem->childRow(const_cast<TreeItem*>(this));

        return 0;
}

int TreeItem::childRow(TreeItem *item) const
{
	return childItems.indexOf(item);
}

// Return true if the first column contain the data
bool TreeItem::has_base(QString _base)
{
	if(_base == base){
		return true;
	}
	return false;
}

// Change the plugin name if the UID correspond and return true if found
bool TreeItem::edit_plugin_name(QString _plugin_name, unsigned long _plugin_uid, const LADSPA_Descriptor *_ladspa_descriptor)
{
	if(_plugin_uid == plugin_uid){
		plugin_name = _plugin_name;
		itemData[1] = plugin_name;
		ladspa_descriptor = _ladspa_descriptor;
		lp_custom_types custom_var(ladspa_descriptor);
		QVariant tmp_var;
		tmp_var.setValue(custom_var);
		itemData[3] = tmp_var;
		return true;
	}
	return false;
}


TreeItem *TreeItem::instance()
{
	return this;
}
