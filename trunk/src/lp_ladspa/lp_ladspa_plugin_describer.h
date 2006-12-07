#ifndef LP_LADSPA_PLUGIN_DESCRIBER_H
#define LP_LADSPA_PLUGIN_DESCRIBER_H

#include <iostream>

#include <ladspa.h>

#include "../lp_utils/qstring_char.h"

// C libs
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include <QString>
#include <QStringList>
#include <QVariant>

class lp_ladspa_plugin_describer
{
public:
	lp_ladspa_plugin_describer(QString file_path);
	~lp_ladspa_plugin_describer();
	QString get_plugin_name(unsigned long index);
	void get_plugins(QStringList *names, 
			QList<unsigned long> *UIDs,
			QList<QVariant> *ladspa_descriptors = 0);
	QString find_plugin_name(unsigned long UID);
//	unsigned long get_plugin_ID(unsigned long index);
//	const LADSPA_Descriptor *get_plugin_descriptor(unsigned long plugin_ID);

private:
	// The LADSPA_Descriptor_Function passed to plugin to obtain a descriptor
	LADSPA_Descriptor_Function pv_LDF;
	// the library handler (returned by dlopen() )
	void *pv_lib_handler;

	QString pv_plugin_path;

	// obtain a LADSPA_Descriptor_Function: if not ok, return -1
	int pv_obtain_desriptor_function(QString file_path);
};


#endif

