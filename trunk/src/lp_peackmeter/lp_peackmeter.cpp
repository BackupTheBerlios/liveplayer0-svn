#include <qpainter.h>
#include <qpixmap.h>
#include <qpaintdevice.h>
#include <qlayout.h>
#include <iostream>

#include <qevent.h>
#include <qapplication.h>

#include "lp_peackmeter.h"

/**
	The peackmeter class
**/

// Public

lp_peackmeter::lp_peackmeter(QWidget *parent, const char *name, int nb_channels, int buf_size) : QWidget(parent, name)
{
	// channels
	if((nb_channels < 1)||(nb_channels > MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter::lp_peackmeter: nb_channels is not in range\n\tSet to 1\n";
		pv_channels = 1;
	}else{
		pv_channels = nb_channels;
	}

	// Defaukt audio range
	pv_range_low = 0.0001;	// Mettre -50dBu
	pv_range_high = 1.0;

	// Default clip hold time
	pv_clip_timeout = 200000;

	// Memory for buffer
	if(buf_size <= 0){
		std::cerr << "lp_peackmeter::lp_peackmeter: buf_size is in bad value!\n";
		return;
	}
	pv_buf_size = buf_size;
	pv_buffer = new float[pv_buf_size];
	if(pv_buffer == 0) {
		std::cerr << "lp_peackmeter::lp_peackmeter: cannot allocate memory for pv_buffer\n";
		pv_buffer = 0;
		return;
	}

	// FIXME:
	// I have several performance problem. With low sample_time, the clipping
	// detection works not. The value here is hazardous
	pv_minimal_sample_time = 1; // AMD Athlon 2Ghz

	// Get a dlg instance
	pv_peack_meter_dlg = new lp_peackmeter_dlg(parent, name, this);

	// Start the thread
	start();
}

lp_peackmeter::~lp_peackmeter()
{
	if(pv_buffer != 0){
		delete[] pv_buffer;
	}
}

int lp_peackmeter::run_buffer(float *buffer, int len)
{
	// Check
	if(buffer == 0){
		std::cerr << "lp_peackmeter::run_buffer: buffer is Null\n";
		return -1;
	}
	if(len < 1){
		std::cerr << "lp_peackmeter::run_buffer: buffer len is not set\n";
		return -1;
	}else{
		pv_buf_len = len;
	}
	if(len > pv_buf_size){
		std::cerr << "lp_peackmeter::run_buffer: buffer len is bigger than alocated memory\n";
		return -1;
	}

	//std::cout << "lp_peackmeter: recus " << len << " samples\n";

	int i;
	QMutexLocker locker(&pv_mutex);
	pv_buf_end = TRUE;
	// Copy buffer, wake up running thread and return
	for(i=0; i<len; i++){
		pv_buffer[i] = buffer[i];
	}

	while(pv_running == TRUE){
		usleep(100);
	}
	pv_qwc.wakeOne();

	return i+1;
}

int lp_peackmeter::get_channels()
{
	return pv_channels;
}

int lp_peackmeter::set_samplerate(int samplerate)
{
	// Sampletime
	double d_tmp1 = 0, d_tmp2 = 0;
	// Verification
	if(samplerate == 0) {
		std::cerr << "lp_peackmeter::lp_peackmeter: samplerate is Null\n";
		return -1;
	}
	// Set the sample time
	d_tmp1 = (double)samplerate;
	d_tmp2 = ((1 / (d_tmp1 * (double)pv_channels)) * 1000000);
	d_tmp2 = round(d_tmp2);
	//pv_sample_time = (unsigned int)((1 / (samplerate * pv_channels)) * 1000000);
	pv_sample_time = (unsigned int)d_tmp2;
	std::cout << "Sample time: " << (unsigned int)d_tmp2 << std::endl;

	return 0;
}

// Protected

// NOTE: runs mono for now !!
void lp_peackmeter::run()
{
	while(1){
		// Wait wakeUp
		pv_qwc.wait();
		pv_running = TRUE;
		pv_buf_end = FALSE;
		float tmp;

		int i, j;
		for(i=0; i<(pv_buf_len/pv_channels); i++) {
			for(j=0; j<pv_channels; j++){
				tmp = pv_buffer[i*pv_channels+j];
				//pv_val[j] = tmp;
	
				// Set negative to positive
				if(tmp < 0){
					tmp = -tmp;
				}

				pv_val[j] = tmp;
	
				// Get absolute vaue
				//pv_val = abs((float)pv_val);
	
				// Get RMS value
				//pv_val = sqrt((float)pv_val);
	
				// Keep buffer peack
				if(pv_val[j] > pv_last_val[j]){
					pv_last_val[j] = pv_val[j];
				}
	
				// Set 0.0 to range_low
				if(pv_val[j] < pv_range_low) {
					pv_val[j] = pv_range_low;
				}
	
				// Clip detection
				if(pv_val[j] >= pv_range_high){
					pv_val[j] = pv_range_high;
				//	std::cout << "Clip decteted ! ****\n";
					pv_start_clip();
				}
			}
			//std::cout << "Sleeping for " << pv_sample_time << "\n";
			usleep(pv_sample_time);
			// If we are to slow
			if(pv_buf_end == TRUE){
				pv_buf_end = FALSE;
				if(pv_sample_time > pv_minimal_sample_time){
					pv_sample_time--;
				}
				std::cout << "BREAK\n\t-> New sample_time: " << pv_sample_time << "\n";
				break;
			}
		}
		//std::cout << "Peak metre: Nbre de samples traites: " << i << std::endl;
		pv_running = FALSE;
	}
}

bool lp_peackmeter::get_clip()
{
	QMutexLocker locker(&pv_dlg_mutex);

/*	if(pv_clip_is_timeout() == TRUE){
		std::cout << "Clip is timeout\n";
	}
	if(pv_clip_is_timeout() == FALSE){
		std::cout << "Clip is NOT timeout\n";
	}
	if(pv_clip_to_show == TRUE){
		std::cout << "Clip is to show\n";
	}
	if(pv_clip_to_show == FALSE){
		std::cout << "Clip is  NOT to show\n";
	}
*/
	if((pv_clip_is_timeout() == FALSE)||(pv_clip_to_show == TRUE)) {
		pv_clip_to_show = FALSE;
		return TRUE;
	}else{
		return FALSE;
	}
}

float lp_peackmeter::get_value()
{
	QMutexLocker locker(&pv_dlg_mutex);
	float tmp;
	tmp = pv_last_val[0];
	//pv_val[0] = pv_last_val[0];
	pv_last_val[0] = 0;
	//return pv_val[0];
	return tmp;
}

// Private

void lp_peackmeter::pv_start_clip()
{
	gettimeofday(&pv_clip_start, 0);
	pv_clip_to_show = TRUE;
}

bool lp_peackmeter::pv_clip_is_timeout()
{
	struct timeval curent_time;
	gettimeofday(&curent_time, 0);

	if((curent_time.tv_usec + (curent_time.tv_sec * 1000000)) - (pv_clip_start.tv_usec + (pv_clip_start.tv_sec * 1000000)) >= pv_clip_timeout) {
		return TRUE;
	}else{
		return FALSE;
	}
}


/**
	lp_peackmeter_dlg class
**/

// Public

lp_peackmeter_dlg::lp_peackmeter_dlg(QWidget *parent, const char *name, lp_peackmeter *pm) : QWidget(parent, name)
{
	pv_peack_meter = pm;

	pv_channels = pm->get_channels();

	pv_bl = new QBoxLayout(this, QBoxLayout::LeftToRight, 5, 10);
	// Widget size
	setMinimumHeight(250);
	setMinimumWidth(50 * pv_channels);

	// Build widgets
	if(pv_build_widgets() < 0){
		std::cerr << "lp_peackmeter_dlg::lp_peackmeter_dlg: Warning! Unable to build Widgets\n";
		return;
	}

	int i;
	if((pv_channels >= 1)||(pv_channels <= MAX_CHANNELS)){
		for(i=0; i<pv_channels; i++){
			draw_meter(pv_lb[i], 0);
		}
	}

	pv_clip = TRUE;

	// Down factors
	pv_down_ref_time = 1500000;
	pv_hold_ref_time = 300000;
	//pv_down_factor = pow(10, (-55/20) );
	pv_down_factor = pow(10, (-20/20) );
	std::cout << "Down factor: " << pv_down_factor << std::endl;

	start();
	show();
}

lp_peackmeter_dlg::~lp_peackmeter_dlg()
{

}

// Protected
void lp_peackmeter_dlg::paintEvent(QPaintEvent*)
{
	// Verification
	if((pv_channels < 1)||(pv_channels > MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_dlg::paintEvent: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return;
	}

	// Draw
	int i;
	for(i=0; i<pv_channels; i++){
		draw_meter(pv_lb[i], 0);
	}
}

void lp_peackmeter_dlg::customEvent(QCustomEvent *event)
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_dlg::customEvent: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return;
	}

	if((int)event->type() == 8000){
		vu_event *t_event = (vu_event*)event;
		// Draw
		int i;
		for(i=0; i<pv_channels; i++){
			draw_meter(pv_lb[i], t_event->val);
			pv_txt[i]->setNum(t_event->txt_val);
		//	std::cout << "Val: " << t_event->val << " , txt_val: " << t_event->txt_val << std::endl;
		}
	}else{
		QWidget::customEvent(event);
	}
}

