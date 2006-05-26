#include "LP_ladspa_manager.h"

/******************************* LP_ladspa_manager impllementation **************************************************/

// Contructeur
LP_ladspa_manager::LP_ladspa_manager()
{
	// Directory handler
	dh = new dir_handle;
	dh->dp = 0;
	//pv_ui_instance = 0;

	// Set audio params to 0
	pv_channels = 0;
	pv_sample_rate = 0;
	pv_buffer_len = 0;

	// Set plugins ptr to 0
	unsigned long ul;
	for(ul=0; ul<LP_MAX_PLUGIN; ul++){
		pv_plugins[ul] = 0;
	}
}

LP_ladspa_manager::~LP_ladspa_manager()
{
	// If a directory is open, close it
	if(dh->dp != 0){
		pv_close_dir();
		dh->dp = 0;
	}
	delete dh;

	// Delete plugins instances
	unsigned long ul;
	for(ul=0; ul<LP_MAX_PLUGIN; ul++){
		if(pv_plugins[ul] != 0){
			delete pv_plugins[ul];
			pv_plugins[ul] = 0;
		}
	}

}

/// Retourne le nom du plugin demande, 0 si err ou fin de liste
/// Le repertoire passe en argument sera utilise pour la rechrche
/// Note: La variable out_name est allouee par la fonctio, mais il faut
/// la liberer apres avoir utilise cette fonction.
long LP_ladspa_manager::get_plugin_name(char *path, char *out_name[LP_MAX_PLUGIN], unsigned long *UniqueID, char *plugin_path)
{
	// Variables
	unsigned long i = 0;
	int ret = 0;
	for(i=0; i < LP_MAX_PLUGIN ; i++){
		out_name[i] = new char[NAME_MAX];
	}
	i=0;
	//UniqueID = new unsigned long[100];

	// list plugins in given path
	if((path != 0) && (strlen(path) > 0)) {
		// Open dir if not allready opened
		if(dh->dp == 0){
			if((ret = (pv_open_dir(path))) < 0){
				return -1;
			}
		}
		// List content
		if((ret = (pv_get_dir_entry())) != LP_DIR_END){
			if(ret == LP_DIR_ENTRY_FOUND){
				LP_ladspa_plugin *llpl = new LP_ladspa_plugin(dh->out_path);
				for(i = 0; (llpl->get_plugin_name(i))!= 0; i++){
				//for(i = 0; i < 2; i++){
					// Give all plugins name found in lib
					strcpy(out_name[i], llpl->get_plugin_name(i));
					if(UniqueID != 0){
						UniqueID[i] = llpl->get_plugin_ID(i);
					}
				}
				// Give the plugin path
				if(plugin_path != 0){
					strcpy(plugin_path, dh->out_path);
				}
				delete llpl;
			}
			if(ret < 0){
				std::cerr << "LP_ladspa_manager::list_plugins: call to pv_get_dir_entry failed\n";
				return -1;
			}
		}else{
			// end of directory
			if((ret = (pv_close_dir())) < 0){
				return -1;
			}
			return -1;
		}

	}else{
		// Bad args
		std::cerr << "LP_ladspa_manager::get_plugin_name: path was not defined\n";
		return -1;
	}
	// return numbers of plugins found in lib
	return i;
	// Liberer variables cree

}

// add a plugin and return it's instance
LP_ladspa_plugin *LP_ladspa_manager::add_plugin(unsigned long index, unsigned long ID, char *path)
{
	if(index >= LP_MAX_PLUGIN){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: index is too big !\n";
		return 0;
	}
	if((path == 0)||(strlen(path) < 1)){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: path is not set\n";
		return 0;
	}
	if(pv_channels < 1){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: channels can't be < 1\n";
		return 0;
	}
	if(pv_sample_rate == 0){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: sample rate is not set\n";
		return 0;
	}
	if(pv_buffer_len == 0){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: buffer len is not set\n";
		return 0;
	}
	// set a new plugin instance
	pv_plugins[index] = new LP_ladspa_plugin(path);
	if(pv_plugins[index] == 0){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: plugin's instance failed\n";
		return 0;
	}
	// Initialise the plugin
	if((pv_plugins[index]->init_plugin(ID, pv_channels, pv_sample_rate, pv_buffer_len))<0){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: plugin's init falied\n";
		return 0;
	}
	return pv_plugins[index];
}

