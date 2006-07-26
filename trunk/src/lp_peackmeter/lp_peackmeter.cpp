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
	// Initialize clip system...
	int i;
	for(i=0; i<pv_channels; i++){
		pv_start_clip(i);
	}

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
	// I have several performance problem. With low sample_time. The value here is hazardous
	pv_minimal_sample_time = 1; // AMD Athlon 2Ghz

	// Get the dlg instance
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

// Copy the samples so fast as possible and return
// The display is then performed from other threads
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

// Calcualte the time to wait after each sample
int lp_peackmeter::set_samplerate(int samplerate)
{
	QMutexLocker locker(&pv_mutex);

	// Verification
	if(samplerate == 0) {
		std::cerr << "lp_peackmeter::lp_peackmeter: samplerate is Null\n";
		return -1;
	}
	// Sampletime
	double d_tmp1 = 0, d_tmp2 = 0;
	// Set the sample time
	d_tmp1 = (double)samplerate;
	d_tmp2 = ((1 / (d_tmp1 * (double)pv_channels)) * 1000000);
	d_tmp2 = round(d_tmp2);
	pv_sample_time = (unsigned int)d_tmp2;
	std::cout << "Sample time: " << (unsigned int)d_tmp2 << std::endl;

	return 0;
}

// Protected

// The "fast" thread: no graphical displaying here
// only peack are stored and clip detection is 
// performed here
void lp_peackmeter::run()
{
	while(1){
		// Wait wakeUp
		pv_qwc.wait();
		pv_running = TRUE;
		pv_buf_end = FALSE;
		//float tmp;

		int i, j;
		for(i=0; i<(pv_buf_len/pv_channels); i++) {
			for(j=0; j<pv_channels; j++){

				// Set negative to positive
				if(pv_buffer[i*pv_channels+j] < 0){
					pv_buffer[i*pv_channels+j] = -pv_buffer[i*pv_channels+j];
				}

				pv_val[j] = pv_buffer[i*pv_channels+j];
		
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
					pv_start_clip(j);
				}
			}
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
		pv_running = FALSE;
	}
}

bool lp_peackmeter::get_clip(int channel)
{
	QMutexLocker locker(&pv_dlg_mutex);

	if(channel >= pv_channels){
		std::cerr << "lp_peackmeter::get_clip: channel value is to high\n";
		return -1;
	}

	if((pv_clip_is_timeout(channel) == FALSE)||(pv_clip_to_show[channel] == TRUE)) {
		pv_clip_to_show[channel] = FALSE;
		return TRUE;
	}else{
		return FALSE;
	}
}

// Pass the actuel peack value and reset
float lp_peackmeter::get_value(int channel)
{
	QMutexLocker locker(&pv_dlg_mutex);
	if(channel >= pv_channels){
		std::cerr << "lp_peackmeter::get_value: channel value is to high\n";
		return -1;
	}

	pv_val[channel] = pv_last_val[channel];
	pv_last_val[channel] = 0;
	return pv_val[channel];
}

// Private

void lp_peackmeter::pv_start_clip(int channel)
{
	if(channel >= pv_channels){
		std::cerr << "lp_peackmeter::pv_start_clip: channel value is to high\n";
		return;
	}

	gettimeofday(&pv_clip_start[channel], 0);
	pv_clip_to_show[channel] = TRUE;
}

