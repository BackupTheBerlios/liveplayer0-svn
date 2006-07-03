#include <qpainter.h>
#include <qpixmap.h>
#include <qpaintdevice.h>
#include <qlayout.h>
#include <iostream>

#include <qevent.h>
#include <qapplication.h>

#include "vu_meter.h"

// Public functions

vu_meter::vu_meter(QWidget *parent, const char *name) : QWidget(parent, name)
{
	pv_bl = new QBoxLayout(this, QBoxLayout::LeftToRight, 5, 10);
	//pix = new QPixmap(100, 100);

	pv_channels = 2;

	// Build widgets
	if(pv_build_widgets() < 0){
		std::cerr << "m_widget::m_widget: Warning! pv_channels is not in range\n";
		return;
	}

	// Widget size
	//setMinimumHeight(250);
	setMinimumWidth(50 * pv_channels);

	// Default range
	//pv_range_low = -1.0;
	pv_range_low = 0.001;	// Mettre -50dBu
	pv_range_high = 1.0;
	if(pv_set_range_factor() < 0){
		return;
	}

	// set pv_buffer to Null

	// Default clip hold time (us)
	pv_clip_hold_time = 200000;

	int i;
	if((pv_channels >= 1)||(pv_channels <= MAX_CHANNELS)){
		for(i=0; i<pv_channels; i++){
			draw_vu(pv_lb[i], 0);
		}
	}
	// Start the thread
	start();
}

vu_meter::~vu_meter()
{
	if(pv_buffer != 0){
		delete[] pv_buffer;
	}
}

int vu_meter::set_value(float *buffer ,int buffer_time)
{
	if(pv_buffer == 0) {
		std::cout << "vu_meter::set_value: pv_buffer is Null !\n";
		return -1;
	}

	int i;

	QMutexLocker locker(&pv_mutex);

	for(i=0; i<pv_buf_size; i++){
		pv_buffer[i] = buffer[i];
	}
	pv_buffer_time = buffer_time;
/*	if(val < 0) {
		pv_val = -val;
	}else{
		pv_val = val;
	}
*/	//std::cout << "Set_value: pv_val: " << pv_val << "\n";
	pv_qwc.wakeOne();

	return 0;
}

int vu_meter::alloc_mem(int buf_size)
{
	if(buf_size <= 0){
		std::cerr << "vu_meter::alloc_mem: buf_size is in bad value!\n";
		return -1;
	}

	pv_buf_size = buf_size;
	pv_buffer = new float[pv_buf_size];

	if(pv_buffer == 0) {
		std::cerr << "vu_meter::alloc_mem: cannot allocate memory for pv_buffer\n";
		pv_buffer = 0;
		return -1;
	}

	return 0;
}

// Protected functions

void vu_meter::paintEvent(QPaintEvent*)
{
	// Verification
	if((pv_channels < 1)||(pv_channels > MAX_CHANNELS)) {
		std::cerr << "m_widget::pv_build_widgets: pv_channels is not in range\n";
		return;
	}

	// Draw
	int i;
	for(i=0; i<pv_channels; i++){
		draw_vu(pv_lb[i], 0);
	}
}

void vu_meter::draw_vu(QPaintDevice *dev, int val)
{
	//std::cout << "Event, val = :" << val << std::endl;
	QPainter paint(dev);
	QRect dim = paint.window();
	// nettoyer
	paint.setPen(QPen(Qt::white));
	paint.setBrush(QBrush(Qt::white));
	paint.drawRect(10, 120, 10, -100);

	paint.setPen(QPen(Qt::blue));
	paint.setBrush(QBrush(Qt::blue));
	// clip
//	if(pv_clip == TRUE) {
//		paint.setBrush(QBrush(Qt::red));
//	}else{
//		paint.setBrush(QBrush(Qt::blue));
//	}
	if(pv_clip == TRUE){
		paint.setBrush(QBrush(Qt::red));
		paint.drawEllipse(10, 5, 10, 10);
		paint.setBrush(QBrush(Qt::blue));
	}else{
		paint.setBrush(QBrush(Qt::blue));
		paint.drawEllipse(10, 5, 10, 10);
	}
/*	if(val < -100) {
		val = -100;
	}
	if(val > 0) { val = 0; }
*/	std::cout << "Val: " << val << "\t";
	std::cout << "Draw " << -(val+100) << std::endl;
	paint.drawRect(10, 120, 10, -(val+100));
	// Text label
}

