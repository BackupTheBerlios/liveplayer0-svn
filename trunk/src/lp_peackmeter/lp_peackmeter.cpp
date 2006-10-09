#include "lp_peackmeter.h"

lp_peackmeter::lp_peackmeter(QWidget *parent,  int _nb_channels, int _buf_size)
	: QWidget(parent)
{
	int i;

	// Set channels
	if(_nb_channels > lp_peackmeter_core::MAX_CHANNELS){
		std::cerr << "lp_peackmeter::__FUNC__: _nb_channels to hight, set to " << lp_peackmeter_core::MAX_CHANNELS << std::endl;
		nb_channels = lp_peackmeter_core::MAX_CHANNELS;
	}else{
		nb_channels = _nb_channels;
	}

	// Create layout and place every meter widgets on it
	b_layout = new QHBoxLayout(parent);
	pm_core = new lp_peackmeter_core(parent, nb_channels, _buf_size);
	for(i=0; i<nb_channels; i++){
		pm_widget[i] = new lp_peackmeter_widget(this);
		b_layout->addWidget(pm_widget[i]);
	}
	setLayout(b_layout);
	show();

	// Connect the core signal to the slot
	connect (pm_core, SIGNAL(get_val(float*)), 
		this, SLOT(set_val(float*)) );

	set_range(-50, 0);

	// Down factors
	pv_down_ref_time = 1500000;	// meter time factor
	pv_down_factor = -55;		// meter down factor (-55dB per 1.5s)
	for(i=0; i<nb_channels; i++){
		pv_last_peack[i] = 0;
		pv_down_val[i] = 0;
		down_timer[i].start();
	}
	pv_hold_ref_time = 300000;	// text label time factor

	// default refresh frequency
	set_refresh_freq(25);
}

lp_peackmeter::~lp_peackmeter()
{

}

lp_peackmeter_core* lp_peackmeter::get_core()
{
	return pm_core;
}

void lp_peackmeter::set_refresh_freq(int freq)
{
	unsigned int u_secs;
	u_secs = 1000000 / freq;
	pm_core->set_refresh_time(u_secs);
}

void lp_peackmeter::set_range(int dB_min, int dB_max)
{
	// Set min and max val in float
	float f_min, f_max;
	int i;

	f_min = (float)pow(10, ((double)dB_min / (double)20));
	f_max = (float)pow(10, ((double)dB_max / (double)20));
	// set the core'range
	pm_core->set_range(f_min, f_max);
	// set widget'range
	for(i=0; i<nb_channels; i++){
		pm_widget[i]->set_range(dB_min, dB_max);
	}

}

void lp_peackmeter::set_val(float *val)
{
	int i;
	for(i=0; i<nb_channels; i++){
		//pm_widget[i]->set_val(val_to_db(down_val(val[i], i)), pm_core->get_clip(i), val_to_db(pv_peack_hold(val[i], i)));
		pm_widget[i]->set_val(down_val(val_to_db(val[i]), i), pm_core->get_clip( i), val_to_db(pv_peack_hold(val[i], i)));
		//std::cout << "Set val " << val_to_db(val[i]) << "- clip: " << pm_core->get_clip(i) << std::endl;
	}
}

int lp_peackmeter::down_val(int cur_val, int channel)
{
	if(cur_val >= pv_down_val[channel]){
		pv_down_val[channel] = cur_val;
		pv_last_peack[channel] = cur_val;
		down_timer[channel].start();
	}else{
		pv_down_val[channel] = pv_last_peack[channel] + ((pv_down_factor * (int)down_timer[channel].us_instant_time()) / pv_down_ref_time);
	}

	//std::cout << "cur_val: " << cur_val << " - last_peack: " << pv_last_peack[channel] << " - down_val: " << pv_down_val[channel] << std::endl;
	return pv_down_val[channel];
}

float lp_peackmeter::pv_peack_hold(float act_val, int channel)
{
	struct timeval actual_time;
	gettimeofday(&actual_time, 0);

	unsigned int actual_usec;	// time in usec
	actual_usec = actual_time.tv_usec + (actual_time.tv_sec * 1000000);

	unsigned int last_usec;		// last time in usec
	last_usec = pv_last_hold_time[channel].tv_usec + (pv_last_hold_time[channel].tv_sec * 1000000);

	unsigned int delta_t;
	delta_t = actual_usec - last_usec;

	float tmp_val;

	if(delta_t >= pv_hold_ref_time){
		tmp_val = act_val;
		gettimeofday(&pv_last_hold_time[channel], 0);
		pv_last_hold_peack[channel] = act_val;
	//	std::cout << "HOLD (Reset): return " << tmp_val << std::endl;
	}else{
		tmp_val = pv_last_hold_peack[channel];
	//	std::cout << "HOLD: return " << tmp_val << std::endl;
	}

	return tmp_val;
}


int lp_peackmeter::val_to_db(float f_val)
{
	return (int)(20*((float)log10((double)f_val)));
}
