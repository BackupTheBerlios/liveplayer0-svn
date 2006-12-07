#include "lp_ladspa_lister.h"

#include <QtGui>
#include <QSortFilterProxyModel>

#include <lrdf.h>
#include <ladspa.h>

#include "../../lp_utils/lp_custom_types/lp_custom_types.h"

lp_ladspa_lister::lp_ladspa_lister(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	// Set headers
	QList<QVariant>list_headers;
	list_headers << tr("Category") << tr("Plugin") << tr("ID");
	// Must provide a empty valid Null pointer for column 4 (ladspa descriptor)
	LADSPA_Descriptor *descriptor = 0;
	lp_custom_types custom_type(descriptor);
	QVariant tmp_var;
	tmp_var.setValue(custom_type);
	list_headers << tmp_var;
	// Ok, get a new model
	lister_model = new lp_ladspa_lister_model(list_headers, this);

	set_rdfs_path("/usr/share/ladspa/rdf");
	set_plugins_path("/usr/lib/ladspa");
	list_plugins();

	// Sorting
	sorting_proxy = new QSortFilterProxyModel;
	sorting_proxy->setSourceModel(lister_model);
	ls_plugins->setSortingEnabled(true);
	// Set data model to TreeView
	ls_plugins->setModel(sorting_proxy);
	ls_plugins->header()->setSortIndicator(0, Qt::AscendingOrder);
	ls_plugins->header()->setSortIndicatorShown(true);
	ls_plugins->header()->setClickable(true);

	pv_ladspa_descriptor = 0;

	QItemSelectionModel *selection_model = ls_plugins->selectionModel();
	connect(selection_model, SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),
	 this, SLOT(display_info(const QItemSelection, const QItemSelection)));

	connect(pb_ok, SIGNAL(clicked()), this, SLOT(send_ok()));
	// FIXME Wrong !...
	connect(ls_plugins, SIGNAL(doubleClicked(const QModelIndex)),
	 this, SLOT(double_click_selection(const QModelIndex)));
	connect(pb_cancel, SIGNAL(clicked()), this, SLOT(close()));
}

const LADSPA_Descriptor *lp_ladspa_lister::get_ladspa_descriptor()
{
	return pv_ladspa_descriptor;
}

// FIXME Wrong !...
void lp_ladspa_lister::double_click_selection(const QModelIndex &item)
{
	const LADSPA_Descriptor *descriptor = 0;

	if((item.data().canConvert<lp_custom_types>()) == false){
		std::cerr << "Conv impossible !\n";
	}else{
		if(item.data().value<lp_custom_types>().get_ladspa_descriptor() != 0){
			descriptor = item.data().value<lp_custom_types>().get_ladspa_descriptor();
			pv_ladspa_descriptor = descriptor;
			emit signal_ok();
		}
	}
}

void lp_ladspa_lister::display_info(const QItemSelection & selected, const QItemSelection&)
{
	QModelIndexList items = selected.indexes();

	const LADSPA_Descriptor *descriptor = 0;

	if((items[3].data().canConvert<lp_custom_types>()) == false){
		std::cerr << "Conv impossible !\n";
	}else{
		if(items[3].data().value<lp_custom_types>().get_ladspa_descriptor() != 0){
			descriptor = items[3].data().value<lp_custom_types>().get_ladspa_descriptor();
			lb_plugname->setText(descriptor->Name);
			lb_maker->setText(descriptor->Maker);
			lb_copyright->setText(descriptor->Copyright);
			pv_ladspa_descriptor = descriptor;
		}
	}
}

void lp_ladspa_lister::send_ok()
{
	emit signal_ok();
}

void lp_ladspa_lister::list_plugins()
{
	int i, y;

	parse_rdfs();
	// scan plugins dir
	for(i=0; i<plugins_dir_list.size(); i++){
		QFileInfo file_info = plugins_dir_list.at(i);
		// Tmp var ok ? oui...
		lp_ladspa_plugin_describer plugin_describer(file_info.filePath());
		QStringList names;
		QList<unsigned long> UIDs;
		QList<QVariant> ladspa_descriptors;
		plugin_describer.get_plugins(&names, &UIDs, &ladspa_descriptors);
		for(y=0; y<UIDs.count(); y++){
			// Add the plugin to the model
			if((ladspa_descriptors[y].canConvert<lp_custom_types>()) == false){
				std::cerr << "lp_ladspa_lister::" << __FUNCTION__ <<": unable to convert QVariant to lp_custom_types\n";
			}else{
				lister_model->add_plugin(names[y], UIDs[y], ladspa_descriptors[y].value<lp_custom_types>().get_ladspa_descriptor());
			}
		}
	}
}