void vu_meter::run()
{
	// Timer values for event send
	struct timeval t0, t1;
	gettimeofday(&t0, 0);
	// Timer values for Clip hold
	struct timeval clip_t0, clip_t1;
	// Wait time
	int wait_time;

	while(1){
		// Wait wakeUp
		pv_qwc.wait();

		// Calculate wait time
		wait_time = (int)(pv_buffer_time / pv_buf_size);

		int i;
		for(i=0; i<pv_buf_size; i++) {
			pv_val = pv_buffer[i];

			// Set 0.0 to range_low
			if(pv_val <= pv_range_low) {
				pv_val = pv_range_low;
			}
	
			// Detect Clip
			if(pv_val >= pv_range_high) {
				//std::cout << "Clip !\n";
				pv_clip = TRUE;
				pv_val = pv_range_high;
				// start hold time
				gettimeofday(&clip_t0, 0);
				// Send a vu_event now (to show the clip as soon as possible)
				vu_event *event = new vu_event;
				event->val = pv_val_to_db(pv_val);
				QApplication::postEvent(this, event);
			}else{
				// Get the clip hold time
				gettimeofday(&clip_t1, 0);
				if(((clip_t1.tv_usec + (clip_t1.tv_sec * 1000000)) - (clip_t0.tv_usec + (clip_t0.tv_sec * 1000000))) >= pv_clip_hold_time){
					pv_clip = FALSE;
				}else{
					pv_clip = TRUE;
				}
			}
	
			// envois toutes les 1/25 sec (=0.04s --> 40ms --> 40000us)
			// Send a vu_event every 1/25 seconds (=0.04s --> 40ms --> 40000us)
			gettimeofday(&t1, 0);
			if(((t1.tv_usec + (t1.tv_sec * 1000000)) - (t0.tv_usec + (t0.tv_sec * 1000000))) >= 40000){
				//std::cout << "Intervale: " << t1.tv_usec - t0.tv_usec << std::endl;
				gettimeofday(&t0, 0);
				vu_event *event = new vu_event;
	
				// Traitement ICI
	
				// Tests
				//std::cout << "Niveau: " << pv_val_to_db(pv_val) << std::endl;
	
				//event->val = (int)(pv_val * pv_range_factor);
				event->val = pv_val_to_db(pv_val);
				QApplication::postEvent(this, event);
			}
			usleep(wait_time);
		}
	}
}

void vu_meter::customEvent(QCustomEvent *event)
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "m_widget::pv_build_widgets: pv_channels is not in range\n";
		return;
	}

	if((int)event->type() == 8000){
		vu_event *t_event = (vu_event*)event;
		// Draw
		int i;
		for(i=0; i<pv_channels; i++){
			draw_vu(pv_lb[i], t_event->val);
			pv_txt[i]->setNum(t_event->val);
		}
	}else{
		QWidget::customEvent(event);
	}
}



// Private functions

int vu_meter::pv_val_to_db(float f_val)
{
	return (int)(20*((float)log10(f_val)));
}

// Build the widgets
int vu_meter::pv_build_widgets()
{
	// Verification
	if((pv_channels < 1)||(pv_channels >= MAX_CHANNELS)) {
		std::cerr << "m_widget::pv_build_widgets: pv_channels is not in range\n";
		return -1;
	}

	// Build
	int i;
	for(i=0; i<pv_channels; i++){
		pv_hbl[i] = new QBoxLayout(this, QBoxLayout::TopToBottom, 5, 10);
		pv_lb[i] = new QLabel(this);
		pv_lb[i]->setBackgroundColor(Qt::green);
		pv_lb[i]->setMinimumHeight(130);
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

int vu_meter::pv_set_range_factor()
{
	// Normal I think audio floats are set between -1.0 and 1.0
	// If this is the case, I take a factor to 100
	if((pv_range_low == 0.0)&&(pv_range_high == 1.0)) {
		pv_range_factor = 100;
	}else{
		std::cerr << "vu_meter::pv_set_range_factor: unable to set range factor\n";
	//	return -1;
	}
	return 0;
}

vu_event::vu_event() : QCustomEvent(8000)
{
}
