#ifndef LP_PEACKMETER_H
#define LP_PEACKMETER_H

#include "lp_peackmeter_core.h"
#include "lp_peackmeter_widget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <math.h>
#include <sys/time.h>

class lp_peackmeter : public QWidget
{
Q_OBJECT
public:
	lp_peackmeter(QWidget *parent = 0,  int _nb_channels = 0, int _buf_size = 0);
	~lp_peackmeter();
	lp_peackmeter_core* get_core();
	void set_range(int dB_min, int dB_max);
	void set_refresh_freq(int freq);	// Set the refresh frequency
private slots:
	void set_val(float *val);
private:
	lp_peackmeter_core *pm_core;
	lp_peackmeter_widget *pm_widget[lp_peackmeter_core::MAX_CHANNELS];
	QHBoxLayout *b_layout;
	int nb_channels;
	// Calculate a instant value depend on time (peack_meter goes down with 55dB/1.5 sec in std)
	float down_val(float act_val, int channel);
	struct timeval pv_last_time[lp_peackmeter_core::MAX_CHANNELS];
	float pv_last_val[lp_peackmeter_core::MAX_CHANNELS];
	float pv_last_peack[lp_peackmeter_core::MAX_CHANNELS];
	float pv_down_factor;
	unsigned int pv_down_ref_time;

	// Hold a value for some time (Used to display numerical values slow, else there ares illisible)
	float pv_peack_hold(float act_val, int channel);
	float pv_last_hold_peack[lp_peackmeter_core::MAX_CHANNELS];		// last value for peack hold
	unsigned int pv_hold_ref_time;						// ref. for peack hold
	struct timeval pv_last_hold_time[lp_peackmeter_core::MAX_CHANNELS]; 	// elapsed time at last peack - for peack hold

	int val_to_db(float f_val);
};

#endif
