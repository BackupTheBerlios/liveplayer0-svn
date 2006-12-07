#include "lp_ladspa_layout.h"

lp_ladspa_layout::lp_ladspa_layout()
{

}

lp_ladspa_layout::~lp_ladspa_layout()
{

}

int lp_ladspa_layout::get_row(unsigned long plugin_UID, unsigned long port, int initial_row)
{
	// TAP EQ
	if(plugin_UID == 2141){
		if(port < 8){
			return initial_row;
		}else if(port > 7){
			return initial_row + 1;
		}
	// TAP EQ BW
	}else if(plugin_UID == 2151){
		if(port < 8){
			return initial_row;
		}else if((port > 7)&&(port < 16)){
			return initial_row + 1;
		}else if(port > 15){
			return initial_row + 2;
		}
	}
	// Arraving here, nothing was found
	return initial_row;
}

int lp_ladspa_layout::get_col(unsigned long plugin_UID, unsigned long port, int initial_col)
{
	// TAP EQ
	if(plugin_UID == 2141){
		if(port < 8){
			return initial_col;
		}else if(port > 7){
			return initial_col - 8;
		}
	// TAP EQ BW
	}else if(plugin_UID == 2151){
		if(port < 8){
			return initial_col;
		}else if((port > 7)&&(port < 16)){
			return initial_col - 8;
		}else if(port > 15){
			return initial_col - 16;
		}
	}
	// Arraving here, nothing was found
	return initial_col;

}

int lp_ladspa_layout::get_ctl_type(unsigned long plugin_UID, unsigned long port)
{
	// TAP EQ
	if(plugin_UID == 2141){
		if(port < 8){
			return LP_LADSPA_TYPE_SLIDER;
		}else if(port > 7){
			return LP_LADSPA_TYPE_KNOB;
		}
	// TAP EQ BW
	}else if(plugin_UID == 2151){
		if(port < 8){
			return LP_LADSPA_TYPE_SLIDER;
		}else if((port > 7)&&(port < 16)){
			return LP_LADSPA_TYPE_KNOB;
		}else if(port > 15){
			return LP_LADSPA_TYPE_KNOB;
		}
	}
	// Use default
	return 0;
}