// Remove a plugin instance
int LP_ladspa_manager::rem_plugin(unsigned long index)
{
	if(index >= LP_MAX_PLUGIN){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: index is too big !\n";
		return -1;
	}
	if(pv_plugins[index] == 0){
		std::cerr << "LP_ladspa_plugin *LP_ladspa_manager::add_plugin: plugin is Null\n";
		return -1;
	}
	delete pv_plugins[index];
	pv_plugins[index] = 0;
	return 0;
}

int LP_ladspa_manager::list_plugins(char *path)
{
	int i, ret;

	// list plugins in given path
	if((path != 0) && (strlen(path) > 0)) {
		// Open dir
		if((ret = (pv_open_dir(path))) < 0){
			return ret;
		}
		// List content
		while((ret = (pv_get_dir_entry())) != LP_DIR_END){
			if(ret == LP_DIR_ENTRY_FOUND){
				LP_ladspa_plugin *llpl = new LP_ladspa_plugin(dh->out_path);
				llpl->list();
				delete llpl;
/*				for(y = 0; (llpl->get_plugin_name(y))!= 0; y++){
					ui->add_item(llpl->get_plugin_name(y));
				}
*/			}
			if(ret < 0){
				std::cerr << "LP_ladspa_manager::list_plugins: call to pv_get_dir_entry failed\n";
				return ret;
			}
		}
		if((ret = (pv_close_dir())) < 0){
			return ret;
		}		
	}else{
		// List plugins aviable in LADSPA_PATH
		for(i=0; (path = (pv_get_ladspa_env(i))) != 0; i++){
			//std::cout << "RECH pour i: " << i << path << std::endl;
			// Open dir
			if((ret = (pv_open_dir(path))) < 0){
				return ret;
			}
			// List content
			while((ret = (pv_get_dir_entry())) != LP_DIR_END){
				if(ret == LP_DIR_ENTRY_FOUND){
					std::cout << dh->out_path << std::endl;
					//llpl->describe_plugins_library(dh->out_path);
				LP_ladspa_plugin *llpl = new LP_ladspa_plugin(dh->out_path);
				llpl->list();
				delete llpl;
				}
			}
			if((ret = (pv_close_dir())) < 0){
				return ret;
			}
		}
	}

	delete dh;
	return 0;
}

// Set the audio parameters
int LP_ladspa_manager::set_audio_params(int channels, int sample_rate, int buffer_len)
{
	// store passed vars in class vars
	if(channels < 0){
		std::cerr << "LP_ladspa_manager::set_audio_params: invalid nb of channels\n";
		return -1;
	}
	pv_channels = channels;
	if(sample_rate < 1000){
		std::cerr << "LP_ladspa_manager::set_audio_params: invalid sample_rate\n";
		return -1;
	}
	pv_sample_rate = sample_rate;
	if(buffer_len < 2){
		std::cerr << "LP_ladspa_manager::set_audio_params: invalid buffer len\n";
		return -1;
	}
	pv_buffer_len = buffer_len;
	return 0;
}

int LP_ladspa_manager::run_plugins(float *buffer)
{
    // For each used plugin, if active, call run NOTE: 20 must be replaced !!
    unsigned long ul;
    for(ul=0; ul<20; ul++){
	if(pv_plugins[ul] != 0){
	    if(pv_plugins[ul]->get_active_state() == TRUE){
		//std::cout << "Run.................\n";
		pv_plugins[ul]->run(buffer);
	    }
	}
    }
    return 0;
}

// Implement private functions

