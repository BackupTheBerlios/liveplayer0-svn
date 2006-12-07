#include "lp_ladspa_handle.h"

#include <iostream>

lp_ladspa_handle::lp_ladspa_handle()
{
	pv_descriptor = 0;
	pv_instance = 0;
}

lp_ladspa_handle::~lp_ladspa_handle()
{
}

int lp_ladspa_handle::instantiate(const LADSPA_Descriptor *descriptor, unsigned long samplerate)
{
	if(descriptor == 0){
		std::cerr << "lp_ladspa_handle::" << __FUNCTION__ << ": descriptor is Null\n";
		return -1;
	}
	pv_descriptor = descriptor;
	pv_instance = descriptor->instantiate(descriptor, samplerate);
	if(pv_instance == 0){
		std::cerr << "lp_ladspa_handle::" << __FUNCTION__ << ": instantiation failed\n";
		return -1;
	}
	return 0;
}

LADSPA_Handle lp_ladspa_handle:: get_instance()
{
	return pv_instance;
}

void lp_ladspa_handle::run(int len)
{
	if(pv_descriptor == 0){
		std::cerr << "lp_ladspa_handle::" << __FUNCTION__ << ": pv_descriptor is Null\n";
		return;
	}
	std::cout << "lp_ladspa_handle::" << __FUNCTION__ << ": running - instance " << pv_instance << std::endl;
	pv_descriptor->run(pv_instance, len);
}
