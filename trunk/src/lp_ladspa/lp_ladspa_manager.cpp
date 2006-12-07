#include "lp_ladspa_manager.h"

#include <iostream>

lp_ladspa_manager::lp_ladspa_manager(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	pv_manager_items_count = 1;
	pv_manager_items = 0;
	pv_plugins = 0;
	pv_layout = 0;
	pv_channels = 0;
	pv_buf_size = 0;
	pv_samplerate = 0;
}

lp_ladspa_manager::~lp_ladspa_manager()
{
	if(pv_manager_items != 0){
		delete[] pv_manager_items;
	}
}

int lp_ladspa_manager::init(int manager_items_count, int channels, int buf_size, int samplerate)
{
	int i;
	pv_channels = channels;
	pv_buf_size = buf_size;
	pv_samplerate = samplerate;

	if(manager_items_count < 1){
		std::cerr << "lp_ladspa_manager::" << __FUNCTION__ << ": manager_items_count is < 1\n";
		return -1;
	}
	pv_manager_items_count = manager_items_count;
	pv_layout = new QGridLayout(this);

	// Allocate the items array and init each
	pv_manager_items = new lp_ladspa_manager_item[pv_manager_items_count];
	for(i=0; i<pv_manager_items_count; i++){
		pv_manager_items[i].init(pv_channels, pv_buf_size, pv_samplerate);
		pv_layout->addWidget(&pv_manager_items[i], 0, i);
	}

	setLayout(pv_layout);

	return 0;
}

int lp_ladspa_manager::run_interlaced_buffer(float *buffer, int tot_len)
{
	// Run each plugin
	int i, retval, last_ret;
	for(i=0; i<pv_manager_items_count; i++){
		retval = pv_manager_items[i].run_interlaced_buffer(buffer, tot_len);
		if(retval >= last_ret){
			last_ret = retval;
		}else{
			retval = last_ret;
		}
	//	std::cout << "Called item[" << i << "] - protected samples: " << retval << "\n";
	}
	return retval;
}
