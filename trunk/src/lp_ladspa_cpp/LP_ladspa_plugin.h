// Classe LADSPA  -- version tests
#ifndef LADSPA_PLUGIN_H
#define LADSPA_PLUGIN_H

// CPP libs
#include <iostream>
#include <string>
#include "LP_ladspa_utils.h"
// C libs
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include "ladspa.h"
// QT libs
#include <qvariant.h>
#include <qdialog.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
// Qwt classes (needs libqwt4c2 and libqwt-dev on Debian)
#include <qwt/qwt_slider.h>
#include <qwt/qwt_knob.h>

// Assume we dont'have more that LP_MAX_PLUGIN in one file
#ifndef LP_MAX_PLUGIN
const unsigned long	LP_MAX_PLUGIN 	  = 100;
#endif

// Assume we dont't have more than LP_MAX_PORT in one plugin
#ifndef LP_MAX_PORT
const unsigned long	LP_MAX_PORT   	  = 150;
#endif

// Ports data structure
class lp_ladspa_ports{
	public:
	lp_ladspa_ports(){ 
		//name = new char[NAME_MAX];
	}
	~lp_ladspa_ports(){
		delete[] name;
	}
	// Port's name
	char *name;
	// Port's hints
	bool is_in_ctl;
	bool is_toggle;
	bool is_int;
	LADSPA_Data low_bound;
	LADSPA_Data high_bound;
	bool is_logarithmic;
	LADSPA_Data default_value;
	// The values of each port are stored here, and passed to the plugin
};

class LP_ladspa_plugin_dlg;
class LP_ladspa_port_dlg;

// Classe ladspa: doit etre un plugin simple, et non une libraie.
// On doit pouvoir charger un plugin, et en obtenir une instance
// avec une instance de cette classe
class LP_ladspa_plugin{

	public:
		LP_ladspa_plugin(char *file_path);
		~LP_ladspa_plugin();

		// initialize plugin and draw the plugin'ui
		int init_plugin(/*QWidget* parent, const char* name, */unsigned long unique_ID,int channels, int sample_rate, int buf_len);
		// Activate the pèlugin's instance
		int activate();
		int run(LADSPA_Data *buffer);
		bool get_active_state();
		// Return a ports structure
		lp_ladspa_ports *get_ports();
		// Return ports count
		unsigned long get_ports_count();
		// Returns plugin name
		char *get_plug_name();
		// Return the plugin instance
		LP_ladspa_plugin *get_instance();

		// Set the plugin port's values
		int set_port_value(unsigned long port, LADSPA_Data value);

		char *get_plugin_name(unsigned long index);
		unsigned long get_plugin_ID(unsigned long index);
		int list();
		void test();

	private:
		// obtain a LADSPA_Descriptor_Function: if not ok, return 0
		int pv_obtain_desriptor_function(char *file_path);

		// Describe a plugin
		int pv_describe_plugin(unsigned long unique_ID);
		int pv_describe_plugin(char *label);

		// List aviable plugins in a lib
		int pv_list_plugins();
		int pv_init_plugin();
		int pv_init_ports();

		/* Variables membres */
		// The plugin path
		char *pv_plugin_path;
		// the library handler (returned by dlopen() )
		void *pv_lib_handler;
		// The LADSPA_Descriptor_Function passed to plugin to obtain a descriptor
		LADSPA_Descriptor_Function pv_LDF;
		// The LADSPA_Descriptor
		const LADSPA_Descriptor *pv_LD;
		// The LADSPA_PortDescriptor
		LADSPA_PortDescriptor pv_port_desc;
		// The LADSPA_Handle
		LADSPA_Handle pv_ladspa_handle;

		// Sample rate
		unsigned int pv_srate;

		/* Plugins parameters */
		char *pv_plug_name;
		char *pv_label;
		char *pv_maker;
		char *pv_copyright;
		int pv_must_rt;		// Must run real time
		int pv_inplace_brocken;
		int pv_hard_rt;		// Is hard real-time capable

		// Set the plugin "active state"
		bool pv_active;
		bool pv_has_activate;

		/* Ports parameters */
		//lp_ladspa_ports pv_ports[LP_MAX_PORT];
		lp_ladspa_ports pv_ports[];

