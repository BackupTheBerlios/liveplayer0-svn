#ifndef LP_LADSPA_LISTER_H
#define LP_LADSPA_LISTER_H

#include <QDir>
#include <QFileInfoList>
#include <QList>

#include <ladspa.h>

#include "lp_ladspa_lister_model.h"
#include "ui_lp_ladspa_lister.h"
#include "../lp_ladspa_plugin_describer.h"

class QTreeView;
class QSortFilterProxyModel;

class lp_ladspa_lister : public QWidget, Ui::lp_ladspa_lister
{
Q_OBJECT
public:
	lp_ladspa_lister(QWidget * parent = 0);
	const LADSPA_Descriptor *get_ladspa_descriptor();

public slots:
	void display_info(const QItemSelection & selected, const QItemSelection&);
	void double_click_selection(const QModelIndex &item);

public slots:
	void list_plugins();
	void send_ok();

signals:
	void signal_ok();

private:
	void parse_rdfs();
	void descend(char *uri, char *base);

	// Rdfs path
	QDir rdfs_dir;
	QFileInfoList rdfs_dir_list;
	int set_rdfs_path(QString path);
	// Plugins path
	QDir plugins_dir;
	QFileInfoList plugins_dir_list;
	int set_plugins_path(QString path);

	lp_ladspa_lister_model *lister_model;
	QSortFilterProxyModel *sorting_proxy;

	const LADSPA_Descriptor *pv_ladspa_descriptor;
};

#endif
