#ifndef LP_PEACKMETER_CORE_H
#define LP_PEACKMETER_CORE_H

#include <math.h>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

//#include "../lp_timer/lp_timer.h"
#include "lp_utils/lp_timer/lp_timer.h"
#include "lp_peackmeter_widget.h"

class lp_peackmeter_core : public QThread
{
	Q_OBJECT

public:
	lp_peackmeter_core(QObject *parent = 0, int _nb_channels = 0, int _buf_size = 0);
	~lp_peackmeter_core();
	void set_range(float min, float max);
	int run_interlaced_buffer(float *_buffer, int _len);
	bool get_clip(int _channel);
	void set_refresh_time(unsigned int u_secs);
	// Constants
	static const int MAX_CHANNELS = 20;

signals:
	void get_val(float *val);

protected:
	void run();

private:

	// audio process function, vars
	int nb_channels;
	float range_low;		// For range ( 0.001 for example)
	float range_high;		// For range ( 1.0 for example)
	float val[MAX_CHANNELS];	// Instant value
	float last_val[MAX_CHANNELS];// Store last value (store peaks)
	int buf_size;			// Buffer size for allocation
	int buf_len;			// Buffer len (can call smaller size than buf_len)
	float *buffer;			// Stores the audio samples

	// Clip detection
	void start_clip(int _channel);
	bool clip_to_show[MAX_CHANNELS];

	// Timer
	lp_timer clip_timer[MAX_CHANNELS];

	// Data requierd by running process
	QMutex mutex;
	QWaitCondition condition;
	volatile bool end_process;
	unsigned int clip_timeout;
	int pv_i;			// Counter, used by run_interlaced_buffer()
	lp_timer send_timer;
};

#endif
