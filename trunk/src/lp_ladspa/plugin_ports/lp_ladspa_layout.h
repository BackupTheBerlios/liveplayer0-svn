#ifndef LP_LADSPA_LAYOUT_H
#define LP_LADSPA_LAYOUT_H

// Defines for the widget type
#define LP_LADSPA_TYPE_SLIDER	1
#define LP_LADSPA_TYPE_KNOB	2

class lp_ladspa_layout
{
public:
	lp_ladspa_layout();
	~lp_ladspa_layout();
	// returns the custom row if exists (edit lp_ladspa_layout.cpp to add)
	// if not found, returns -1
	int get_row(unsigned long plugin_UID, unsigned long port, int initial_row);
	int get_col(unsigned long plugin_UID, unsigned long port, int initial_col);
	// Returns val corrsponding to defines above - 0 = use default
	int get_ctl_type(unsigned long plugin_UID, unsigned long port);
};

#endif
