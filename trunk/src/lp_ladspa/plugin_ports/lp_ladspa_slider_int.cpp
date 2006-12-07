#include "lp_ladspa_slider_int.h"
#include <iostream>

#include "math.h"


lp_ladspa_slider_int::lp_ladspa_slider_int(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	/// Look to clean this connections !!!
	//connect(sl_val, SIGNAL(valueChanged(int)), sp_val, SLOT(setValue(int)));
	connect(sl_val, SIGNAL(valueChanged(int)), this, SLOT(emit_changed(int)));
	connect(sp_val, SIGNAL(valueChanged(int)), sl_val, SLOT(setValue(int)));
	connect(pb_def_val, SIGNAL(clicked()), this, SLOT(pos_def_val()));
}

lp_ladspa_slider_int::~lp_ladspa_slider_int()
{

}

int lp_ladspa_slider_int::init(const LADSPA_Descriptor *descriptor,
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
		if(LADSPA_IS_HINT_SAMPLE_RATE(hints)){
			f_val = f_val * (LADSPA_Data)pv_samplerate;
		}
		pv_low_val = (int)f_val;
		f_low = f_val;
	}else{
		// Set a default val
		pv_low_val = 0;
		f_low = 0.0f;
	}

	if(LADSPA_IS_HINT_BOUNDED_ABOVE(hints)){
		f_val = range_hints.UpperBound;
		if(LADSPA_IS_HINT_SAMPLE_RATE(hints)){
			f_val = f_val * (LADSPA_Data)pv_samplerate;
		}
		pv_high_val = (int)f_val;
		f_high = f_val;
	}else{
		// Set a default val
		pv_high_val = 10;
		f_high = 10.0f;
	}

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
		pv_def_val = (int)f_val;
	}

	// Some plugins gives a range from 0 to 0, set a default range...
	if(pv_low_val == 0) { pv_low_val = 0;}
	if(pv_high_val == 0){ pv_high_val = 10;}

	// Set range to the slider and spinbox
	sl_val->setRange(pv_low_val, pv_high_val);
	sp_val->setRange(pv_low_val, pv_high_val);

	txt_name->setText(descriptor->PortNames[port]);

	// set the middle value
	set_def_val();
	emit_changed(pv_def_val);

	return 0;
}

void lp_ladspa_slider_int::set_def_val()
{
	sl_val->setSliderPosition(pv_def_val);
	sp_val->setValue(pv_def_val);
}

void lp_ladspa_slider_int::set_def_val(int val)
{
	sl_val->setSliderPosition(val);
	sp_val->setValue(val);
}

void lp_ladspa_slider_int::pos_def_val()
{
	sl_val->setSliderPosition(pv_def_val);
	sp_val->setValue(pv_def_val);
	emit(val_changed((float)pv_def_val));
}

void lp_ladspa_slider_int::emit_changed(int val)
{
	if(val != pv_last_val){
		pv_last_val = val;
		sp_val->setValue(val);
		emit(val_changed((float)val));
	}
}
