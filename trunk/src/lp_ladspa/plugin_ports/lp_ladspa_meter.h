#ifndef LP_LADSPA_METER_H
#define LP_LADSPA_METER_H

#include <ui_lp_ladspa_meter.h>
#include <ladspa.h>
#include <qwt_thermo.h>
#include <qwt_scale_engine.h>

class lp_ladspa_meter : public QWidget, Ui::lp_ladspa_meter
{
Q_OBJECT
public:
	lp_ladspa_meter(QWidget *parent = 0);
	~lp_ladspa_meter();
	int init(const LADSPA_Descriptor *descriptor
		, unsigned long port
		, int samplerate);

public slots:
	void reciev_val(float val);

private:
	double pv_low_val;
	double pv_default_low_val;
	double pv_high_val;
	double pv_default_high_val;
	double pv_def_val;
	double pv_samplerate;
	double pv_last_val;
	bool pv_is_log;
	QwtLog10ScaleEngine *log10_scale_engine;
};

#endif
