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

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <ladspa.h>

class TreeItem
{
public:
        TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
	// Constructor used by lp_plugin_lister class
	TreeItem(const QString _base = 0,
		 const QString _plugin_name = 0,
		 const unsigned long _plugin_uid = 0,
		 const LADSPA_Descriptor *ladspa_descriptor = 0,
		 TreeItem *parent = 0);
        ~TreeItem();

        void appendChild(TreeItem *child);

        TreeItem *child(int row);
        int childCount() const;
        int columnCount() const;
        QVariant data(int column) const;
        int row() const;
        TreeItem *parent();

	// Return true if the first column contain the data
	bool has_base(QString _base);
	// Change the plugin name if the UID correspond and return true if found
	bool edit_plugin_name(QString _plugin_name, unsigned long _plugin_uid, const LADSPA_Descriptor *_ladspa_descriptor = 0);
	// Return the "this" pointer
	TreeItem *instance();

private:
	int childRow(TreeItem *item) const;
        QList<TreeItem*> childItems;
	QList<QVariant>itemData;
        TreeItem *parentItem;
	QString base;
	// Variables used by lp_plugin_lister class
	QString plugin_name;
	unsigned long plugin_uid;
	const LADSPA_Descriptor *ladspa_descriptor;
};

#endif
