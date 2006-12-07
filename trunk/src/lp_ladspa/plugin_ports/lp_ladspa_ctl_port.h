#ifndef LP_LADSPA_CTL_PORT_H
#define LP_LADSPA_CTL_PORT_H

#include <QThread>
#include <QMutex>
#include <ladspa.h>

#include "lp_ladspa_slider_float.h"
#include "lp_ladspa_knob_float.h"
#include "lp_ladspa_slider_int.h"
#include "lp_ladspa_toggel.h"
#include "lp_ladspa_meter.h"
#include "lp_ladspa_handle.h"
#include "lp_ladspa_layout.h"

// This class should be thread safe
// It's used from gui ports to set values
// and from porcessing plugin core to get values
// Only connect_port() and set_value() are threadsafe !
// Instanciate and init this class from main thread !
class lp_ladspa_ctl_port : public QThread
{
Q_OBJECT
public:
	lp_ladspa_ctl_port(QObject *parent = 0);
	~lp_ladspa_ctl_port();
	int init_port(const LADSPA_Descriptor *descriptor = 0, unsigned long port = 0, int samplerate = 0
		, QGridLayout *layout = 0, int _row = 0, int _col = 0);
	int instantiate(lp_ladspa_handle *instances, int handles_count);
	void connect_port();

public slots:
	void set_value(float val);
//	void set_value(int val);

private:
	QMutex mutex;
	LADSPA_Data pv_val, *pv_run_val;
	const LADSPA_Descriptor *pv_descriptor;
	lp_ladspa_handle *pv_instances;	// Array of lp_ladspa_handle
	int pv_handles_count;		// Store the number of handles needed
	unsigned long pv_port;
	lp_ladspa_layout custom_layout;
	lp_ladspa_slider_float *float_slider;
	lp_ladspa_knob_float *float_knob;
	lp_ladspa_slider_int *int_slider;
	lp_ladspa_toggel *toggel;
	lp_ladspa_meter *meter;
	bool pv_is_output;
	void run();
};

#endif