bool lp_peackmeter::pv_clip_is_timeout(int channel)
{
	if(channel >= pv_channels){
		std::cerr << "lp_peackmeter::pv_clip_is_timeout: channel value is to high\n";
		return -1;
	}

	struct timeval curent_time;
	gettimeofday(&curent_time, 0);

	if((curent_time.tv_usec + (curent_time.tv_sec * 1000000)) - (pv_clip_start[channel].tv_usec + (pv_clip_start[channel].tv_sec * 1000000)) >= pv_clip_timeout) {
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
	int i;
	pv_peack_meter = pm;

	pv_channels = pm->get_channels();
	for(i=0; i<pv_channels; i++){
		pv_lb[i] = 0;
	}

	pv_bl = new QBoxLayout(this, QBoxLayout::LeftToRight, -1, 0, "lp_peackmeter_dlg layout");
	set_meter_size(3);
	// Widget size
	//setMinimumWidth(40 * pv_channels);
	//setMaximumWidth(40 * pv_channels);
	//pv_bl->setMargin(0);

	// Build widgets
	if(pv_build_widgets() < 0){
		std::cerr << "lp_peackmeter_dlg::lp_peackmeter_dlg: Warning! Unable to build Widgets\n";
		return;
	}

	if((pv_channels >= 1)||(pv_channels <= MAX_CHANNELS)){
		for(i=0; i<pv_channels; i++){
			draw_meter(pv_lb[i], 0, i);
			pv_clip[i] = TRUE;
		}
	}

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

void lp_peackmeter_dlg::set_meter_size(int size)
{
	if(size == 1){
		pv_meter_height = 100;
		//setMinimumHeight(250);
	}else if(size == 2){
		pv_meter_height = 200;
		//setMinimumHeight(250);
	}else if(size == 3){
		pv_meter_height = 300;
	}else{
		std::cerr << "lp_peackmeter_dlg::set_meter_size: size is not in range (aviable: 1, 2 or 3)\n";
		std::cerr << "\t-> Chosen value: 2 (medium)\n";
		pv_meter_height = 200;
	}
	// Resize the meter
	int i;
	for(i=0; i<pv_channels; i++){
		if(pv_lb[i] != 0){
			pv_lb[i]->setMinimumHeight(pv_meter_height + 30);
			pv_lb[i]->setMaximumHeight(pv_meter_height + 30);
		}
	}
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
		draw_meter(pv_lb[i], 0, i);
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
			draw_meter(pv_lb[i], t_event->val[i], i);
			pv_txt[i]->setNum(t_event->txt_val[i]);
		}
	}else{
		QWidget::customEvent(event);
	}
}

void lp_peackmeter_dlg::run()
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_dlg::run: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return;
	}	

	float val[MAX_CHANNELS], old_val[MAX_CHANNELS], peack_hold_val[MAX_CHANNELS];
	int i;
	struct timeval t0, t1;
	gettimeofday(&t0, 0);

	while(1){
		for(i=0; i<pv_channels; i++){
			if(pv_peack_meter->get_clip(i) == TRUE){
		//		std::cout << "###### CLIP ! #######\n";
				pv_clip[i] = TRUE;
			}else{
				pv_clip[i] = FALSE;
			}
			val[i] = pv_peack_meter->get_value(i);
	
			val[i] = pv_down_val(val[i], i);
			peack_hold_val[i] = pv_peack_hold(val[i], i);
	
		}
		// envois toutes les 1/25 sec (=0.04s --> 40ms --> 40000us)
		// Send a vu_event every 1/25 seconds (=0.04s --> 40ms --> 40000us)
		gettimeofday(&t1, 0);
		if(((t1.tv_usec + (t1.tv_sec * 1000000)) - (t0.tv_usec + (t0.tv_sec * 1000000))) >= 40000){
			//std::cout << "Intervale: " << t1.tv_usec - t0.tv_usec << std::endl;
			gettimeofday(&t0, 0);
			vu_event *event = new vu_event;

			// Traitement ICI
			for(i=0; i<pv_channels; i++){
				event->val[i] = pv_val_to_db(val[i]);
				event->txt_val[i] = pv_val_to_db(peack_hold_val[i]);
				old_val[i] = val[i];
			}
			QApplication::postEvent(this, event);
		}
		usleep(100);
	}

}

