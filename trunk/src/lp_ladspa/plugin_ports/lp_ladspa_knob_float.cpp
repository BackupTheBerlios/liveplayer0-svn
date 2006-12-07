#include "lp_ladspa_knob_float.h"
#include <iostream>

#include "math.h"

#include <qwt_slider.h>


lp_ladspa_knob_float::lp_ladspa_knob_float(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	log10_scale_engine = new QwtLog10ScaleEngine;

	// Initial defaults
	pv_default_low_val = 0.001;
	pv_default_high_val = 10.0;
	pv_is_log = false;
	pv_def_val = 1.0;

	/// Look to clean this connections !!!
	//connect(kb_val, SIGNAL(valueChanged(double)), sp_val, SLOT(setValue(double)));
	connect(kb_val, SIGNAL(valueChanged(double)), this, SLOT(emit_changed(double)));
	connect(kb_val, SIGNAL(valueChanged(double)), this, SLOT(set_sb_val(double)));
	connect(sp_val, SIGNAL(valueChanged(double)), this, SLOT(set_kb_val(double)));
	//connect(sp_val, SIGNAL(valueChanged(double)), kb_val, SLOT(setValue(double)));
	//connect(sp_val, SIGNAL(valueChanged(double)), this, SLOT(emit_changed(double)));
	connect(pb_def_val, SIGNAL(clicked()), this, SLOT(pos_def_val()));
}

lp_ladspa_knob_float::~lp_ladspa_knob_float()
{

}

int lp_ladspa_knob_float::init(const LADSPA_Descriptor *descriptor,
				 unsigned long port,
				 int samplerate)
{
	LADSPA_Data f_val, f_low, f_high;

	if(descriptor == 0){
		std::cerr << "lp_ladspa_slider_int::" << __FUNCTION__ << ": pdescriptor is Null\n";
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

	if(LADSPA_IS_HINT_LOGARITHMIC(hints)){
		pv_is_log = true;
		// verifiy we have somthing else than 0
		if(pv_low_val == 0){
			pv_low_val = pv_default_low_val;
		}
		if(pv_high_val == 0){
			pv_high_val = pv_default_high_val;
		}
		// Set the log scale
		kb_val->setScaleEngine(log10_scale_engine);
		kb_val->setScale(pv_low_val, pv_high_val);
		// Set this range to the spinbox
		sp_val->setRange(pv_low_val, pv_high_val);

		// Calcule the slider range
		double d_low, d_high;
		d_low = log10(pv_low_val);
		d_high = log10(pv_high_val);

		// Set range to the slider
		kb_val->setRange(d_low, d_high);
	}else{
		// Set range to the slider and spinbox
		kb_val->setRange(pv_low_val, pv_high_val);
		sp_val->setRange(pv_low_val, pv_high_val);
	}

	// Store the range in f_val's
	f_low = (LADSPA_Data)pv_low_val;
	f_high = (LADSPA_Data)pv_high_val;

	// Default values
	if(LADSPA_IS_HINT_HAS_DEFAULT(hints)){
		if(LADSPA_IS_HINT_DEFAULT_MINIMUM(hints)){
			pv_def_val = pv_low_val;
		}
		if(LADSPA_IS_HINT_DEFAULT_MAXIMUM(hints)){
			pv_def_val = pv_high_val;
		}
		if(LADSPA_IS_HINT_DEFAULT_LOW(hints)){
			if(LADSPA_IS_HINT_LOGARITHMIC(hints)){
				f_val = exp(log(f_low)*0.75f + log(f_high)*0.25f);
			}else{
				f_val = (f_low*0.75f + f_high*0.25f);
			}
		}
		if(LADSPA_IS_HINT_DEFAULT_MIDDLE(hints)){
			if(LADSPA_IS_HINT_LOGARITHMIC(hints)){
				f_val = exp(log(f_low)*0.5f + log(f_high)*0.5f);
			}else{
				f_val = (f_low*0.5f + f_high*0.5f);
			}
		}
		if(LADSPA_IS_HINT_DEFAULT_HIGH(hints)){
			if(LADSPA_IS_HINT_LOGARITHMIC(hints)){
				f_val = exp(log(f_low)*0.25f + log(f_high)*0.75f);
			}else{
				f_val = (f_low*0.25f + f_high*0.75f);
			}
		}
		if(LADSPA_IS_HINT_DEFAULT_0(hints)){
			f_val = 0.0f;
		}
		if(LADSPA_IS_HINT_DEFAULT_1(hints)){
			f_val = 1.0f;
		}
		if(LADSPA_IS_HINT_DEFAULT_100(hints)){
			f_val = 100.0f;
		}
		if(LADSPA_IS_HINT_DEFAULT_440(hints)){
			f_val = 440.0f;
		}
		// Set default value
		pv_def_val = (double)f_val;
	}else{
		pv_def_val = pv_low_val;
	}

	txt_name->setText(descriptor->PortNames[port]);

	// set the middle value
	set_def_val();
	emit_changed(pv_def_val);

	return 0;
}

void lp_ladspa_knob_float::set_def_val()
{
	set_kb_val(pv_def_val);
	sp_val->setValue(pv_def_val);
	//emit(val_changed((float)pv_def_val));
}

void lp_ladspa_knob_float::set_def_val(double val)
{
	set_kb_val(val);
	sp_val->setValue(val);
}

void lp_ladspa_knob_float::pos_def_val()
{
	set_kb_val(pv_def_val);
	sp_val->setValue(pv_def_val);
	emit(val_changed((float)pv_def_val));
}

void lp_ladspa_knob_float::set_sb_val(double val)
{
	double d_val;
	if(pv_is_log == true){
		d_val = pow(10.0, val);
	}else{
		d_val = val;
	}
	if(sp_val->value() != d_val){
		sp_val->setValue(d_val);
	}
}

void lp_ladspa_knob_float::set_kb_val(double val)
{
	double d_val;
	if(pv_is_log == true){
		d_val = log10(val);
	}else{
		d_val = val;
	}
	if(kb_val->value() != d_val){
		kb_val->setValue(d_val);
	}
}

void lp_ladspa_knob_float::emit_changed(double val)
{
	if(val != pv_last_val){
		pv_last_val = val;
		//sp_val->setValue(val);
		if(pv_is_log == true){
			val = pow(10.0, val);
			//val = exp(val);
		}
		emit(val_changed((float)val));
	}
}
