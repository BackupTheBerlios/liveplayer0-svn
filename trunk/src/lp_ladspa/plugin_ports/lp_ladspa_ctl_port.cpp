#include "lp_ladspa_ctl_port.h"

#include <iostream>

lp_ladspa_ctl_port::lp_ladspa_ctl_port(QObject *parent)
	: QThread(parent)
{
	pv_descriptor = 0;
	pv_instances = 0;
	float_slider = 0;
	float_knob = 0;
	int_slider = 0;
	toggel = 0;
	meter = 0;
	pv_run_val = 0;
	pv_is_output = false;
}

lp_ladspa_ctl_port::~lp_ladspa_ctl_port()
{
	if(float_slider != 0){
		delete float_slider;
		float_slider = 0;
	}
	if(float_knob != 0){
		delete float_knob;
		float_knob = 0;
	}
	if(int_slider != 0){
		delete int_slider;
		int_slider = 0;
	}
	if(toggel != 0){
		delete toggel;
		toggel = 0;
	}
	if(meter != 0){
		delete meter;
		meter = 0;
	}
	if(pv_run_val != 0){
		delete pv_run_val;
		pv_run_val = 0;
	}
}

int lp_ladspa_ctl_port::init_port(const LADSPA_Descriptor *descriptor, unsigned long port, int samplerate
		, QGridLayout *layout, int _row, int _col)
{
	if(descriptor == 0){
		std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": descriptor is Null\n";
		return -1;
	}
	pv_descriptor = descriptor;
	if(layout == 0){
		std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": layout is Null\n";
		return -1;
	}
	if(samplerate == 0){
		std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": samplerate is not set\n";
		return -1;
	}

	LADSPA_PortRangeHintDescriptor hints;
	hints = descriptor->PortRangeHints[port].HintDescriptor;
	// Setup the ctl_port
	if(descriptor->PortDescriptors[port] == 0){
	//	std::cout << "TEST------\n";
	}

	int row = 0;
	// Search for custom row, col and widget type
	row = custom_layout.get_row(pv_descriptor->UniqueID, port, _row);
	int col = 0;
	col = custom_layout.get_col(pv_descriptor->UniqueID, port, _col);
	int type = 0;
	type = custom_layout.get_ctl_type(pv_descriptor->UniqueID, port);

	std::cout << "ADDING ctl port: col " << col << " row " << row << "\n";

	if((LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[port])) && (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[port]))){
		if(LADSPA_IS_HINT_INTEGER(hints)){
			int_slider = new lp_ladspa_slider_int;
			connect(int_slider, SIGNAL(val_changed(float)), this, SLOT(set_value(float)));
			layout->addWidget(int_slider, row, col);
			int_slider->init(descriptor, port, samplerate);
			std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": int slider ctl for port " << port << "\n";
		}else if(LADSPA_IS_HINT_TOGGLED(hints)){
			toggel = new lp_ladspa_toggel;
			connect(toggel, SIGNAL(val_changed(float)), this, SLOT(set_value(float)));
			layout->addWidget(toggel, row, col);
			toggel->init(descriptor, port, samplerate);
			std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": toggel ctl for port " << port << "\n";
		}else{
			if((type == LP_LADSPA_TYPE_SLIDER) || (type == 0)){
				float_slider = new lp_ladspa_slider_float;
				connect(float_slider, SIGNAL(val_changed(float)), this, SLOT(set_value(float)));
				layout->addWidget(float_slider, row, col);
				float_slider->init(descriptor, port, samplerate);
			}else if(type == LP_LADSPA_TYPE_KNOB){
				float_knob = new lp_ladspa_knob_float;
				connect(float_knob, SIGNAL(val_changed(float)), this, SLOT(set_value(float)));
				layout->addWidget(float_knob, row, col);
				float_knob->init(descriptor, port, samplerate);
			}
			std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": float slider ctl for port " << port << "\n";
		}
		// For input, get a new float - see connect_port() below
		pv_run_val = new float;
	}
	if((LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[port])) && (LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[port]))){
		meter = new lp_ladspa_meter;
		layout->addWidget(meter, row, col);
		meter->init(descriptor, port, samplerate);
		//meter->reciev_val(-12);
		pv_is_output = true;
		std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": meter ctl for port " << port << "\n";
	}

	pv_port = port;

	return 0;
}

int lp_ladspa_ctl_port::instantiate(lp_ladspa_handle *instances, int handles_count)
{
	int i;
	if(instances == 0){
		std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": instances is Null\n";
		return -1;
	}
	pv_instances = instances;
	pv_handles_count = handles_count;
}

void lp_ladspa_ctl_port::set_value(float val)
{
	mutex.lock();
	pv_val = (LADSPA_Data)val;
	std::cout << "VALUE: " << pv_val << std::endl;
	mutex.unlock();
}

void lp_ladspa_ctl_port::connect_port()
{
	mutex.lock();
	int i;
	if((pv_instances != 0)&&(pv_descriptor != 0)){
		// If we have an output, pass pv_val as address
		if(pv_is_output == true){
			for(i=0; i<pv_handles_count; i++){
				pv_descriptor->connect_port(pv_instances[i].get_instance(), pv_port, &pv_val);
				std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": CTL(I/O) - connect port " << pv_port << "\n";
			}
		}else{
			// Input: have concurence with the slot "set_value" - copy the value
			*pv_run_val = pv_val;
			for(i=0; i<pv_handles_count; i++){
				pv_descriptor->connect_port(pv_instances[i].get_instance(), pv_port, pv_run_val);
				std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": CTL(I/O) - connect port " << pv_port << "\n";
			}
		}
		if(pv_is_output == true){
			meter->reciev_val(pv_val);
		}
	}
	mutex.unlock();
}

void lp_ladspa_ctl_port::run()
{
}
