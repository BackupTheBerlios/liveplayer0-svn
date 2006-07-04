#ifndef M_WIDGET_H
#define M_WIDGET_H

#include <qwidget.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qthread.h>
#include <qmutex.h>

#include <sys/time.h>
#include <unistd.h>
#include <math.h>

class QBoxLayout;

class vu_meter : public QWidget, QThread
{
	Q_OBJECT

	public:
	vu_meter( QWidget *parent = 0, const char *name = 0, int nb_channels = 1);
	~vu_meter();
	// Set audio parameters
	int set_samplerate(int samplerate);
	int alloc_mem(int buf_size);
	// Transmit buffer to display
	int run_buffer(float *buffer, int buf_len);

	protected:
	virtual void paintEvent(QPaintEvent*);
	virtual void customEvent(QCustomEvent *event);
	virtual void run();
	void draw_vu(QPaintDevice *dev, int val);

	private:
	// some constants
	static const int MAX_CHANNELS = 20;

	// Audio params
	int pv_channels;

	// Data
	float pv_range_low;	// For range ( 0.001 for example)
	float pv_range_high;	// For range ( 1.0 for example)
	float pv_val;		// Instant value
	float *pv_buffer;
	int pv_s_rate;		// Audio samplerate
	unsigned int pv_sample_time;
	int pv_buf_size;
	bool pv_buf_end;	// Whenn new call, reset process and run new buffer
	float pv_range_factor;	// For conversion to int ( 0.658 becomes 65 for exemple )
	int pv_set_range_factor();
	bool pv_clip;
	unsigned int pv_clip_hold_time;

	// Convert a float value to dB val
	int pv_val_to_db(float f_val);

	// Qt objects
	QBoxLayout *pv_bl;
	QBoxLayout *pv_hbl[MAX_CHANNELS];
	QLabel *pv_lb[MAX_CHANNELS];	// The vu_meter
	QLabel *pv_txt[MAX_CHANNELS];	// The text
	QWaitCondition pv_qwc;
	bool pv_running;
	QMutex pv_mutex;

	// Time
	struct timeval pv_time_v;

	// Build widgets
	int pv_build_widgets();

};

class vu_event : public QCustomEvent
{
	public:
	vu_event();
	int val;
};

#endif