int lp_ladspa_lister::set_rdfs_path(QString path)
{
	rdfs_dir = path;
	if(!rdfs_dir.exists()){
		qstring_char qsc(path);
		char *msg_tmp = qsc.to_char();
		std::cerr << "lp_ladspa_lister::" << __FUNCTION__ <<": " << msg_tmp << " isn't a valid directory\n";
		free(msg_tmp);
		return -1;
	}else{
		rdfs_dir.setFilter(QDir::Files);
		rdfs_dir_list = rdfs_dir.entryInfoList();
	}
	return 0;
}

int lp_ladspa_lister::set_plugins_path(QString path)
{
	plugins_dir = path;
	if(!plugins_dir.exists()){
		qstring_char qsc(path);
		char *msg_tmp = qsc.to_char();
		std::cerr << "lp_ladspa_lister::" << __FUNCTION__ <<": " << msg_tmp << " isn't a valid directory\n";
		free(msg_tmp);
		return -1;
	}else{
		plugins_dir.setFilter(QDir::Files);
		plugins_dir_list = plugins_dir.entryInfoList();
	}
	return 0;
}

void lp_ladspa_lister::parse_rdfs()
{
	int i;

	// lrdf prefix
	QString prefix("file://");

	lrdf_init();

	// Parse lrdf files in given directory
	for(i = 0; i< rdfs_dir_list.size(); ++i){
		QFileInfo file_info = rdfs_dir_list.at(i);
		QString tmp_uri = prefix + file_info.filePath();
		// QString to char convertion...
		qstring_char qsc_uri(tmp_uri);
		char *tmp_c = qsc_uri.to_char();
		// parse one file
		if(lrdf_read_file(tmp_c)){
			free(tmp_c);
			exit(1);
		}
		free(tmp_c);
	}

	descend(LADSPA_BASE "Plugin", "");

	lrdf_cleanup();
}

void lp_ladspa_lister::descend(char *uri, char *base)
{
	lrdf_uris *uris;
	unsigned int i;
	int y;
	char *newbase;
	char *label;
	QString q_tmp;
	QStringList q_instaces;

	uris = lrdf_get_instances(uri);

	if(uris != NULL){
		for(i=0; i< uris->count; i++){
			//printf("Instances:\t%s/\t[%ld]\n", base, lrdf_get_uid(uris->items[i]));
			q_tmp = base;
			q_instaces = q_tmp.split("/");
			// Add bases (Categories)
			if(q_instaces.count() > 0){
				// Add rootItem base
				if(q_instaces.count() == 1){
					lister_model->add_base("", q_instaces[0]);
				}
				// Add child bases (Sub Categories)
				if(q_instaces.count() > 1){
					for(y=1; y<q_instaces.count(); y++){
						lister_model->add_base(q_instaces[y-1], q_instaces[y]);
						qstring_char qsc(q_instaces[y]);
						char *c_tmp = qsc.to_char();
						//std::cout << c_tmp << std::endl;
						free(c_tmp);
					}
				}
			}
			// Add plugin
			lister_model->add_plugin(q_instaces[q_instaces.count()-1], "", lrdf_get_uid(uris->items[i]));
		}
		lrdf_free_uris(uris);
	}

	uris = lrdf_get_subclasses(uri);

	if(uris != NULL){
		for(i=0; i< uris->count; i++){
			label = lrdf_get_label(uris->items[i]);
			newbase = (char*)malloc(strlen(base) + strlen(label) + 2);
			//printf("Subclasses:\tbase: %s\n", base);
			//printf("Subclasses:\tlabel: %s\n", label);
			sprintf(newbase, "%s/%s", base, label);
			//printf("%s\n", newbase);
			descend(uris->items[i], newbase);
			free(newbase);
			//printf("\n");
		}
		lrdf_free_uris(uris);
	}
}
