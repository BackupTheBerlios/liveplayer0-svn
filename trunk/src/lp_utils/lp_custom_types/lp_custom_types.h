/// This class contains types that are not native supported in QVariant

#ifndef MON_TYPE_H
#define MON_TYPE_H

#include <ladspa.h>

class lp_custom_types
{
public:
	lp_custom_types();
	lp_custom_types(int val);
	lp_custom_types(const LADSPA_Descriptor *ladspa_descriptor);
	~lp_custom_types();
	int get_val();
	const LADSPA_Descriptor *get_ladspa_descriptor();

private:
	int pv_val;
	const LADSPA_Descriptor *pv_ladspa_descriptor;
};

#include <QVariant>
Q_DECLARE_METATYPE(lp_custom_types);

#endif
