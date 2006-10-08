#ifndef M_WIDGET_H
#define M_WIDGET_H

#include "ui_lp_peackmeter_widget_ui.h"

#include <QPainter>
#include <QColor>
#include <QPaintEvent>
#include <QRect>
#include <QRadialGradient>
#include <QEvent>

class lp_event;

// This is the peackmeter's user interface
class lp_peackmeter_widget : public QWidget, Ui::lp_peackmeter_widget_ui
{
Q_OBJECT
public:
	lp_peackmeter_widget(QWidget *parent = 0);
	int set_range(int dBmin = 0, int dBmax = 0);
	int set_headroom(int h_dB);
	void set_val(int val_dB, bool clip, int txt_val);
private:
	QRect dev_rect;
	QColor green_zone_color;
	QColor orange_zone_color;
	QColor vu_color;
	int cur_val;//, max_val;
	void draw_vu();
	void paintEvent(QPaintEvent*);
	int pv_dBmax, pv_dBmin, pv_dB_offset;
	int pv_headroom;
	bool pv_clip;
};

#endif
