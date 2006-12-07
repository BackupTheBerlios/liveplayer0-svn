#include "lp_ladspa_plugin.h"
#include <iostream>

lp_ladspa_plugin::lp_ladspa_plugin(QWidget *parent)
	:QWidget(parent)
{
	in_process = false;
	end = false;
	pv_descriptor = 0;
	base_widget = 0;
	gain_slider = 0;
	in_peackmeter = 0;
	in_pmc = 0;
	out_peackmeter = 0;
	out_pmc = 0;
	pv_ctl_ports = 0;
	pv_audio_ports = 0;
	pv_instances = 0;
	pv_handles_count = 0;
	pv_active = false;
	pv_has_activate = false;
	pv_has_deactivate = false;

	start(TimeCriticalPriority);
}

lp_ladspa_plugin::~lp_ladspa_plugin()
{
	// Make plugin unactive - be shure all run_interlaced_buffer() request returns without processing
	deactivate();

	// End the thread
	mutex.lock();
	end = true;
	in_process = true;
	new_process.wakeAll();
	mutex.unlock();
	// Wait the end of the thread
	wait();

	// Ok - clear all memebers...
	int i;
	for(i=0; i<pv_handles_count; i++){
		pv_descriptor->cleanup(pv_instances[i].get_instance());
		std::cout << "Cleaning instances[" << i << "]\n";
	}
	pv_descriptor = 0;

	if(gain_slider != 0){
		delete gain_slider;
	}
	if(in_peackmeter != 0){
		delete in_peackmeter;
	}
	if(in_pmc != 0){
		delete in_pmc;
	}
	if(out_peackmeter != 0){
		delete out_peackmeter;
	}
	if(out_pmc != 0){
		delete out_pmc;
	}
	if(pv_ctl_ports != 0){
		delete[] pv_ctl_ports;
	}
	if(pv_audio_ports != 0){
		delete pv_audio_ports;
	}

	if(base_widget != 0){
		delete base_widget;
		base_widget = 0;
	}
}

int lp_ladspa_plugin::init(const LADSPA_Descriptor *descriptor, int channels, int buf_size, int samplerate)
{
	if(descriptor == 0){
		std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": descriptor is Null\n";
		return -1;
	}
	pv_descriptor = descriptor;

	if(pv_descriptor->activate != 0){
		pv_has_activate = true;
	}
	if(pv_descriptor->deactivate != 0){
		pv_has_deactivate = true;
	}

	int col = 2;

	// Layout
	layout = new QGridLayout(this);

	// Add the gain control
	gain_slider = new lp_ladspa_slider_float;
	gain_slider->init(-20.0, 10.0, 0.0, true);
	gain_slider->set_name(QObject::tr("Input gain"));
	layout->addWidget(gain_slider, 1, 0);

	// Add input meter
	in_peackmeter = new lp_peackmeter(this, channels, buf_size);
	in_pmc = in_peackmeter->get_core();
	layout->addWidget(in_peackmeter, 1, 1);


	// Plugin parameters here
	if(LADSPA_IS_REALTIME(pv_descriptor->Properties)){
		std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": Real time dependencies\n";
	}
	if(LADSPA_IS_INPLACE_BROKEN(pv_descriptor->Properties)){
		std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": INPLACE BROCKEN !\n";
	}
	if(LADSPA_IS_HARD_RT_CAPABLE(pv_descriptor->Properties)){
		std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": Hard RT\n";
	}


	txt_routing = new QLabel;
	layout->addWidget(txt_routing, 0, 0);

	unsigned long i, pcount, ctl_count = 0;
	pcount = descriptor->PortCount;

	// Audio ports - the pv_audio_ports instance manages all audio ports
	pv_audio_ports = new lp_ladspa_audio_ports;
	pv_audio_ports->init(descriptor, channels, buf_size, samplerate);
	txt_routing->setText("Routing mode: " + pv_audio_ports->get_merge_mode_name());
	pv_handles_count = pv_audio_ports->needed_handles_count();
	std::cout << "handles count: " << pv_handles_count << "\n";

	// Count ctl ports
	for(i=0; i<pcount; i++){
		if(LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
			ctl_count++;
			//std::cout << "Ctl port: " << ctl_count << std::endl;
		}
	}
	pv_ctl_ports = new lp_ladspa_ctl_port[ctl_count];
	// init each ctl port
	ctl_count = 0;
	for(i=0; i<pcount; i++){
		if(LADSPA_IS_PORT_CONTROL(descriptor->PortDescriptors[i])) {
			if((pv_ctl_ports[ctl_count].init_port(descriptor, i, samplerate, layout, 1, col))<0){
				std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": initialisation failed !\n";
			}
			col++;
			ctl_count++;
		}
	}
	pv_ctl_count = (int)ctl_count;
	// Instanciate plugin
	int y;
	pv_instances = new lp_ladspa_handle[pv_handles_count];
	if(pv_instances == 0){
		std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": plugin instantiation failed\n";
	}
	for(y=0; y<pv_handles_count; y++){
		if(pv_instances[y].instantiate(pv_descriptor, (unsigned long)samplerate) < 0){
			std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": plugin instantiation failed\n";
			return -1;
		}
	}

	// call instantiate
	pv_audio_ports->instantiate(pv_instances);
	for(i=0; i<ctl_count; i++){
		pv_ctl_ports[i].instantiate(pv_instances, pv_handles_count);
	}

	// Add output meter
	out_peackmeter = new lp_peackmeter(this, channels, buf_size);
	out_pmc = out_peackmeter->get_core();
	layout->addWidget(out_peackmeter, 1, pv_ctl_count + 2);

	setLayout(layout);
	setWindowTitle(pv_descriptor->Name);

	return 0;
}

