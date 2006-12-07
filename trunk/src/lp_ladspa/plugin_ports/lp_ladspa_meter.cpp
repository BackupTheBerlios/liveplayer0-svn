#include "lp_ladspa_meter.h"
#include <iostream>

#include <math.h>

lp_ladspa_meter::lp_ladspa_meter(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	// Initial defaults
	pv_default_low_val = 0.001;
	pv_default_high_val = 10.0;
	pv_is_log = false;
	pv_def_val = 1.0;

	log10_scale_engine = new QwtLog10ScaleEngine;
}

lp_ladspa_meter::~lp_ladspa_meter()
{

}

int lp_ladspa_meter::init(const LADSPA_Descriptor *descriptor,
				 unsigned long port,
				 int samplerate)
{
	LADSPA_Data f_val, f_low, f_high;

	if(descriptor == 0){
		std::cerr << "lp_ladspa_meter::" << __FUNCTION__ << ": pdescriptor is Null\n";
		return -1;
	}

	pv_samplerate = samplerate;

	LADSPA_PortRangeHintDescriptor hints;
	hints = descriptor->PortRangeHints[port].HintDescriptor;

	LADSPA_PortRangeHint range_hints;
	range_hints = descriptor->PortRangeHints[port];

	if(LADSPA_IS_HINT_BOUNDED_BELOW(hints)){
		f_val = range_hints.LowerBound;
		if(LADSPA_IS_HINT_SAMPLE_RATE(hints) && f_val != 0){
			f_val = f_val * (LADSPA_Data)pv_samplerate;
		}
		pv_low_val = (double)f_val;
		f_low = f_val;
	}else{
		// Set a default val
		pv_low_val = pv_default_low_val;
		f_low = (LADSPA_Data)pv_default_low_val;
	}

	if(LADSPA_IS_HINT_BOUNDED_ABOVE(hints)){
		f_val = range_hints.UpperBound;
		if(LADSPA_IS_HINT_SAMPLE_RATE(hints) && f_val != 0){
			f_val = f_val * (LADSPA_Data)pv_samplerate;
		}
		pv_high_val = (double)f_val;
		f_high = f_val;
	}else{
		// Set a default val
		pv_high_val = pv_default_high_val;
		f_high = (LADSPA_Data)pv_default_high_val;
	}
	// NOTE: To test, this is a copy of float slider, shuld it be inverted ?
	if(LADSPA_IS_HINT_LOGARITHMIC(hints)){
		std::cout << "LOGa\n";
		pv_is_log = true;
		// verifiy we have somthing else than 0
		if(pv_low_val == 0){
			pv_low_val = pv_default_low_val;
		}
		if(pv_high_val == 0){
			pv_high_val = pv_default_high_val;
		}
		// Set the log scale
		th_val->setScaleEngine(log10_scale_engine);
		th_val->setScale(pv_low_val, pv_high_val);

		// Calcule the thermo range
		double d_low, d_high;
		d_low = log10(pv_low_val);
		d_high = log10(pv_high_val);

		// Set range to the thermo
		th_val->setRange(d_low, d_high);
	}else{
		// Set range to the slider and spinbox
		th_val->setRange(pv_low_val, pv_high_val);
	}

	txt_name->setText(descriptor->PortNames[port]);

	return 0;
}

void lp_ladspa_meter::reciev_val(float val)
{
	// NOTE: implement logarithmic if needed...
	th_val->setValue(val);
}