void lp_peackmeter_dlg::run()
{
	float val, old_val, peack_hold_val;
	struct timeval t0, t1;
	gettimeofday(&t0, 0);

	while(1){
		if(pv_peack_meter->get_clip() == TRUE){
	//		std::cout << "###### CLIP ! #######\n";
			pv_clip = TRUE;
		}else{
			pv_clip = FALSE;
		}
		val = pv_peack_meter->get_value();

		val = pv_down_val(val);
		peack_hold_val = pv_peack_hold(val);

		//std::cout << "VAL: " << val << "\n";
		// envois toutes les 1/25 sec (=0.04s --> 40ms --> 40000us)
		// Send a vu_event every 1/25 seconds (=0.04s --> 40ms --> 40000us)
		gettimeofday(&t1, 0);
		if(((t1.tv_usec + (t1.tv_sec * 1000000)) - (t0.tv_usec + (t0.tv_sec * 1000000))) >= 40000){
			//std::cout << "Intervale: " << t1.tv_usec - t0.tv_usec << std::endl;
			gettimeofday(&t0, 0);
			vu_event *event = new vu_event;

			// Traitement ICI
			event->val = pv_val_to_db(val);
			event->txt_val = pv_val_to_db(peack_hold_val);
			QApplication::postEvent(this, event);
			old_val = val;
		}

		usleep(100);
	}

}

