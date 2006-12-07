#ifndef LP_LADSPA_HANDLE_H
#define LP_LADSPA_HANDLE_H

#include <ladspa.h>

class lp_ladspa_handle
{
public:
	lp_ladspa_handle();
	~lp_ladspa_handle();
	int instantiate(const LADSPA_Descriptor *descriptor, unsigned long samplerate);
	LADSPA_Handle get_instance();
	void run(int len);
private:
	const LADSPA_Descriptor *pv_descriptor;
	LADSPA_Handle pv_instance;
};

#endif
