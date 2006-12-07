#include "lp_ladspa_plugin_describer.h"

#include "../lp_utils/lp_custom_types/lp_custom_types.h"

lp_ladspa_plugin_describer::lp_ladspa_plugin_describer(QString file_path)
{
	if(file_path.length() < 1){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": file_path is not set\n";
		return;
	}

	// Obtain a LADSPA Descriptor Function
	if((pv_obtain_desriptor_function(file_path)) < 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": failed to obtain LADSPA_Descriptor_Function\n";
		return;
	}

	// Ok, store path
	pv_plugin_path = file_path;
} 

// Destructor
lp_ladspa_plugin_describer::~lp_ladspa_plugin_describer()
{

}

QString lp_ladspa_plugin_describer::get_plugin_name(unsigned long index)
{
	QString tmp_name;

	if(pv_lib_handler == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": pv_lib_handler is Null\n";
		return 0;
	}
	if(pv_LDF == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": pv_LDF is Null\n";
		return 0;
	}

	const LADSPA_Descriptor *descriptor = 0;
	// give plugin name if aviable
	if((descriptor = pv_LDF(index)) != 0){
		tmp_name = descriptor->Name;
	}else{
		//free(descriptor);
		return 0;
	}

	//free(descriptor);
	return tmp_name;
}

void lp_ladspa_plugin_describer::get_plugins(QStringList *names,
				QList<unsigned long> *UIDs, 
				QList<QVariant> *ladspa_descriptors)
{
	QString tmp_name;
	unsigned long index = 0;

	if(pv_lib_handler == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": pv_lib_handler is Null\n";
		return;
	}
	if(pv_LDF == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": pv_LDF is Null\n";
		return;
	}

	const LADSPA_Descriptor *descriptor = 0;
	// Search plugin in library
	while((descriptor = pv_LDF(index)) != 0){
		*names << descriptor->Name;
		*UIDs << descriptor->UniqueID;
		lp_custom_types custom_tmp(descriptor);
		QVariant tmp_var;
		tmp_var.setValue(custom_tmp);
		*ladspa_descriptors << tmp_var;
		index++;
	}
}

QString lp_ladspa_plugin_describer::find_plugin_name(unsigned long UID)
{
	QString tmp_name;
	unsigned long index = 0;

	if(pv_lib_handler == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": pv_lib_handler is Null\n";
		return 0;
	}
	if(pv_LDF == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": pv_LDF is Null\n";
		return 0;
	}

	const LADSPA_Descriptor *descriptor = 0;
	// Search plugin in library
	while((descriptor = pv_LDF(index)) != 0){
		if(descriptor->UniqueID == UID){
			return descriptor->Name;
		}
		index++;
	}

	//free(descriptor);
	return 0;
}

/* Private functions */
int lp_ladspa_plugin_describer::pv_obtain_desriptor_function(QString file_path)
{
	pv_LDF = 0;
	char *dl_err_output = 0;
	char *tmp_path;

	if(file_path.length() < 1){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": file_path is not set\n";
		return -1;
	}
	qstring_char qsc1(file_path);
	tmp_path = qsc1.to_char();

	// Try to open file
	pv_lib_handler = dlopen(tmp_path, RTLD_LAZY);
	if(pv_lib_handler == 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": unable to load" << tmp_path << std::endl;
		std::cerr << "\tError was: " << dlerror() << std::endl;
		free(tmp_path);
		return -1;
	}

	// It's a shared library - continue
	pv_LDF = (LADSPA_Descriptor_Function) dlsym (pv_lib_handler, "ladspa_descriptor");

	// Tests dlerror() output and pv_LDF
	if((dl_err_output = dlerror()) != 0){
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ": unable to load" << tmp_path << std::endl;
		std::cerr << "\tError was: " << dl_err_output << std::endl;
		free(tmp_path);
		return -1;
	}

	// Tests if this is a LADSPA library
	if(!pv_LDF){
		dlclose(pv_lib_handler);
		std::cerr << "lp_ladspa_plugin_describer::" << __FUNCTION__ << ":" << tmp_path << " is not a LADSPA library " << std::endl;
		free(tmp_path);
		return -1;
	}

	// Ok - LADSPA library found
	free(tmp_path);
	return 0;
}
