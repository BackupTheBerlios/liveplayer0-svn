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

/** This class is to instanciate for use
    Please get the instance from the main thread (GUI thread)
    After this, you cann call the public function from another thread
    NOTE: you should test this..
**/

class lp_peackmeter : public QThread, QWidget
{
	public:
	/// Here is the constructor to use for instanciation
	/// Give your widget as argument to put the peack-meter on it
	lp_peackmeter( QWidget *parent = 0, const char *name = 0, int nb_channels = 1, int buf_size = 0);
	~lp_peackmeter();
	/// Use this to set / change the samplerate
	int set_samplerate(int samplerate = 0);	// To change / set samplerate
	/// Use this function to run the samples
	int run_buffer(float *buffer, int len);

	// These are specific functions called by lp_peackmeter_dlg
	// for interactions. Please do not use tis functions
	bool get_clip(int channel);
	float get_value(int channel);
	int get_channels();

	protected:
	virtual void run();

	private:
	// some constants
	static const int MAX_CHANNELS = 20;

	// Audio
	float pv_range_low;		// For range ( 0.001 for example)
	float pv_range_high;		// For range ( 1.0 for example)
	float pv_val[MAX_CHANNELS];	// Instant value
	float pv_last_val[MAX_CHANNELS];// Store last value (store peaks)
	int pv_s_rate;			// Audio samplerate
	int pv_buf_size;		// Buffer size for allocation
	int pv_buf_len;			// Buffer len (can call smaller size than buf_len)
	float *pv_buffer;		// Stores the audio samples
	int pv_channels;

	// Timer
	unsigned int pv_sample_time;			// Time to sleep after each samples
	unsigned int pv_minimal_sample_time;		// To store the minimal sample_time (I use 1us)

	// Clip detection
	struct timeval pv_clip_start[MAX_CHANNELS];	// Stores the start time of clipping
	void pv_start_clip(int channel);		// Start clip timer (stores current time in pv_clip_start)
	int pv_clip_timeout;				// Store the clip timeout time (in usec)
	bool pv_clip_to_show[MAX_CHANNELS];		// Store clip status: after detection, if timeout AND showed, it goes FALSE
	bool pv_clip_is_timeout(int channel);		// Returns TRUE if timeout is reached

	bool pv_buf_end;				// Whenn new call, reset process and run new buffer
	bool pv_running;				// Whenn new call and process is not properly aborted: wait
	QWaitCondition pv_qwc;
	QMutex pv_mutex;
	QMutex pv_dlg_mutex;

	lp_peackmeter_dlg *pv_peack_meter_dlg;		// Store the dlg instance here
};

// This is used from lp_peackmeter class
class lp_peackmeter_dlg : public QWidget, QThread
{
	Q_OBJECT

	public:
	lp_peackmeter_dlg(QWidget *parent = 0, const char *name = 0, lp_peackmeter *pm = 0);
	~lp_peackmeter_dlg();

	// Set meter size: 1 = little, 2 = medium, 3 = big
	void set_meter_size(int size);

	protected:
	virtual void paintEvent(QPaintEvent*);
	virtual void customEvent(QCustomEvent *event);
	virtual void run();

	private:
	// some constants
	static const int MAX_CHANNELS = 20;

	int pv_channels;

	// Clip display
	bool pv_clip[MAX_CHANNELS];

	int pv_val_to_db(float f_val);		// Convert a float value to dB val

	// Calculate a instant value depend on time (peack_meter goes down with 55dB/1.5 sec in std)
	float pv_down_val(float act_val, int channel);
	float pv_down_factor;				// down factor: -55dB in std -> pow(10, (-55/20))
	unsigned int pv_down_ref_time;			// ref. time: 1.5sec in std -> 1 500 000us
	struct timeval pv_last_time[MAX_CHANNELS];	// elapsed time at last peack
	float pv_last_peack[MAX_CHANNELS];		// Last peack value
	float pv_last_val[MAX_CHANNELS];		// Store previous value

	// Import from meterbridge sources (http://plugin.org.uk/meterbridge/)
	int pv_iec_scale(float db);			// Give the meter the scale

	// Hold a value for some time (Used to display numerical values slow, else there ares illisible)
	float pv_peack_hold(float act_val, int channel);
	float pv_last_hold_peack[MAX_CHANNELS];		// last value for peack hold
	unsigned int pv_hold_ref_time;			// ref. for peack hold
	struct timeval pv_last_hold_time[MAX_CHANNELS]; // elapsed time at last peack - for peack hold

	lp_peackmeter *pv_peack_meter;			// The peack_meter instance

	// Qt objects
	QBoxLayout *pv_bl;				// Main box layout
	QBoxLayout *pv_hbl[MAX_CHANNELS];		// Horizontal box layout
	QLabel *pv_lb[MAX_CHANNELS];			// The vu_meters
	QLabel *pv_txt[MAX_CHANNELS];			// The texts
	QWaitCondition pv_qwc;
	QMutex pv_mutex;
	QMutex pv_cfg_mutex;

	// Dimentions
	int pv_meter_height;		// The "bargraph" height

	// Build widgets
	int pv_build_widgets();

	void draw_meter(QPaintDevice *dev, int val, int channel);
};

// The event class to transmission between lp_peackmeter_dlg's run thread and main thread
class vu_event : public QCustomEvent
{
	private:
	// some constants
	static const int MAX_CHANNELS = 20;

	public:
	vu_event();
	int val[MAX_CHANNELS];
	int txt_val[MAX_CHANNELS];
};

#endif // LP_PEAKMETER_H
