#include "lp_peackmeter_widget.h"
#include <iostream>

#include <QColorDialog>

lp_peackmeter_widget::lp_peackmeter_widget(QWidget *parent) 
	: QWidget(parent)
{
	// Ne pas oublier !
	setupUi(this);

	// Connaitre maximum slider
	set_range( -50, 0 );
	set_headroom(6);
	cur_val = 0;

	// Initial colors
	green_zone_color.setRgb(0, 255, 0, 255);
	orange_zone_color.setRgb(255, 220, 0, 255);
	vu_color.setRgb(0, 0, 255, 180);

	// debug...
	std::cout << "m_widget: appel constructeur - fini\n";
}

void lp_peackmeter_widget::set_val(int val_dB, bool clip, int txt_val)
{
	//std::cout << "Val: " << val << std::endl;
	cur_val = val_dB + pv_dB_offset;
	pv_clip = clip;

	lb_val->setNum(txt_val);
	update();
}

int lp_peackmeter_widget::set_range(int dBmin, int dBmax)
{
	int range = 0;

	if(dBmin >= dBmax){
		std::cerr << "m_widget::__FUNC__: dBmin cannot be >= dBmax\n";
		return -1;
	}

	// If dBmin is negative, we move the range to positive range
	if(dBmin < 0){
		range = dBmax - dBmin;
		dBmax = dBmax + range;
		dBmin = dBmin + range;
		pv_dB_offset = range;
	}

	pv_dBmax = dBmax;
	pv_dBmin = dBmin;

	std::cout << "Range: " << dBmin << " to " << dBmax << std::endl;
	return 0;
}

int lp_peackmeter_widget::set_headroom(int h_dB)
{
	if(h_dB >= pv_dBmax){
		std::cerr << "m_widget::__FUNC__: the headroom cannot be >= dBmax\n";
		return -1;
	}

	pv_headroom = h_dB;

	return 0;
}

void lp_peackmeter_widget::draw_vu()
{
	QPainter p(this);
	int factor, cur_h;
	int green_val, orange_val;
	//bool clip = false;

	p.setPen(Qt::NoPen);

	// Dimentions
	dev_rect = lb_vu->frameGeometry();

	// Facteur...
	factor = (dev_rect.height() * 1000) / pv_dBmax;

	// Valeur: clip
	if(cur_val >= pv_dBmax) {
		//clip = true;
		cur_val = pv_dBmax;
	}

	// Dessiner le fond
	// Dessiner zone verte
	green_val = pv_dBmax - pv_headroom;
	orange_val = pv_dBmax - green_val;
	cur_h = (green_val * factor) / 1000;
	// Decalage du top
	dev_rect.setTop( dev_rect.top() + dev_rect.height() );
	// Dessiner zone verte
	dev_rect.setHeight(-cur_h);
	p.setBrush(green_zone_color);
	p.drawRect(dev_rect);
	// Decalage du top - zone orange
	dev_rect.setTop( dev_rect.top() - cur_h );
	// hauteur orange
	cur_h = (orange_val * factor) / 1000;
	dev_rect.setHeight(-cur_h);
	p.setBrush(orange_zone_color);
	p.drawRect(dev_rect);


	// Valeur:
	cur_h = (cur_val * factor) / 1000;
	// Decalage du top
	dev_rect = lb_vu->frameGeometry();
	dev_rect.setTop( dev_rect.top() + dev_rect.height() );
	// largeur
	dev_rect.setWidth(dev_rect.width() / 2);
	dev_rect.translate(dev_rect.width() / 2, 0);
	// Dessiner vu
	dev_rect.setHeight(-cur_h);
	p.setBrush(vu_color);
	p.drawRect(dev_rect);

	// Clip
	dev_rect = lb_clip->frameGeometry();
	if(pv_clip == true){
		p.setBrush(Qt::red);
	}else{
		p.setBrush(Qt::green);
	}
	p.drawEllipse(dev_rect);
}

void lp_peackmeter_widget::paintEvent(QPaintEvent*)
{
	draw_vu();
}
