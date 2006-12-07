#ifndef LP_LADSPA_MANAGER_H
#define LP_LADSPA_MANAGER_H

#include "ui_lp_ladspa_manager.h"
#include "lp_ladspa_manager_item.h"
#include "lp_ladspa_plugin.h"

#include <QGridLayout>

class lp_ladspa_manager : public QWidget, Ui::lp_ladspa_manager
{
Q_OBJECT
public:
	/// This is the class to instanciate in main thread
	lp_ladspa_manager(QWidget *parent=0);
	~lp_ladspa_manager();
	/// Call this init function. manager_items_count is the number plugins you
	/// want to call at same time
	int init(int manager_items_count, int channels, int buf_size, int samplerate);
	/// Runs a buffer of data - if plugin is unactiv, returns 0. if err, returns -1
	/// else, it returns the number of element processed (= tot_len)
	int run_interlaced_buffer(float *buffer, int tot_len);

private:
	lp_ladspa_manager_item *pv_manager_items;	// Array of manager_items
	lp_ladspa_plugin *pv_plugins;			// Array of plugins
	int pv_manager_items_count;
	QGridLayout *pv_layout;
	int pv_channels;
	int pv_buf_size;
	int pv_samplerate;
};

#endif