int lp_ladspa_plugin::run_interlaced_buffer(float *buffer, int tot_len)
{
	mutex.lock();
	if(pv_active == false){
		mutex.unlock();
		return 0;
	}
	if(in_process == true){
		end_process.wait(&mutex);
	}
	mutex.unlock();

	// Pass the address of pv_buffer,
	// wake process thread on. When
	// the processing ends, the new values
	// are stored in the buffer
	pv_buffer = buffer;
	pv_len = tot_len;

	pv_gain = gain_slider->get_val();

	mutex.lock();
	in_process = true;
	new_process.wakeAll();
	mutex.unlock();

	mutex.lock();
	if(in_process == true){
		end_process.wait(&mutex);
	}
	mutex.unlock();

	return pv_retval;
}

void lp_ladspa_plugin::activate()
{
	int i;
	if(pv_audio_ports->supported_routing() != true){
		pv_active = false;
	}else{
		if((pv_has_activate == true)&&(pv_active != true)){
			for(i=0; i<pv_handles_count; i++){
				pv_descriptor->activate(pv_instances[i].get_instance());
			}
		}
		pv_active = true;
	}
}

void lp_ladspa_plugin::deactivate()
{
	int i;
	if((pv_has_deactivate == true)&&(pv_active == true)){
		for(i=0; i<pv_handles_count; i++){
			pv_descriptor->deactivate(pv_instances[i].get_instance());
		}
	}
	pv_active = false;
}

bool lp_ladspa_plugin::is_active()
{
	return pv_active;
}

QString lp_ladspa_plugin::get_name()
{
	if(pv_descriptor == 0){
		std::cerr << "lp_ladspa_plugin::" << __FUNCTION__ << ": pv_descriptor is Null\n";
		return "Error";
	}
	return pv_descriptor->Name;
}

// Thread
void lp_ladspa_plugin::run()
{
	std::cout << "Thread started\n";
	int i;
	while(end != true){
		mutex.lock();
		if(in_process != true){
			new_process.wait(&mutex);
		}
		mutex.unlock();

		if(end != true){
			// Make processing here
			// Set gain
			for(i=0; i<pv_len; i++){
				pv_buffer[i] = pv_buffer[i] * pv_gain;
			}
			// run input meter
			in_pmc->run_interlaced_buffer(pv_buffer, pv_len);
			for(i=0; i<pv_ctl_count; i++){
				pv_ctl_ports[i].connect_port();
			}
			pv_retval = pv_audio_ports->run_interlaced_buffer(pv_buffer, pv_len);
			// run output meter
			out_pmc->run_interlaced_buffer(pv_buffer, pv_len);
			//std::cout << "Processing done " << pv_retval << " samples\n";
		}

		mutex.lock();
		in_process = false;
		end_process.wakeAll();
		mutex.unlock();
	}
}
