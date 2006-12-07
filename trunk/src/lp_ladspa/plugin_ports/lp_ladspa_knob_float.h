#ifndef LP_LADSPA_KNOB_FLOAT
#define LP_LADSPA_KNOB_FLOAT

#include <ladspa.h>
#include <ui_lp_ladspa_knob_float.h>
#include <qwt_scale_engine.h>

class lp_ladspa_knob_float : public QWidget, Ui::lp_ladspa_knob_float
{
Q_OBJECT
public:
	lp_ladspa_knob_float(QWidget *parent = 0);
	~lp_ladspa_knob_float();
	int init(const LADSPA_Descriptor *descriptor
		, unsigned long port
		, int samplerate);
	void set_def_val();
	void set_def_val(double val);

public slots:
	void pos_def_val();
	void set_sb_val(double val);
	void set_kb_val(double val);
	void emit_changed(double val);

signals:
	void val_changed(float val);

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
