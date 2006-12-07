#include "lp_ladspa_toggel.h"
#include <iostream>

lp_ladspa_toggel::lp_ladspa_toggel(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	connect(pb_val, SIGNAL(clicked()), this, SLOT(emit_changed()));
}

lp_ladspa_toggel::~lp_ladspa_toggel()
{

}

int lp_ladspa_toggel::init(const LADSPA_Descriptor *descriptor,
				 unsigned long port,
				 int samplerate)
{
	if(descriptor == 0){
		std::cerr << "lp_ladspa_toggel::" << __FUNCTION__ << ": pdescriptor is Null\n";
		return -1;
	}

//	LADSPA_Data f_val;

	LADSPA_PortRangeHintDescriptor hints;
	hints = descriptor->PortRangeHints[port].HintDescriptor;

	LADSPA_PortRangeHint range_hints;
	range_hints = descriptor->PortRangeHints[port];

	if(LADSPA_IS_HINT_HAS_DEFAULT(hints)){
		if(LADSPA_IS_HINT_DEFAULT_0(hints)){
			//f_val = 0.0f;
			set_off();
		}else if(LADSPA_IS_HINT_DEFAULT_1(hints)){
			//f_val = 1.0f;
			set_on();
		}else{
			// Considere it off
			//f_val = 0.0f;
			set_off();
		}
	}

	txt_name->setText(descriptor->PortNames[port]);
	emit_changed();

	return 0;
}

void lp_ladspa_toggel::set_on()
{
	pb_val->setChecked(true);
	pb_val->setText(tr("On"));
	emit(val_changed(1.0));
}

void lp_ladspa_toggel::set_off()
{
	pb_val->setChecked(false);
	pb_val->setText(tr("Off"));
	emit(val_changed(0.0));
}

void lp_ladspa_toggel::emit_changed()
{
	if(pb_val->isChecked() == true){
		emit(val_changed(1.0));
		pb_val->setText(tr("On"));
	}else{
		emit(val_changed(0.0));
		pb_val->setText(tr("Off"));
	}
}
