#ifndef LP_LADSPA_MANAGER_ITEM_H
#define LP_LADSPA_MANAGER_ITEM_H

#include "ui_lp_ladspa_manager_item.h"
#include "lp_ladspa_plugin.h"
#include "plugin_lister/lp_ladspa_lister.h"
#include <QWidget>

class lp_ladspa_manager_item : public QWidget, Ui::lp_ladspa_manager_item
{
Q_OBJECT
public:
	lp_ladspa_manager_item(QWidget *parent = 0);
	~lp_ladspa_manager_item();
	int init(int channels, int buf_size, int samplerate);
	/// Runs a buffer of data - if plugin is unactiv, returns 0. if err, returns -1
	/// else, it returns the number of element processed (= tot_len)
	int run_interlaced_buffer(float *buffer, int tot_len);

private slots:
	void remove_plugin();
	void select_plugin();
	void init_plugin();
	void edit_plugin();
	void activate_plugin();

private:
	lp_ladspa_plugin *pv_plugin;
	lp_ladspa_lister *pv_lister;
	int pv_channels;
	int pv_buf_size;
	int pv_samplerate;
};

#endif
