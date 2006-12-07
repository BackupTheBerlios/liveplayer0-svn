#ifndef LP_LADSPA_PLUGIN_H
#define LP_LADSPA_PLUGIN_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QString>

#include <iostream>
#include <ladspa.h>

#include "plugin_ports/lp_ladspa_handle.h"
#include "plugin_ports/lp_ladspa_ctl_port.h"
#include "plugin_ports/lp_ladspa_audio_ports.h"
#include "plugin_ports/lp_ladspa_slider_float.h"
#include "../lp_peackmeter/lp_peackmeter.h"
#include "../lp_peackmeter/lp_peackmeter_core.h"


class lp_ladspa_plugin : public QWidget, QThread
{
public:
	lp_ladspa_plugin(QWidget *parent = 0);
	~lp_ladspa_plugin();
	int init(const LADSPA_Descriptor *descriptor, int channels, int buf_size, int samplerate);
	/// Activate the plugin audio ports (if plugin don't support this,
	///  the active/unactive is allways supported here)
	void activate();
	void deactivate();
	bool is_active();
	/// Runs a buffer of data - if plugin is unactiv, returns 0. if err, returns -1
	/// else, it returns the number of element processed (= tot_len)
	int run_interlaced_buffer(float *buffer, int tot_len);
	QString get_name();

protected:
	void run();

private:
	QWidget *base_widget;
	QGridLayout *layout;
	QLabel *txt_routing;
	// The input gain slider
	lp_ladspa_slider_float *gain_slider;
	float pv_gain;
	// peackmeters
	lp_peackmeter *in_peackmeter, *out_peackmeter;
	lp_peackmeter_core *in_pmc, *out_pmc;
	// A array of control port class
	lp_ladspa_ctl_port *pv_ctl_ports;
	int pv_ctl_count;
	// A instance of audio ports class
	lp_ladspa_audio_ports *pv_audio_ports;
	lp_ladspa_handle *pv_instances;
	int pv_handles_count;		// Store the number of handles needed
	bool pv_has_activate;
	bool pv_has_deactivate;
	bool pv_active;
	const LADSPA_Descriptor *pv_descriptor;
	float *pv_buffer;
	int pv_len;
	int pv_retval;

	// Threading vars
	volatile bool in_process;
	volatile bool end;
	QWaitCondition new_process;
	QWaitCondition end_process;
	QMutex mutex;
};

#endif
