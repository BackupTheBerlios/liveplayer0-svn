#include "lp_custom_types.h"

lp_custom_types::lp_custom_types()
{
	pv_val = 0;
	pv_ladspa_descriptor = 0;
}

lp_custom_types::lp_custom_types(int val)
{
	pv_val = val;
}

lp_custom_types::lp_custom_types(const LADSPA_Descriptor *ladspa_descriptor)
{
	pv_ladspa_descriptor = ladspa_descriptor;
}

lp_custom_types::~lp_custom_types()
{
}

int lp_custom_types::get_val()
{
	return pv_val;
}

const LADSPA_Descriptor *lp_custom_types::get_ladspa_descriptor()
{
	return pv_ladspa_descriptor;
}