void lp_peackmeter_dlg::draw_meter(QPaintDevice *dev, int val)
{
	QPainter paint(dev);
	QRect dim = paint.window();
	// nettoyer
	paint.setPen(QPen(Qt::white));
	paint.setBrush(QBrush(Qt::white));
	paint.drawRect(10, 250, 10, -200);

	paint.setPen(QPen(Qt::blue));
	paint.setBrush(QBrush(Qt::blue));
	// clip
	if(pv_clip == TRUE){
		paint.setBrush(QBrush(Qt::red));
		paint.drawEllipse(10, 5, 10, 10);
		paint.setBrush(QBrush(Qt::blue));
		pv_clip = FALSE;
	}else{
		paint.setBrush(QBrush(Qt::blue));
		paint.drawEllipse(10, 5, 10, 10);
	}
	paint.drawRect(10, 250, 10, -(pv_iec_scale((float)val)));
//	std::cout << "Draw: " <<  -(pv_iec_scale((float)val)) << std::endl;
}

// Private functions

// Tests: imported from meterbrigde sources
int lp_peackmeter_dlg::pv_iec_scale(float db) {
         float def = 0.0f; /* Meter deflection %age */
 
         if (db < -70.0f) {
                 def = 0.0f;
         } else if (db < -60.0f) {
                 def = (db + 70.0f) * 0.25f;
         } else if (db < -50.0f) {
                 def = (db + 60.0f) * 0.5f + 5.0f;
         } else if (db < -40.0f) {
                 def = (db + 50.0f) * 0.75f + 7.5;
         } else if (db < -30.0f) {
                 def = (db + 40.0f) * 1.5f + 15.0f;
         } else if (db < -20.0f) {
                 def = (db + 30.0f) * 2.0f + 30.0f;
         } else if (db < 0.0f) {
                 def = (db + 20.0f) * 2.5f + 50.0f;
         } else {
                 def = 100.0f;
         }
 
         return (int)(def * 2.0f);
}

