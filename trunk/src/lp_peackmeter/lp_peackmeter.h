#ifndef LP_PEAKMETER_H
#define LP_PEAKMETER_H

#include <qwidget.h>
#include <qlabel.h>
#include <qthread.h>
#include <qmutex.h>

#include <sys/time.h>
#include <unistd.h>
#include <math.h>

class QBoxLayout;
class lp_peackmeter_dlg;

/// This class is to instanciate for use
class lp_peackmeter : public QThread, QWidget
{
	public:
	lp_peackmeter( QWidget *parent = 0, const char *name = 0, int nb_channels = 1, int buf_size = 0);
	~lp_peackmeter();
	int run_buffer(float *buffer, int len);
	bool get_clip();
	float get_value();
	int get_channels();
	int set_samplerate(int samplerate = 0);	// To change / set samplerate

	protected:
	virtual void run();

	private:
	// some constants
	static const int MAX_CHANNELS = 20;

	// Audio
	float pv_range_low;	// For range ( 0.001 for example)
	float pv_range_high;	// For range ( 1.0 for example)
	float pv_val[MAX_CHANNELS];		// Instant value
	float pv_last_val[MAX_CHANNELS];	// Store last value (store peakcs)
	int pv_s_rate;		// Audio samplerate
	int pv_buf_size;	// Buffer size for allocation
	int pv_buf_len;		// Buffer len (can call smaller size than buf_len)
	float *pv_buffer;
	int pv_channels;

	// Timer
	//unsigned int pv_buffer_time;
	unsigned int pv_sample_time;
	unsigned int pv_minimal_sample_time;	// Have speed problems. With to low sleep_time, the clipping works bad
	struct timeval pv_clip_start;		// Stores the start time of clipping
	void pv_start_clip();			// Start clip timer (stores current time in pv_clip_start)
	int pv_clip_timeout;			// Store the clip timeout time (in usec)
	bool pv_clip_to_show;			// Store clip status: after detection, if timeout AND showed, it goes FALSE
	bool pv_clip_is_timeout();		// Returns TRUE if timeout is reached

	bool pv_buf_end;			// Whenn new call, reset process and run new buffer
	bool pv_running;			// Whenn new call and process is not properly aborted: wait
	QWaitCondition pv_qwc;
	QMutex pv_mutex;
	QMutex pv_dlg_mutex;

	lp_peackmeter_dlg *pv_peack_meter_dlg;
};

// This is used from lp_peackmeter class
class lp_peackmeter_dlg : public QWidget, QThread
{
	Q_OBJECT

	public:
	lp_peackmeter_dlg(QWidget *parent = 0, const char *name = 0, lp_peackmeter *pm = 0);
	~lp_peackmeter_dlg();

	protected:
	virtual void paintEvent(QPaintEvent*);
	virtual void customEvent(QCustomEvent *event);
	virtual void run();

	private:
	// some constants
	static const int MAX_CHANNELS = 20;

	int pv_channels;
	bool pv_clip;

	// Convert a float value to dB val
	int pv_val_to_db(float f_val);
	// Calculate a instant value depend on time (peack_meter goes down with 55dB/1.5 sec in std)
	float pv_down_val(float act_val);
	float pv_peack_hold(float act_val);

	unsigned int pv_down_ref_time;	// ref. time: 1.5sec in std -> 1 500 000us
	unsigned int pv_hold_ref_time;	// ref. for peack hold
	struct timeval pv_last_time;	// elapsed time at last peack
	struct timeval pv_last_hold_time; // elapsed time at last peack - for peack hold
	float pv_down_factor;		// down factor: -55dB in std -> pow(10, (-55/20))
	float pv_last_peack;		// Last peack value
	float pv_last_hold_peack;	// last value for peack hold
	float pv_last_val;		// Store previous value

	lp_peackmeter *pv_peack_meter;		// The peack_meter instance

	// Qt objects
	QBoxLayout *pv_bl;
	QBoxLayout *pv_hbl[MAX_CHANNELS];
	QLabel *pv_lb[MAX_CHANNELS];		// The vu_meter
	QLabel *pv_txt[MAX_CHANNELS];		// The text
	QWaitCondition pv_qwc;
	QMutex pv_mutex;

	void draw_meter(QPaintDevice *dev, int val);

	// Build widgets
	int pv_build_widgets();

	// Tests: import from meterbridge sources
	int pv_iec_scale(float db);
};

// The event class to transmission between lp_peackmeter_dlg's run thread and main thread
class vu_event : public QCustomEvent
{
	public:
	vu_event();
	int val;
	int txt_val;
};

#endif // LP_PEAKMETER_H
