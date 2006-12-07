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
        treemodel.cpp

        Provides a simple tree model to show how to create and use hierarchical
        models.
*/

#include <QtGui>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel(const QList<QVariant> &data, QObject *parent)
        : QAbstractItemModel(parent)
{
        rootItem = new TreeItem(data);
}

TreeModel::~TreeModel()
{
	delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
        if (parent.isValid())
            return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
        else
            return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
        if (!index.isValid())
            return QVariant();

        if (role != Qt::DisplayRole)
            return QVariant();

        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

        return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return rootItem->data(section);

        return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
        TreeItem *parentItem;

        if (!parent.isValid())
            parentItem = rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        TreeItem *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
        if (!index.isValid())
            return QModelIndex();

        TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
        TreeItem *parentItem = childItem->parent();

        if (parentItem == rootItem)
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
        TreeItem *parentItem;

        if (!parent.isValid())
            parentItem = rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        return parentItem->childCount();
}

/*bool TreeModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
        QVariant leftData = sourceModel()->data(left);
        QVariant rightData = sourceModel()->data(right);

        if (leftData.type() == QVariant::DateTime) {
            return leftData.toDateTime() < rightData.toDateTime();
        } else {
            return QString::localeAwareCompare(leftData.toString(),
                                               rightData.toString()) < 0;
        }
}
*/
