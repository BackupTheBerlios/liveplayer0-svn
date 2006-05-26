/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <iostream>
#include "LP_ladspa_manager.h"
#include "LP_ladspa_plugin.h"

void ladspa_manager_dlg::init() 
{
    // Obtenir une instance du plugin manager
    llm = new LP_ladspa_manager;
    
    // Parameters of lw_plugin_list
    lw_plugin_list->setColumnText(0, tr("Plugin ID"));
    lw_plugin_list->addColumn(tr("Plugin Name"));
    lw_plugin_list->addColumn(tr("Path"));
    lw_plugin_list->clear();
    
    // Parameters for lw_used_plugin
    lw_used_plugin->setColumnText(0, tr("Plugin ID"));
    lw_used_plugin->addColumn(tr("Plugin Name"));
    lw_used_plugin->addColumn(tr("Path"));
    lw_used_plugin->addColumn(tr("Plugin index"));
    lw_used_plugin->clear();
    
    // used plugins
    used_plugins = 1;
    int i;
    for(i=0; i<20; i++){
	used_plugin[i] = 0;
    }
}
//
int ladspa_manager_dlg::set_audio_params(int channels, int sample_rate, int buffer_len)
{
	// store passed vars in class vars
	if(channels < 0){
		std::cerr << "ladspa_manager::ladspa_manager: invalid nb of channels\n";
	}
	pv_channels = channels;
	if(sample_rate < 1000){
		std::cerr << "ladspa_manager::ladspa_manager: invalid sample_rate\n";
	}
	pv_sample_rate = sample_rate;
	if(buffer_len < 2){
		std::cerr << "ladspa_manager::ladspa_manager: invalid buffer len\n";
	}
	pv_buffer_len = buffer_len;
	return 0;
}

void ladspa_manager_dlg::destroy()
{
    // Suppression de l'instance du manager
    delete llm;
}

// List aviable plugins -> lw_list_plugin
int ladspa_manager_dlg::list_plugin_col()
{
       // Lister les plugins
    long i = 0;
    unsigned long y = 0;
    char *path = strdup("/usr/lib/ladspa");
    char *plug_name[LP_MAX_PLUGIN];
    unsigned long plug_id[LP_MAX_PLUGIN];
    char plug_path[NAME_MAX+1];
    
    // Clear the list content
    lw_plugin_list->clear();
    
    while(( i = llm->get_plugin_name(path, plug_name, &plug_id[0], &plug_path[0])) >= 0)
    {
	for(y=0; y<(unsigned long)i; y++)
	{
	    if( y >= LP_MAX_PLUGIN){
		std::cerr << "ladspa_manager_dlg::list_plugin_col: y is to big, max is: " << LP_MAX_PLUGIN-1 << std::endl;
		return -1;
	    }
	   new QListViewItem( lw_plugin_list, QString::number(plug_id[y]), 
			      plug_name[y],
			     plug_path);
	}
	for(i=0; (unsigned long) i < LP_MAX_PLUGIN ; i++){
	    delete plug_name[i];
	    plug_name[i] = 0;
	}
    }

    return 0;
}

// use selected plugin
int ladspa_manager_dlg::get_lw_plugin()
{    
    // Vars
    QString snb;
    unsigned long nb = 0;
    int plug_idx = 0;
    QString qs;
    char *plug_dir = 0;
    
    // Get selected plugin ID
    snb = lw_used_plugin->selectedItem()->text(0);
    if(snb != 0){
	// Get selected plugin's UniqueID
	nb = snb.toInt();
	// Get selected plugin's path
	qs= lw_used_plugin->selectedItem()->text(2);
	plug_dir = new char[qs.length() + 1];
	strcpy(plug_dir, qs.ascii() );
	//LP_ladspa_plugin *llpl = new LP_ladspa_plugin(plug_dir);
	// Find a free idx
	int i;
	for(i=0; i<20; i++){
	    // search in list
	    
	}
	snb = lw_used_plugin->selectedItem()->text(3);
	plug_idx = snb.toInt();
	    if(used_plugin[plug_idx] != 0) {
		// plugin allready used
		return 0;
	    }
	    
	used_plugin[plug_idx] = new LP_ladspa_plugin_dlg(0, "TEST", 0, plug_dir);
	//llpl->init_plugin(this, "TEST", nb);
	//used_plugin[plug_idx]->init_plugin(this, "Test", nb, pv_channels, pv_sample_rate, pv_buffer_len);
	used_plugin[plug_idx]->init_plugin( nb, pv_channels, pv_sample_rate, pv_buffer_len);

    }
    return 0;
}


int ladspa_manager_dlg::select_plug_from_list()
{
        // Test if possible to add one plugin to used list
    if(used_plugins >= 20){
	std::cerr << "ladspa_manager_dlg::get_lw_plugin: to much plugins used\n";
	return -1;
    }
    // Store selected item from lw_plugin_list to lw_used_plugin
   QListViewItem *qlv = lw_plugin_list->selectedItem();
   new QListViewItem(lw_used_plugin, qlv->text(0),
		     qlv->text(1),
		     qlv->text(2),
		     QString::number(used_plugins) ); 
   used_plugins++;
    return 0;
}

// Remove plugin from used plugs
int ladspa_manager_dlg::rem_lw_used_plug_item()
{
    // Find plugin and delete then, then delete list entry
    if(lw_used_plugin->selectedItem() == 0){
	return 0;
    }
    QString snb;
    int plug_idx = 0;
    snb = lw_used_plugin->selectedItem()->text(3);
    plug_idx = snb.toInt();
    if(used_plugin[plug_idx] != 0){
	delete used_plugin[plug_idx];
	used_plugin[plug_idx] = 0;
	if(used_plugins > 0){
	    used_plugins--;
	}
    }
    
    delete lw_used_plugin->selectedItem();
}



int ladspa_manager_dlg::run_plugins(float *buffer)
{
    // For each used plugin, if active, call run
    int i;
    for(i=0; i<20; i++){
	if(used_plugin[i] != 0){
/*	    if(used_plugin[i]->get_active_state() == TRUE){
		std::cout << "Run.................\n";
		used_plugin[i]->run(buffer);
	    }
*/	}
    }
    return 0;
}