char *LP_ladspa_manager::pv_get_ladspa_env(int index)
{
	char *ladspa_env = 0;
	char *pc_buffer	= 0;	
	char *pc_start	= 0;
	char *pc_end	= 0;
	int m_index = 0;
	DIR *dp = 0;
	//struct dirent *entry;

	ladspa_env = getenv("LADSPA_PATH");
	if((ladspa_env == 0)||(strlen(ladspa_env) < 1)){
		std::cerr << "LP_ladspa_manager::get_ladspa_env: your LADSPA_PATH is not set !\n";
		return 0; // Null
	}

	// parse the var
	pc_start = ladspa_env;
	while(*pc_start != '\0'){
		pc_end = pc_start;
		while(*pc_end != ':' && *pc_end != '\0'){
			pc_end++;
		}
		//pc_buffer = (char*)malloc(1 + pc_end - pc_start);
		pc_buffer = new char[1 + pc_end - pc_start];
		if(pc_end > pc_start){
			strncpy(pc_buffer, pc_start, pc_end - pc_start);
		}
		// suppress final '/' if we have one
		if(pc_buffer[(pc_end - pc_start) - 1] == '/'){
			pc_buffer[(pc_end - pc_start) - 1] = '\0';
		}else{
			pc_buffer[pc_end - pc_start] = '\0';
		}

		// If m_index == index, this is the path we must return
		if(m_index == index){
			// Test validity of path
			if((dp = opendir(pc_buffer)) == 0){
				std::cerr << "LP_ladspa_manager::get_ladspa_env: " << pc_buffer << " is not valid path\n";
				std::cerr << "\tError was: " << strerror(errno) << std::endl;
				std::cerr << "\tCheck your LADSPA_PATH\n";
				return 0; // Null
			}
			errno = 0;
			// Test ok, close
			if(closedir(dp) != 0){
				std::cerr << "LP_ladspa_manager::get_ladspa_env: problems by closing dir " << pc_buffer << std::endl;
				std::cerr << "\tError was: " <<strerror(errno) << std::endl;
			}
			// seems ok, return pc_buffer
			return pc_buffer;
		}

		//free(pc_buffer);
		delete[] pc_buffer;
		pc_start = pc_end;
		if(*pc_start == ':'){
			pc_start++;
		}
		m_index++;
	}
	free(dp);
	return 0;
}

const int LP_ladspa_manager::pv_open_dir(char *path)
{
	// If path is NULL, we list current dir
	if(path == 0){
		path = strdup(".");
		if(path == 0){
			std::cerr << "LP_ladspa_manager::pv_open_dir: can't define path !\n";
			return -1;
		}
	}

	//TODO: segment fault occurs when scan in /lib abd /usr/lib
	// I just return if these path's are given
	if((strcmp(path, "/lib")==0) || (strcmp(path, "/lib/")==0)){
		std::cerr << "LP_ladspa_manager::pv_open_dir: can't look in " << path << std::endl;
		std::cerr << "\tIt's a bug: when look here, segfault can occur, sorry\n";
		return -1;
	}
	if((strcmp(path, "/usr/lib")==0) || (strcmp(path, "/usr/lib/")==0)){
		std::cerr << "LP_ladspa_manager::pv_open_dir: can't look in " << path << std::endl;
		std::cerr << "\tIt's a bug: when look here, segfault can occur, sorry\n";
		return -1;
	}

	// Tests if we need a '/' at end of path
	if(path[strlen(path)-1] != '/'){
		strcat(path, "/");
	}

	dh->in_path = path;
	dh->out_path = new char[NAME_MAX + 1];

	if((dh->dp = opendir(dh->in_path)) == 0){
		std::cerr << "LP_ladspa::pv_get_dir: filed to open dir " << dh->in_path << std::endl;
		std::cerr << "\t Error was: " << strerror(errno) << std::endl;
		return -1;
	}

	return 0;
}