int lp_peackmeter_dlg::pv_val_to_db(float f_val)
{
	return (int)(20*((float)log10(f_val)));
}

float lp_peackmeter_dlg::pv_down_val(float act_val)
{
	struct timeval actual_time;
	gettimeofday(&actual_time, 0);

	unsigned int actual_usec;	// time in usec
	actual_usec = actual_time.tv_usec + (actual_time.tv_sec * 1000000);

	unsigned int last_usec;		// last time in usec
	last_usec = pv_last_time.tv_usec + (pv_last_time.tv_sec * 1000000);

	unsigned int delta_t;
	delta_t = actual_usec - last_usec;

	float tmp_val;

	if(act_val < pv_last_val){
		tmp_val = pv_last_peack * ((pv_down_factor * pv_down_ref_time) / delta_t );

		if(tmp_val < pv_last_val){
			pv_last_val = tmp_val;
		}
		return pv_last_val;
	}else{
		// reset timer
//		std::cout << "PEAK - Reset timer ###################################################\n";
		pv_last_peack = act_val;
		pv_last_val = act_val;
		gettimeofday(&pv_last_time, 0);
	}

	return act_val;
}

float lp_peackmeter_dlg::pv_peack_hold(float act_val)
{
	struct timeval actual_time;
	gettimeofday(&actual_time, 0);

	unsigned int actual_usec;	// time in usec
	actual_usec = actual_time.tv_usec + (actual_time.tv_sec * 1000000);

	unsigned int last_usec;		// last time in usec
	last_usec = pv_last_hold_time.tv_usec + (pv_last_hold_time.tv_sec * 1000000);

	unsigned int delta_t;
	delta_t = actual_usec - last_usec;

	float tmp_val;

	if(delta_t >= pv_hold_ref_time){
		tmp_val = act_val;
		gettimeofday(&pv_last_hold_time, 0);
		pv_last_hold_peack = act_val;
	//	std::cout << "HOLD (Reset): return " << tmp_val << std::endl;
	}else{
		tmp_val = pv_last_hold_peack;
	//	std::cout << "HOLD: return " << tmp_val << std::endl;
	}

	return tmp_val;
}

int lp_peackmeter_dlg::pv_build_widgets()
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "vu_meter::pv_build_widgets: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return -1;
	}

	// Build
	int i;
	for(i=0; i<pv_channels; i++){
		pv_hbl[i] = new QBoxLayout(this, QBoxLayout::TopToBottom, 5, 10);
		pv_lb[i] = new QLabel(this);
		pv_lb[i]->setBackgroundColor(Qt::green);
		pv_lb[i]->setMinimumHeight(300);
		pv_hbl[i]->addWidget(pv_lb[i], -1, 0);
		// Text label
		pv_txt[i] = new QLabel(this);
		pv_txt[i]->setAlignment(Qt::AlignHCenter);
		pv_txt[i]->setText("0");
		pv_txt[i]->setBackgroundColor(Qt::green);
		pv_txt[i]->setMaximumHeight(30);
		pv_hbl[i]->addWidget(pv_txt[i], -1, 0);
		// Add to horizontal layout
		pv_bl->addLayout(pv_hbl[i], -1);
	}
	return 0;
}

/* Peackmeter event implementation */
vu_event::vu_event() : QCustomEvent(8000)
{
}
