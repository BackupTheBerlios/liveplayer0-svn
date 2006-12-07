#ifndef LP_LADSPA_TOGGLE_H
#define LP_LADSPA_TOGGLE_H

#include <ladspa.h>
#include <ui_lp_ladspa_toggel.h>

class lp_ladspa_toggel : public QWidget, Ui::lp_ladspa_toggel
{
Q_OBJECT
public:
	lp_ladspa_toggel(QWidget *parent = 0);
	~lp_ladspa_toggel();
	int init(const LADSPA_Descriptor *descriptor
		, unsigned long port
		, int samplerate);

public slots:
	void set_on();
	void set_off();
	void emit_changed();

signals:
	void val_changed(float val);

private:
	LADSPA_Data pv_val;
	LADSPA_Data pv_def_val;
};

#endif