const int LP_ladspa_manager::pv_get_dir_entry()
{
	errno = 0;
	if((dh->entry = readdir(dh->dp)) == 0){
		return LP_DIR_END;
	}
	if(errno != 0){
		std::cerr << "LP_ladspa_manager::pv_get_dir_entry: can't list some entry\n";
		std::cerr << "\tError was: " << strerror(errno) << std::endl;
		return -1;
	}

	// Need a temp char buffer
	char *tmp = new char[NAME_MAX + 1];

	// Check if enty is a normal file_path
	if(dh->entry->d_type == DT_REG){
		// Test if dh->out_path is not to short to add entry->d_name
		if((strlen(dh->in_path) + strlen(dh->entry->d_name)) > NAME_MAX){
			std::cerr << "LP_ladspa_manager::pv_get_dir_entry: path is to long !" << std::endl;
			return -1;
		}
		// Copy in_path --> tmp, then add d_name.
		tmp = strcpy(tmp, dh->in_path);
		if(tmp == 0){
			std::cerr << "LP_ladspa_manager::pv_get_dir_entry: error when copy dh->in_path to tmp\n";
			return -1;
		}
		tmp = strcat(tmp, dh->entry->d_name);
		if(tmp == 0){
			std::cerr << "LP_ladspa_manager::pv_get_dir_entry: error when add dh->entry->d_name to tmp\n";
			return -1;
		}
		dh->out_path = strcpy(dh->out_path, tmp);
		if(dh->out_path == 0){
			std::cerr << "LP_ladspa_manager::pv_get_dir_entry: error when copy tmp to dh->out_path\n";
			return -1;
		}
		return LP_DIR_ENTRY_FOUND;
	} else {
		return LP_DIR_NO_ENTRY;
	}

	delete[] tmp;
	return 0;
}

const int LP_ladspa_manager::pv_list_dir(char *path)
{
	// If path is NULL, we list current dir
	if(path == 0){
		path = strdup(".");
		if(path == 0){
			std::cerr << "LP_ladspa_manager::list_dir: can't define path !\n";
			return -1;
		}
	}

	DIR *dp;
	struct dirent *entry;

	if((dp = opendir(path)) == 0){
		std::cerr << "LP_ladspa::list_dir: filed to open dir " << path << std::endl;
		std::cerr << "\t Error was: " << strerror(errno) << std::endl;
		return -1;
	}

	errno = 0;
	while((entry = readdir(dp)) != 0){
		std::cout << entry->d_name;
		// Check if enty is a normal file_path
		if(entry->d_type == DT_REG){
			std::cout << "\tFichier normal\n";
		}else{
			std::cout << std::endl;
		}
	}

	if(errno != 0){
		std::cerr << "LP_ladspa_manager::list_dir: can't list some entry\n";
		std::cerr << "\tError was: " << strerror(errno) << std::endl;
	}

	// close dir
	if(closedir(dp) != 0){
		std::cerr << "LP_ladspa_manager::list_dir: problems by closing dir " << path << std::endl;
		std::cerr << "\tError was: " <<strerror(errno) << std::endl;
	}

	free(dp);
	free(entry);
	return 0;
}
 
const int LP_ladspa_manager::pv_close_dir()
{
	// close dir if one is open
	if(dh->dp != 0){
		if(closedir(dh->dp) != 0){
			std::cerr << "LP_ladspa_manager::list_dir: problems by closing dir " << dh->in_path <<  std::endl;
			std::cerr << "\tError was: " <<strerror(errno) << std::endl;
		}
		dh->dp = 0;
	}

	delete[] dh->out_path;
	return 0;
}

