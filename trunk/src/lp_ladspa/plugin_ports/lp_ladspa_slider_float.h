#ifndef LP_LADSPA_SLIDER_FLOAT
#define LP_LADSPA_SLIDER_FLOAT

#include <ladspa.h>
#include <ui_lp_ladspa_slider_float.h>
#include <qwt_scale_engine.h>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

class lp_ladspa_slider_float : public QWidget, Ui::lp_ladspa_slider_float
{
Q_OBJECT
public:
	lp_ladspa_slider_float(QWidget *parent = 0);
	~lp_ladspa_slider_float();
	int init(const LADSPA_Descriptor *descriptor
		, unsigned long port
		, int samplerate);
	// To use this slider for other pupose than LADSPA plugin
	// If display_db is true, linear display is set, but the return
	// of get_val (below) caculates the "raw" val.
	int init(float low_val, float high_val, float def_val, bool display_db);
	void set_name(QString name);
	float get_val();
	void set_def_val();
	void set_def_val(double val);

public slots:
	void pos_def_val();
	void set_sb_val(double val);
	void set_sl_val(double val);
	void emit_changed(double val);

signals:
	void val_changed(float val);

private:
	double pv_low_val;
	double pv_default_low_val;
	double pv_high_val;
	double pv_default_high_val;
	double pv_def_val;
	double pv_samplerate;
	double pv_last_val;
	bool pv_is_log;
	bool pv_display_db;
	QwtLog10ScaleEngine *log10_scale_engine;
	QMutex mutex;	// sl_val is in concurence with get_val()
};

#endif