void lp_peackmeter_dlg::draw_meter(QPaintDevice *dev, int val, int channel)
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_dlg::draw_meter: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return;
	}	

	QPainter paint(dev);
	QRect dim = paint.window();
	// nettoyer
	paint.setPen(QPen(Qt::white));
	paint.setBrush(QBrush(Qt::white));
	paint.drawRect(10, pv_meter_height + 20, 10, -pv_meter_height);

	// clip
	if(pv_clip[channel] == TRUE){
		paint.setBrush(QBrush(Qt::red));
		paint.drawEllipse(10, 5, 10, 10);
		paint.setBrush(QBrush(Qt::green));
		pv_clip[channel] = FALSE;
	}else{
		paint.setBrush(QBrush(Qt::blue));
		paint.drawEllipse(10, 5, 10, 10);
	}
	// Bar
	paint.setPen(QPen(Qt::green));
	paint.setBrush(QBrush(Qt::green));
	paint.drawRect(10, pv_meter_height + 20, 10, -(pv_iec_scale((float)val)));
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
 
         //return (int)(def * 2.0f);
	return (int)(def * (float)pv_meter_height/100);
}

int lp_peackmeter_dlg::pv_val_to_db(float f_val)
{
	return (int)(20*((float)log10(f_val)));
}

float lp_peackmeter_dlg::pv_down_val(float act_val, int channel)
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_dlg::pv_down_val: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return -1;
	}	

	struct timeval actual_time;
	gettimeofday(&actual_time, 0);

	unsigned int actual_usec;	// time in usec
	actual_usec = actual_time.tv_usec + (actual_time.tv_sec * 1000000);

	unsigned int last_usec;		// last time in usec
	last_usec = pv_last_time[channel].tv_usec + (pv_last_time[channel].tv_sec * 1000000);

	unsigned int delta_t;
	delta_t = actual_usec - last_usec;

	float tmp_val;

	if(act_val < pv_last_val[channel]){
		tmp_val = pv_last_peack[channel] * ((pv_down_factor * pv_down_ref_time) / delta_t );

		if(tmp_val < pv_last_val[channel]){
			pv_last_val[channel] = tmp_val;
		}
		return pv_last_val[channel];
	}else{
		// reset timer
//		std::cout << "PEAK - Reset timer ###################################################\n";
		pv_last_peack[channel] = act_val;
		pv_last_val[channel] = act_val;
		gettimeofday(&pv_last_time[channel], 0);
	}

	return act_val;
}

float lp_peackmeter_dlg::pv_peack_hold(float act_val, int channel)
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_dlg::pv_peack_hold: pv_channels is not in range\n";
		std::cerr << "\tGiven value: " << pv_channels << " , range from 1 to " << MAX_CHANNELS << std::endl;
		return -1;
	}	

	struct timeval actual_time;
	gettimeofday(&actual_time, 0);

	unsigned int actual_usec;	// time in usec
	actual_usec = actual_time.tv_usec + (actual_time.tv_sec * 1000000);

	unsigned int last_usec;		// last time in usec
	last_usec = pv_last_hold_time[channel].tv_usec + (pv_last_hold_time[channel].tv_sec * 1000000);

	unsigned int delta_t;
	delta_t = actual_usec - last_usec;

	float tmp_val;

	if(delta_t >= pv_hold_ref_time){
		tmp_val = act_val;
		gettimeofday(&pv_last_hold_time[channel], 0);
		pv_last_hold_peack[channel] = act_val;
	//	std::cout << "HOLD (Reset): return " << tmp_val << std::endl;
	}else{
		tmp_val = pv_last_hold_peack[channel];
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
		pv_hbl[i] = new QBoxLayout(this, QBoxLayout::TopToBottom, 2, 2);
		pv_lb[i] = new QLabel(this);
		pv_lb[i]->setBackgroundColor(Qt::black);
		pv_lb[i]->setMinimumHeight(pv_meter_height + 30);
		pv_lb[i]->setMaximumHeight(pv_meter_height + 30);
		pv_lb[i]->setMinimumWidth(30);
		pv_lb[i]->setMaximumWidth(30);
		pv_hbl[i]->addWidget(pv_lb[i], -1, 0);
		// Text label
		pv_txt[i] = new QLabel(this);
		pv_txt[i]->setAlignment(Qt::AlignHCenter);
		pv_txt[i]->setText("0");
		//pv_txt[i]->setBackgroundColor(Qt::green);
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
