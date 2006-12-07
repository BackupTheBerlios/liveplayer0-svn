#ifndef LP_LADSPA_SLIDER_INT
#define LP_LADSPA_SLIDER_INT

#include <ladspa.h>
#include <ui_lp_ladspa_slider_int.h>

class lp_ladspa_slider_int : public QWidget, Ui::lp_ladspa_slider_int
{
Q_OBJECT
public:
	lp_ladspa_slider_int(QWidget *parent = 0);
	~lp_ladspa_slider_int();
	int init(const LADSPA_Descriptor *descriptor
		, unsigned long port
		, int samplerate);
	void set_def_val();
	void set_def_val(int val);

public slots:
	void pos_def_val();
	void emit_changed(int val);

signals:
	void val_changed(float val);

private:
	int pv_low_val;
	int pv_high_val;
	int pv_def_val;
	int pv_samplerate;
	int pv_last_val;
};

#endif