/// The plugins manager's UI
/*
 *  Constructs a ladspa_manager_dlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ladspa_manager_dlg::ladspa_manager_dlg( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ladspa_manager_dlg" );
    ladspa_manager_dlgLayout = new QGridLayout( this, 1, 1, 11, 6, "ladspa_manager_dlgLayout"); 

    pb_list_plugin_2 = new QPushButton( this, "pb_list_plugin_2" );

    ladspa_manager_dlgLayout->addMultiCellWidget( pb_list_plugin_2, 2, 2, 0, 1 );

    lw_plugin_list = new QListView( this, "lw_plugin_list" );
    lw_plugin_list->addColumn( tr( "Column 1" ) );

    ladspa_manager_dlgLayout->addMultiCellWidget( lw_plugin_list, 0, 1, 0, 0 );

    pb_rem_used_plugin = new QPushButton( this, "pb_rem_used_plugin" );

    ladspa_manager_dlgLayout->addWidget( pb_rem_used_plugin, 1, 1 );

    lw_used_plugin = new QListView( this, "lw_used_plugin" );
    lw_used_plugin->addColumn( tr( "Column 1" ) );

    ladspa_manager_dlgLayout->addWidget( lw_used_plugin, 0, 1 );
    languageChange();
    resize( QSize(592, 657).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( pb_list_plugin_2, SIGNAL( clicked() ), this, SLOT( list_plugin_col() ) );
    connect( lw_used_plugin, SIGNAL( doubleClicked(QListViewItem*) ), this, SLOT( get_lw_plugin() ) );
    connect( lw_plugin_list, SIGNAL( doubleClicked(QListViewItem*) ), this, SLOT( select_plug_from_list() ) );
    connect( pb_rem_used_plugin, SIGNAL( clicked() ), this, SLOT( rem_lw_used_plug_item() ) );

	// Obtenir une instance du plugin manager
	llm = new LP_ladspa_manager;
	if(llm == 0){
		std::cerr << "ladspa_manager_dlg::ladspa_manager_dlg: WARNING! private instance of plugin_manager can't be set !\n";
	}
    
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
	used_plugin_dlg[i] = 0;
    }

}

/*
 *  Destroys the object and frees any allocated resources
 */
ladspa_manager_dlg::~ladspa_manager_dlg()
{
	delete llm;
//    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ladspa_manager_dlg::languageChange()
{
    setCaption( tr( "Form1" ) );
    pb_list_plugin_2->setText( tr( "Lister les plugins" ) );
    lw_plugin_list->header()->setLabel( 0, tr( "Column 1" ) );
    lw_plugin_list->clear();
    QListViewItem * item = new QListViewItem( lw_plugin_list, 0 );
    item->setText( 0, tr( "New Item" ) );

    pb_rem_used_plugin->setText( tr( "Enlever plugin" ) );
    lw_used_plugin->header()->setLabel( 0, tr( "Column 1" ) );
    lw_used_plugin->clear();
    item = new QListViewItem( lw_used_plugin, 0 );
    item->setText( 0, tr( "New Item" ) );

}

// Get the ui instance
LP_ladspa_manager *ladspa_manager_dlg::get_manager_instance()
{
	if(llm == 0){
		std::cerr << "ladspa_manager_dlg::get_manager_intsnace: the manager instance is Null\n";
		return 0;
	}
	return llm;
}

// Methodes de l'UI

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
	    if(used_plugin_dlg[plug_idx] != 0) {
		// plugin allready used
		return 0;
	    }

	// Add a plugin to the manager and get the plugin's instance
	pv_plugin[plug_idx] = llm->add_plugin(plug_idx, nb ,plug_dir);
	// Add a new plugin UI
	used_plugin_dlg[plug_idx] = new LP_ladspa_plugin_dlg(0, "TEST", 0, plug_dir);
	used_plugin_dlg[plug_idx]->init_plugin( pv_plugin[plug_idx]);
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
    // Store selected item from lw_plugin_list to lw_used_plugin_dlg
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
    if(used_plugin_dlg[plug_idx] != 0){
	delete used_plugin_dlg[plug_idx];
	used_plugin_dlg[plug_idx] = 0;
	llm->rem_plugin(plug_idx);
	//delete pv_plugin[plug_idx];
	pv_plugin[plug_idx] = 0;
	if(used_plugins > 0){
	    used_plugins--;
	}
    }
    
    delete lw_used_plugin->selectedItem();
    return 0;
}
