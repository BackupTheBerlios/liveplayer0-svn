#ifndef LP_LADSPA_MANAGER_H
#define LP_LADSPA_MANAGER_H

// CPP libs
#include <iostream>
#include <string>
// QT libs
#include <qapplication.h>
#include <qvariant.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

// C libs
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include "ladspa.h"

#include "LP_ladspa_plugin.h"
//#include "LP_ladspa_manager_ui.h"

const int LP_DIR_END	 	= 0;
const int LP_DIR_ENTRY_FOUND 	= 1;
const int LP_DIR_NO_ENTRY	= 2;

class ladspa_manager_dlg;

class LP_ladspa_manager{

	public:
		LP_ladspa_manager();
		~LP_ladspa_manager();

		/// This function give the plugins name and UniqueID found in path, it's desined
		/// to be used une a bounce. It return the number of plugin found in
		/// path, use this result in a for() bounce to have all names.
		/// At end of directory, or if error occur, it returns -1
		long get_plugin_name(char *path, char *out_name[LP_MAX_PLUGIN], unsigned long *UniqueID, char *plugin_path);

		int set_audio_params( int channels, int sample_rate, int buffer_len );

		// list plugins found in LADSPA_PATH and given path
		int list_plugins(char *path);

		// Add a plugin to array and return instance
		LP_ladspa_plugin *add_plugin(unsigned long index, unsigned long ID, char *path);

		// Remove a plugin
		int rem_plugin(unsigned long index);

		// Return the plugin's instance of inexd
		LP_ladspa_plugin *get_plugin_instance(unsigned long index);

		int run_plugins( float * buffer );

	private:
		// parse the LADSPA_PATH and return the requierd path(first, second etc...) - Null if err
		char *pv_get_ladspa_env(int index);

		// struct for directory handler
		typedef struct {
			char *in_path;
			char *out_path;
			int index;
			DIR *dp;
			struct dirent *entry;
		}dir_handle;
		dir_handle *dh;
		// Open a dir and allocate a dir_handle
		const int pv_open_dir(char *path);
		// Close the dir given in dh and free memory
		const int pv_close_dir();
		// returns the paths in dir stored in dir_handle (result is stored in dir_handle->out_path
		const int pv_get_dir_entry();

		// List directory entries
		const int pv_list_dir(char *path);

		// Tab of plugin's instances
		LP_ladspa_plugin *pv_plugins[LP_MAX_PLUGIN];

		// Audio params
		int pv_channels;
		int pv_sample_rate;
		int pv_buffer_len;

};

/// The plugins manager's UI
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QListView;
class QListViewItem;
class QThread;

class ladspa_manager_dlg : public QDialog
{
    Q_OBJECT

public:
	ladspa_manager_dlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~ladspa_manager_dlg();
	
	QPushButton* pb_list_plugin_2;
	QListView* lw_plugin_list;
	QPushButton* pb_rem_used_plugin;
	QListView* lw_used_plugin;
	
	// return the plugins manager instance
	LP_ladspa_manager *get_manager_instance();
	
//	virtual int run_plugins( float * buffer );
	
public slots:
//	virtual void destroy();
	virtual int list_plugin_col();
	virtual int get_lw_plugin();
	virtual int select_plug_from_list();
	virtual int rem_lw_used_plug_item();

protected:
	LP_ladspa_manager *llm;
	int used_plugins;
	
	QGridLayout* ladspa_manager_dlgLayout;
	
protected slots:
	    virtual void languageChange();

private:
	LP_ladspa_plugin_dlg *used_plugin_dlg[20];
    	LP_ladspa_plugin *pv_plugin[20];

};

#endif 
