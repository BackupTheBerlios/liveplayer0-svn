
#include "LP_ladspa_cpp.h"
//#include <qwidget.h>

// Contructor
lp_ladspa_cpp::lp_ladspa_cpp()
{
	std::cout << "Contructeur\n";
	manager_dlg = new ladspa_manager_dlg(0, "TEST", FALSE, 0);
	manager = manager_dlg->get_manager_instance();
	if(manager == 0){
		std::cerr << "lp_ladspa_cpp::lp_ladspa_cpp: cannot obtain ladspa_manager instance !\n";
	}
	manager->set_audio_params(2, 44100, 20);
	manager_dlg->show();
	QThread::start();
}

// Destructor
lp_ladspa_cpp::~lp_ladspa_cpp()
{
	
	delete manager_dlg;
}

// The run function
void lp_ladspa_cpp::run()
{
	std::cout << "Thread lancé... -- attente de traitement\n";

	// buffer de test
	int i;
	float *buffer = new float[20];

	while(1){
		usleep(100000);
		for(i=0; i<20; i++){
			buffer[i] = i*10;
		}

		manager->run_plugins(buffer);

		for(i=0; i<20; i++){
			std::cout << "Sortie: " << buffer[i] << std::endl;
		}
	}
}

