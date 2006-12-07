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

#include "lp_ladspa_lister_model.h"
#include <QtGui>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ladspa.h>

#include <lrdf.h>
#include "../../lp_utils/qstring_char.h"


lp_ladspa_lister_model::lp_ladspa_lister_model(const QList<QVariant> &data, QObject *parent)
	: TreeModel(data, parent)
{
	// Add headers
	//QList<QVariant> tmp_List;
	//tmp_List <<  << "" << "" << "";
	unknowBase = new TreeItem(QObject::tr("Unsorted"), "", 0lu, 0, rootItem);
	rootItem->appendChild(unknowBase);
}

// TESTS
void lp_ladspa_lister_model::tests()
{
	//QList<TreeItem*>items;
	add_base("Utilities", "");
	add_base("", "Utilities");
	add_base("", "Utilities");
	add_base("Utilities", "Testers");
	add_base("Utilities", "Testers");

	add_plugin("", "TFGHGF", 4557);
	add_plugin("Utilities", "Utils plugin", 1054);
	add_plugin("", "Utils plugin", 1055);


	add_plugin("Reverbes", "TC 1", 5);
	add_plugin("Reverbes", "SPX", 8);
	add_plugin("Reverbes", "BTX", 9);
	add_plugin("Reverbes", "XXX", 3);
	add_plugin("Reverbes", "ATX", 15);
	add_plugin("Spec Revs", "Une TC custom", 1);
}

void lp_ladspa_lister_model::add_plugin(QString base, QString plugin_name, const unsigned long plugin_uid)
{
	// Rechercher l'instance qui comporte la chaine base (Catégorie)
	int i;
	// If plugin UID is Null, return
	if(plugin_uid == 0){
		return;
	}
	// If no base is specified, class to unknow
	if(base.length() < 1){
	//	std::cout << "Class to unknowBase\n";
		TreeItem *item = new TreeItem(unknowLabel, plugin_name, plugin_uid, 0, unknowBase);
		unknowBase->appendChild(item);
		items << item;
		return;
	}
	// Search the base (Category) pointer
	for(i=0; i<items.count(); i++){
		if(items[i]->has_base(base)){
			TreeItem *item = new TreeItem("", plugin_name, plugin_uid, 0,items[i]->instance());
			items[i]->instance()->appendChild(item);
			items << item;
			return;
		}
	}
	// Aucune base (Categorie) trouvée, on ajoute
	TreeItem *item = new TreeItem(base, plugin_name, plugin_uid, 0,rootItem);
	rootItem->appendChild(item);
	items << item;
}

void lp_ladspa_lister_model::add_plugin(QString plugin_name, const unsigned long plugin_uid, const LADSPA_Descriptor *ladspa_descriptor)
{
	int i;

	// If plugin UID is Null, return
	if(plugin_uid == 0){
		return;
	}
	// Edit the imtem pointer that contains the plugin_uid
	for(i=0; i<items.count(); i++){
		if((items[i]->edit_plugin_name(plugin_name, plugin_uid, ladspa_descriptor)) == true){
			return;
		}
	}
	// Not found - class to unknow
	TreeItem *item = new TreeItem(unknowLabel, plugin_name, plugin_uid, ladspa_descriptor, unknowBase);
	unknowBase->appendChild(item);
	items << item;
}

void lp_ladspa_lister_model::add_base(QString base, QString new_base)
{
	// Rechercher l'instance qui comporte la chaine base (Catégorie)
	int i;
	// If new_base is empty, return
	if(new_base.length() < 1){
		return;
	}
	// If new_base allready exists, quit
	for(i=0; i<items.count(); i++){
		if(items[i]->has_base(new_base)){
			return;
		}
	}
	// If base is not set, add to rootItem
	if(base.length() < 1){
		TreeItem *item = new TreeItem(new_base, "", 0lu, 0,rootItem);
		rootItem->appendChild(item);
		items << item;
		return;
	}
	// Search if a base (parent category) exists
	for(i=0; i<items.count(); i++){
		if(items[i]->has_base(base)){
			TreeItem *item = new TreeItem(new_base, "", 0lu, 0, items[i]->instance());
			items[i]->instance()->appendChild(item);
			items << item;
			return;
		}
	}
	// No parent base, add
	TreeItem *item = new TreeItem(new_base, "", 0lu, 0,rootItem);
	rootItem->appendChild(item);
	items << item;
}