		// Control ports parameters
		int pv_nb_ctl_in;
		int pv_nb_ctl_out;
		// The values of each port are stored here, and passed to the plugin
		LADSPA_Data pv_in_ctl_value[LP_MAX_PORT];
		LADSPA_Data pv_out_ctl_value[LP_MAX_PORT];

		// Hint parameters
		LADSPA_PortRangeHintDescriptor pv_port_hint_dec;

		// Audio ports parameters
		int pv_nb_audio_in;
		int pv_nb_audio_out;

		// Nb channels in the given buffer from caller
		int pv_nb_channels;

		// Port's data buffer
		int pv_buf_len;
		LADSPA_Data *pv_in_buffer[LP_MAX_PORT];
		LADSPA_Data *pv_out_buffer[LP_MAX_PORT];

		// Caller's given buffer
		LADSPA_Data *pv_buffer;
		LADSPA_Data **pv_LR_buffer;

		// Connect ports
		int pv_connect_ports();

		// The plugin's UI instance
		LP_ladspa_plugin_dlg *pv_ui;
};

// Declarations for the plugin's UI

//class QVBoxLayout;
//class QHBoxLayout;
//class QGridLayout;
class QSpacerItem;
//class QFrame;
class QLineEdit;
class QLabel;
class QSlider;
class QDial;
class QHBox;
class QVBox;
class QPushButton;


/// Here is the plugin_ui class declaration
//class LP_ladspa_plugin_dlg
class LP_ladspa_plugin_dlg : public QVBox
{
    Q_OBJECT

public:
    LP_ladspa_plugin_dlg( QWidget* parent = 0, const char* name = 0, WFlags fl = 0,  const char *lib_path = 0 );
    ~LP_ladspa_plugin_dlg();

	// Init the plugin (ste a new plugin instance, init it and draw the UI )
	int init_plugin(LP_ladspa_plugin *plugin /*unsigned long plug_id, int channels, int sample_rate, int buf_len*/);

	// Define plugin name label
	int set_plug_name(char *name);

	// Add a port control in ui
	int add_port_ctl(unsigned long index, char *port_name, bool toggel, bool dial, bool is_int);
	// Define ctl range
	int set_port_max_val(unsigned long index, LADSPA_Data val);

	QLabel* ql_plug_name;
	// Set the port range
	int set_range(unsigned long index, LADSPA_Data min, LADSPA_Data max, bool is_log);
	// Set the port default value
	int set_default_value(unsigned long index, LADSPA_Data val);
	QPushButton *pb_test;

	// Get port's value
	LADSPA_Data get_port_value(unsigned long index);

public slots:
	//void pr_display_ctl_value(unsigned long index, LADSPA_Data val);
	//void test(int val);

protected:
//	QVBoxLayout *layout;

protected slots:
    virtual void languageChange();

private:
	QHBox *pv_hbox;
	LP_ladspa_port_dlg *pv_port_ui[LP_MAX_PORT];
	// Plugin instance
	LP_ladspa_plugin *pv_plugin;
	// Plugin's path
	char *pv_plug_path;
	// ctl ports data
	unsigned long pv_nb_ports;
	lp_ladspa_ports *pv_ports;
};

/// The plugin's ctl port UI
class LP_ladspa_port_dlg : public QHBox
{
	Q_OBJECT

public:
	LP_ladspa_port_dlg( QWidget *parent = 0, const char *name = 0, WFlags fl = 0, LP_ladspa_plugin *plugin = 0, bool toggel = FALSE, bool dial = 0 , bool is_int = 0 , unsigned long port_index = 0);
	~LP_ladspa_port_dlg();

	// Set the port range
	int set_range(LADSPA_Data min, LADSPA_Data max, bool is_log);
	int set_default_value(LADSPA_Data val);
	LADSPA_Data get_value();

private:
	// Widgets
	QLabel* ql_port_name;
	QLineEdit* qle_value;
	QSlider* qsl_int_value;
	QwtSlider *qsl_value;
	//QDial* qdl_value;
	QwtKnob *qdl_value;
	QPushButton *pb_value;

	// True if the port is logarithmic
	bool pv_is_log;
	// True if port ctl is integer
	bool pv_is_int;

	// Plugin instance
	LP_ladspa_plugin *pv_plugin;

	unsigned long pv_port_index;

	// Port's value
	LADSPA_Data pv_value;

private slots:
	void pv_set_value(double val);
	void pv_set_value(int val);
};

#endif
