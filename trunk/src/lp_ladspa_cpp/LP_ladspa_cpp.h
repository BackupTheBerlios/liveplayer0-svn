#include <iostream>
#include <qapplication.h>
#include <qthread.h>
#include <qwidget.h>

#include "LP_ladspa_manager.h"
//#include "LP_ladspa_manager_ui.h"

class lp_ladspa_cpp : QThread
{

	public:
	// Contructor and destructor
	lp_ladspa_cpp();
	~lp_ladspa_cpp();
	// The (needed) run() function
	virtual void run();
	ladspa_manager_dlg *manager_dlg;
	LP_ladspa_manager *manager;
};
