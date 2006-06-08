#include "LP_ladspa_plugin.h"
// QT libs
#include <qvariant.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qslider.h>
//#include <qdial.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>

// Contructor
LP_ladspa_plugin::LP_ladspa_plugin(char *file_path)
{
	// Test vars
	if(file_path == 0){
		std::cerr << "LP_ladspa_plugin::LP_ladspa_plugin: file_path is NULL\n";
		this->~LP_ladspa_plugin();
	}
	if(strlen(file_path)<1){
		std::cerr << "LP_ladspa_plugin::LP_ladspa_plugin: file_path is not set\n";
		this->~LP_ladspa_plugin();
	}
	if(strlen(file_path)>NAME_MAX){
		std::cerr << "LP_ladspa_plugin::LP_ladspa_plugin: file_path is too long\n";
		this->~LP_ladspa_plugin();
	}

	// Obtain a LADSPA_Descriptor_Function
	if((pv_obtain_desriptor_function(file_path)) < 0){
		std::cerr << "LP_ladspa_plugin::LP_ladspa_plugin: failed to obtain a LADSPA_Descriptor_Function\n";
		this->~LP_ladspa_plugin();
	}else{ // The given path is a LADSPA plugin lib, store the path
		pv_plugin_path = new char[NAME_MAX];
		strcpy(pv_plugin_path, file_path);
	}

	// LADSPA descriptor
	pv_LD = 0;

	// Plugin's UI
	pv_ui = 0;

	// Set the samplerate to 44100 - TEMPORAIRE !!
	pv_srate = 44100;

	// Set ports ctl value to a default
	unsigned long ul;
	for(ul=0; ul<LP_MAX_PORT; ul++){
		pv_in_ctl_value[ul] = 1;
		pv_out_ctl_value[ul] = 0;
	}

	// Set port buffers to Null
	for(ul=0; ul<LP_MAX_PORT; ul++){
		pv_in_fake_buffer[ul] = 0;
		pv_out_fake_buffer[ul] = 0;
//		pv_in_buffer2[ul] = 0;
//		pv_out_buffer2[ul] = 0;
	}

	// Set the ready state to FALSE
	pv_plugin_ready = FALSE;

	// Set the active state to FALSE as default
	pv_active = FALSE;

	// Set run mode to 0
	pv_run_mode = 0;

	// Set handles to 0
	pv_ladspa_handle = 0;
	pv_ladspa_handle2 = 0;
}

// Destructor
LP_ladspa_plugin::~LP_ladspa_plugin()
{
	// If private var are set, delete them
	if(pv_lib_handler != 0){
		dlclose(pv_lib_handler);
	}
	if(pv_plugin_path != 0){
		delete pv_plugin_path;
	}

	if(pv_LD != 0){
		pv_LD = 0;
	}

	delete pv_ui;
	pv_ui = 0;

	// free the buffers
	unsigned long ul;
	for(ul=0; ul<LP_MAX_PORT; ul++){
		if(pv_in_fake_buffer[ul] != 0){
			delete[] pv_in_fake_buffer[ul];
		}
		// free the buffer
		if(pv_out_fake_buffer[ul] != 0){
			delete[] pv_out_fake_buffer[ul];
		}
	}
	// TODO Delete pv_LR_buffer and FINI !
}


int LP_ladspa_plugin::list()
{
	pv_list_plugins();
	return 0;
}

// Init the plugin and store values
int LP_ladspa_plugin::init_plugin(/*QWidget* parent, const char* name,*/ unsigned long unique_ID, int channels, int sample_rate, int buf_len)
{
//	unsigned long ul;
	unsigned long index = 0, count = 0, port = 0; // count: count plugins found: must be 1
	char *tmp = new char[NAME_MAX];
	LADSPA_Data low_bound = 0, high_bound = 0, def_val = 0;
	// Utilities class
	LP_ladspa_utils utils;

	// test variables
	if(pv_lib_handler == 0){
		std::cerr << "LP_ladspa_plugin::init_plugin: argument pv_lib_handler can't be NULL\n";
		return -1;
	}
	if(pv_LDF == 0){
		std::cerr << "LP_ladspa_plugin::init_plugin: argument Descriptor_Function can't be NULL\n";
		return -1;
	}
	if(channels < 1){
		std::cerr << "LP_ladspa_plugin::init_plugin: extern given channels cannot be < 1\n";
		return -1;
	}
	pv_nb_channels = channels;
	if(sample_rate == 0){
		std::cerr << "LP_ladspa_plugin::init_plugin: sample_rate is not set\n";
		return -1;
	}
	pv_srate = (unsigned int)sample_rate;
	if(buf_len == 0){
		std::cerr << "LP_ladspa_plugin::init_plugin: buf_len is not set\n";
		return -1;
	}
	pv_buf_len = buf_len;

	// Audio ports cout
	pv_nb_audio_in = 0;
	pv_nb_audio_out = 0;

	// Search plugin in given file, if non, return error
	const LADSPA_Descriptor *tmp_LD = 0;
	pv_LD = 0;
	LADSPA_PortRangeHintDescriptor hint_desc = 0;
	// give plugin name if aviable
	while((tmp_LD = pv_LDF(index)) != 0){
		// If the plugin is found
		if(tmp_LD->UniqueID == unique_ID){
			pv_LD = tmp_LD;
//			pv_ui = new LP_ladspa_plugin_dlg(0, name, 0, this);

//			pv_ui->set_plug_name(strdup(pv_LD->Name));
			pv_plug_name = 	(strdup(pv_LD->Name));
			pv_label     =	(strdup(pv_LD->Label));
			pv_maker     =	(strdup(pv_LD->Maker));
			pv_copyright =	(strdup(pv_LD->Copyright));

			// DEBUG
			std::cout << "Plugin: " << pv_plug_name << " , ID: " << pv_LD->UniqueID << std::endl;
			std::cout << pv_label << std::endl;
			std::cout << pv_maker << std::endl;
			std::cout << pv_copyright << std::endl;

			// Must run Realtime ?
			if(LADSPA_IS_REALTIME(pv_LD->Properties) ){
				pv_must_rt = TRUE;
			}else{
				pv_must_rt = FALSE;
			}

			// Hard RT capable ?
			if(LADSPA_IS_HARD_RT_CAPABLE (pv_LD->Properties) ){
				pv_hard_rt = TRUE;
			}else{
				pv_hard_rt = FALSE;
			}

			// Inplace broken ?
			if(LADSPA_IS_INPLACE_BROKEN (pv_LD->Properties) ){
				pv_inplace_broken = TRUE;
			}else{
				pv_inplace_broken = FALSE;
			}

			// Activate function aviable ?
			if(pv_LD->activate != 0){
				pv_has_activate = TRUE;
			}else{
				pv_has_activate = FALSE;
			}

			// Plugin's ports
			for(port = 0; port < pv_LD->PortCount; port++){
				// If the port is a control input
				if((LADSPA_IS_PORT_INPUT (pv_LD->PortDescriptors[port])) && (LADSPA_IS_PORT_CONTROL(pv_LD->PortDescriptors[port]))) {
					// DEBUG	
					std::cout << "\tPort: " << pv_LD->PortNames[port] << std::endl;

					pv_ports[port].name = strdup(pv_LD->PortNames[port]);
					//pv_ports[port].name = pv_LD->PortNames[port];
					// Port hints
					hint_desc = pv_LD->PortRangeHints[port].HintDescriptor;
					if(LADSPA_IS_HINT_TOGGLED( hint_desc)){
//						pv_ui->add_port_ctl( port, strdup(pv_LD->PortNames[port]), TRUE, FALSE, FALSE);
						pv_ports[port].is_in_ctl = TRUE;
						pv_ports[port].is_toggle = TRUE;
					}else{
						if(LADSPA_IS_HINT_INTEGER (hint_desc)){
//							pv_ui->add_port_ctl(port, strdup(pv_LD->PortNames[port]), FALSE, TRUE, TRUE);
							pv_ports[port].is_in_ctl = TRUE;
							pv_ports[port].is_toggle = FALSE;
							pv_ports[port].is_int = TRUE;
						}else{
//							pv_ui->add_port_ctl(port, strdup(pv_LD->PortNames[port]), FALSE, FALSE, FALSE);
							pv_ports[port].is_in_ctl = TRUE;
							pv_ports[port].is_toggle = FALSE;
							pv_ports[port].is_int = FALSE;
						}
					}

					// Set the port range if aviable
					if(LADSPA_IS_HINT_BOUNDED_BELOW (hint_desc)){
						low_bound = pv_LD->PortRangeHints[port].LowerBound;
						if(LADSPA_IS_HINT_SAMPLE_RATE (hint_desc)){
							low_bound = low_bound * pv_srate;
						}
						// QQ TESTS
					}else{
						// Set a default min
						low_bound = 0.001;
					}
					if(LADSPA_IS_HINT_BOUNDED_ABOVE (hint_desc)){
						high_bound = pv_LD->PortRangeHints[port].UpperBound;
						if(LADSPA_IS_HINT_SAMPLE_RATE (hint_desc)){
							high_bound = high_bound * pv_srate;
						}
					}else{
						// Set a default max
						high_bound = 10;
					}
					// Some plugins gives a range from 0 to 0, set a default range...
					if(low_bound == 0){ low_bound = 0.001; }
					if(high_bound == 0) { high_bound = 10; }

					pv_ports[port].low_bound = low_bound;
					pv_ports[port].high_bound = high_bound;

					if(LADSPA_IS_HINT_LOGARITHMIC (hint_desc)){
						std::cout << "LOGARITHMIQUE\n";
//						pv_ui->set_range( port, low_bound, high_bound, TRUE);
						pv_ports[port].is_logarithmic = TRUE;
					}else{
//						pv_ui->set_range( port, low_bound, high_bound, FALSE);
						pv_ports[port].is_logarithmic = FALSE;
					}
					// Set the default port's value if aviable
					if(LADSPA_IS_HINT_HAS_DEFAULT (hint_desc)){
						if(LADSPA_IS_HINT_DEFAULT_MINIMUM (hint_desc)){
							def_val = low_bound;
							std::cout << "LOW\n";
						}
						if(LADSPA_IS_HINT_DEFAULT_MAXIMUM (hint_desc)){
							def_val = high_bound;
							std::cout << "HIGH\n";
						}
						if(LADSPA_IS_HINT_DEFAULT_LOW (hint_desc)){
							if(LADSPA_IS_HINT_LOGARITHMIC (hint_desc)){
								def_val = exp(log(low_bound)*0.75 + log(high_bound)*0.25);
								std::cout << "LOW LOG\n";
							}else{
								def_val = (low_bound*0.75 + high_bound*0.25);
								std::cout << "LOW\n";
							}
						}
						if(LADSPA_IS_HINT_DEFAULT_MIDDLE (hint_desc)){
							if(LADSPA_IS_HINT_LOGARITHMIC (hint_desc)){
								def_val = exp(log(low_bound)*0.5 + log(high_bound)*0.5);
								std::cout << "MIDDLE LOG\n";
							}else{
								def_val = (low_bound*0.5 + high_bound*0.5);
								std::cout << "MIDDLE\n";
							}
						}
						if(LADSPA_IS_HINT_DEFAULT_HIGH (hint_desc)){
							if(LADSPA_IS_HINT_LOGARITHMIC (hint_desc)){
								def_val = exp(log(low_bound)*0.25 + log(high_bound)*0.75);
								std::cout << "HIGH LOG\n";
							}else{
								def_val = (low_bound*0.25 + high_bound*0.75);
								std::cout << "HIGH\n";
							}
						}
						if(LADSPA_IS_HINT_DEFAULT_0 (hint_desc)){
							def_val = 0;
							std::cout << "0\n";
						}
						if(LADSPA_IS_HINT_DEFAULT_1 (hint_desc)){
							def_val = 1;
							std::cout << "1\n";
						}
						if(LADSPA_IS_HINT_DEFAULT_100 (hint_desc)){
							def_val = 100;
							std::cout << "100\n";
						}
						if(LADSPA_IS_HINT_DEFAULT_440 (hint_desc)){
							def_val = 440;
							std::cout << "440\n";
						}
//						pv_ui->set_default_value(port, def_val);
						pv_ports[port].default_value = def_val;
					}
				}// Plugin's ctl ports
				// Output controls
				if((LADSPA_IS_PORT_OUTPUT (pv_LD->PortDescriptors[port])) && (LADSPA_IS_PORT_CONTROL(pv_LD->PortDescriptors[port]))) {
					pv_ports[port].name = strdup(pv_LD->PortNames[port]);
					pv_ports[port].is_out_ctl = true;
				}
				// Audio input ports
				if(LADSPA_IS_PORT_INPUT (pv_LD->PortDescriptors[port]) &&
					(LADSPA_IS_PORT_AUDIO(pv_LD->PortDescriptors[port])) ) {
					pv_ports[port].is_in_ctl = FALSE;
					pv_nb_audio_in++;
				}
				// Audio output ports
				if(LADSPA_IS_PORT_OUTPUT (pv_LD->PortDescriptors[port]) &&
					(LADSPA_IS_PORT_AUDIO(pv_LD->PortDescriptors[port])) ) {
					pv_ports[port].is_in_ctl = FALSE;
					pv_nb_audio_out++;
				}
			} // Plugin's ports

			// Set the run mode (normal, splitted, etc...)
			pv_set_run_mode();


			// Instanciate the plugin
			pv_ladspa_handle = (pv_LD->instantiate)(pv_LD, pv_srate);
			if(pv_ladspa_handle == 0){
				std::cerr << "LP_ladspa_plugin::init_plugin: instantiate fails\n";
				return -1;
			}
			if((pv_run_mode == splitted) || (pv_run_mode == merge_output)||(pv_run_mode == no_in_splitted) ) {
				pv_ladspa_handle2 = (pv_LD->instantiate)(pv_LD, pv_srate);
				if(pv_ladspa_handle2 == 0){
					std::cerr << "LP_ladspa_plugin::init_plugin: instantiate fails\n";
					return -1;
				}
			}

//			pv_ui->show();

			std::cout << "Audio inputs: " << pv_nb_audio_in << std::endl;
			std::cout << "Audio outputs: " << pv_nb_audio_out << std::endl;

			std::cout << "Channels: " << channels << " ,sample rate: " << sample_rate << " ,buffer_len: " << pv_buf_len << std::endl;

			// Plugin counter
			count++;
			// Must jump out here, else
//			break;
		}
		index++;
	}

			// Allocate memory for audio ports NOTE taille ? /2 ??
			pv_in_L_buffer = new LADSPA_Data[pv_buf_len];
			if(pv_in_L_buffer == 0){
				std::cerr << "LP_ladspa_plugin::init_plugin: cannot allocate memory for pv_in_L_buffer\n";
				return -1;
			}
			pv_in_R_buffer = new LADSPA_Data[pv_buf_len];
			if(pv_in_R_buffer == 0){
				std::cerr << "LP_ladspa_plugin::init_plugin: cannot allocate memory for pv_in_R_buffer\n";
				return -1;
			}
			pv_out_L_buffer = new LADSPA_Data[pv_buf_len];
			if(pv_out_L_buffer == 0){
				std::cerr << "LP_ladspa_plugin::init_plugin: cannot allocate memory for pv_out_L_buffer\n";
				return -1;
			}
			pv_out_R_buffer = new LADSPA_Data[pv_buf_len];
			if(pv_out_R_buffer == 0){
				std::cerr << "LP_ladspa_plugin::init_plugin: cannot allocate memory for pv_out_R_buffer\n";
				return -1;
			}
			if(pv_run_mode == merge_output) {
				pv_out_L_buffer2 = new LADSPA_Data[pv_buf_len];
				if(pv_out_L_buffer2 == 0){
					std::cerr << "LP_ladspa_plugin::init_plugin: cannot allocate memory for pv_out_L_buffer2\n";
					return -1;
				}
				pv_out_R_buffer2 = new LADSPA_Data[pv_buf_len];
				if(pv_out_R_buffer2 == 0){
					std::cerr << "LP_ladspa_plugin::init_plugin: cannot allocate memory for pv_out_R_buffer2\n";
					return -1;
				}	
			}
std::cout << "Alloc OK\n";

	// If nothing found, return error
	if(count == 0){
		std::cerr << "LP_ladspa_plugin::init_plugin: plugin ID " << unique_ID << " was not found in " << pv_plugin_path << std::endl;
		return -1;
	}
	delete[] tmp;

	/// Ok, we are ready
	pv_plugin_ready = TRUE;

	if(activate() <0){
		std::cerr << "LP_ladspa_plugin::init_plugin: activate failed\n";
	}

	return 0;
}

// Return a ports structure
lp_ladspa_ports *LP_ladspa_plugin::get_ports()
{
	return &pv_ports[0];
}

// Returns ports count
unsigned long LP_ladspa_plugin::get_ports_count()
{
	if(pv_LD->PortCount >= LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin::get_ports_count: WARNING ports count seams not in legal value !\n";
		std::cerr << "\tIs plugin initialized ?\n";
		return 0;
	}
	return pv_LD->PortCount;
}

// Returns plugin name
char *LP_ladspa_plugin::get_plug_name()
{
	return strdup(pv_plug_name);
}

// Set the plugin's port value
int LP_ladspa_plugin::set_port_value(unsigned long port, LADSPA_Data value)
{
	if(port >= LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin::set_port_value: port index is too big\n";
		return -1;
	}
	pv_in_ctl_value[port] = value;
//	std::cout << "New valeur sur port " << port << ": " << pv_in_ctl_value[port] << std::endl;
	return 0;
}

// Get the plugin's port value
LADSPA_Data LP_ladspa_plugin::get_port_value(unsigned long port, LADSPA_Data value)
{
	if(port >= LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin::get_port_value: port index is too big\n";
		return -1;
	}
	return pv_out_ctl_value[port];
}

// Get the plugin's instance
LP_ladspa_plugin *LP_ladspa_plugin::get_instance()
{
	return this;
}

/// TEST
void LP_ladspa_plugin::test()
{
			activate();
			pv_connect_ports();
			LADSPA_Data *testbuf = new LADSPA_Data[pv_buf_len];
			int y;
			for(y=0; y<pv_buf_len;y++){
				testbuf[y] = 0.1*y;
				std::cout << "[" << y << "] " << testbuf[y] << "\n";
			}
			run(testbuf);
			for(y=0; y<pv_buf_len; y++){
				std::cout << "[" << y << "] " << testbuf[y] << "\n";
			}
			delete[] testbuf;
}
// Activate the plugin
int LP_ladspa_plugin::activate()
{
	// If we are not ready, return error
	if(pv_plugin_ready == FALSE){ return -1; }


	if(pv_LD == 0){
		std::cerr << "LP_ladspa_plugin::activate: pv_LD is Null\n";
		return -1;
	}
	if(pv_ladspa_handle == 0){
		std::cerr << "LP_ladspa_plugin::activate: pv_ladspa_handle is Null\n";
		return -1;
	}

	// Test if activate ptr is not null, and call it
	if(pv_has_activate == TRUE){
		if(pv_LD->activate != 0){
			if(pv_active == FALSE){
				(pv_LD->activate)(pv_ladspa_handle);
				std::cout << "Activation du plugin\n";
				pv_active = TRUE;
			}
		}else{
			std::cerr << "LP_ladspa_plugin::activate: activate ptr is Null\n";
			pv_active = FALSE;
			return -1;
		}
	}

	return 0;
}

/// run
int LP_ladspa_plugin::run(LADSPA_Data *buffer)
{

	//TODO: pass buf_len to unsigned long in class, etc...

	// If we are not ready, simply return
	if(pv_plugin_ready == FALSE){ return 0; }

	unsigned long port = 0;
	LP_ladspa_utils utils;
	int iIN = 0, iOUT = 0, i = 0;

	if(pv_LD == 0){
		std::cout << "LP_ladspa_plugin::run: pv_LD is Null\n";
		return -1;
	}
	if(buffer == 0){
		std::cout << "LP_ladspa_plugin::run: buffer is Null\n";
		return -1;
	}
	if(pv_LD->run == 0){
		std::cout << "LP_ladspa_plugin::run: run function is Null\n";
		return -1;		
	}

	// Test nb_channels and plugin's channels 
	if(pv_nb_channels != 2 ){
		std::cerr << "LP_ladspa_plugin::run: channels passed must be 2\n";
		return -1;
	}

	if((pv_has_activate == TRUE)&&(pv_active == FALSE)) {
		return 0;
	}
std::cout << "Run...\n";
	switch(pv_run_mode) {
		case normal:
	std::cout << "Run normal\n";
			// Copy given data to L and R ports buffer
			utils.LP_LR_to_L_R(pv_in_L_buffer, pv_in_R_buffer, buffer, pv_buf_len);
			if(pv_connect_ports() >= 0){
				(pv_LD->run)(pv_ladspa_handle, (unsigned long)pv_buf_len/2);
			}
			// Copy result to given buffer
			utils.LP_L_R_to_LR(pv_out_L_buffer, pv_out_R_buffer, buffer, pv_buf_len);
			break;
		case splitted:
	std::cout << "Run splitted\n";
			// Copy given data to L and R ports buffer
			utils.LP_LR_to_L_R(pv_in_L_buffer, pv_in_R_buffer, buffer, pv_buf_len);
			if(pv_connect_ports() >= 0){
				(pv_LD->run)(pv_ladspa_handle, (unsigned long)pv_buf_len/2);
				(pv_LD->run)(pv_ladspa_handle2, (unsigned long)pv_buf_len/2);
			}
			// Copy result to given buffer
			utils.LP_L_R_to_LR(pv_out_L_buffer, pv_out_R_buffer, buffer, pv_buf_len);
			break;
		case merge_output:
	std::cout << "Run merge_output\n";
			// Copy given data to L and R ports buffer
			utils.LP_LR_to_L_R(pv_in_L_buffer, pv_in_R_buffer, buffer, pv_buf_len);
			if(pv_connect_ports() >= 0){
				(pv_LD->run)(pv_ladspa_handle, (unsigned long)pv_buf_len/2);
				(pv_LD->run)(pv_ladspa_handle2, (unsigned long)pv_buf_len/2);
			}
			// Copy result: merge L and L2, R and R2
/*			LADSPA_Data tmp1, tmp2;
			for(i=0; i<pv_buf_len; i++){
				// Left part
				tmp1 = pv_out_L_buffer[i] / 2;
				tmp2 = pv_out_L_buffer2[i] / 2;
				pv_out_L_buffer[i] = tmp1 + tmp2;
				// Right part
				tmp1 = pv_out_R_buffer[i] / 2;
				tmp2 = pv_out_R_buffer2[i] / 2;
				pv_out_R_buffer[i] = tmp1 + tmp2;
			}
*/			// Copy result to given buffer
			utils.LP_L_R_to_LR(pv_out_L_buffer2, pv_out_R_buffer, buffer, pv_buf_len);
			break;
		case no_in_normal:
	std::cout << "Run no_in_normal\n";
			// Clen buffer
/*			for(i=0; i<pv_buf_len; i++) {
				buffer[i] = 0;
			}
*/			if(pv_connect_ports() >= 0){
				(pv_LD->run)(pv_ladspa_handle, (unsigned long)pv_buf_len/2);
			}
			// Copy result to given buffer
			utils.LP_L_R_to_LR(pv_out_L_buffer, pv_out_R_buffer, buffer, pv_buf_len);
			break;
		case no_in_splitted:
	std::cout << "Run no_in_splitted\n";
			// Clen buffer
/*			for(i=0; i<pv_buf_len; i++) {
				buffer[i] = 0;
			}
*/			if(pv_connect_ports() >= 0){
				(pv_LD->run)(pv_ladspa_handle, (unsigned long)pv_buf_len/2);
				(pv_LD->run)(pv_ladspa_handle2, (unsigned long)pv_buf_len/2);
			}
			// Copy result to given buffer
			utils.LP_L_R_to_LR(pv_out_L_buffer, pv_out_R_buffer, buffer, pv_buf_len);
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

char *LP_ladspa_plugin::get_plugin_name(unsigned long index)
{
	// test variables
	if(pv_lib_handler == 0){
		std::cerr << "LP_ladspa_plugin::get_plugin_name: argument pv_lib_handler can't be NULL\n";
		return 0;
	}
	if(pv_LDF == 0){
		std::cerr << "LP_ladspa_plugin::get_plugin_name: argument Descriptor_Function can't be NULL\n";
		return 0;
	}
	if(index >= LP_MAX_PLUGIN){
		std::cerr << "LP_ladspa_plugin::get_plugin_name: index (" << index << ") is to big, max is: " << LP_MAX_PLUGIN-1 << std::endl;
		return 0;
	}

	const LADSPA_Descriptor *descriptor = 0;
	// give plugin name if aviable
	if((descriptor = pv_LDF(index)) != 0){
		return strdup(descriptor->Name);
	} else {
		return 0;
	}

	delete descriptor;
}

unsigned long LP_ladspa_plugin::get_plugin_ID(unsigned long index)
{
	// test variables
	if(pv_lib_handler == 0){
		std::cerr << "LP_ladspa_plugin::get_plugin_ID: argument pv_lib_handler can't be NULL\n";
		return 0;
	}
	if(pv_LDF == 0){
		std::cerr << "LP_ladspa_plugin::get_plugin_ID: argument Descriptor_Function can't be NULL\n";
		return 0;
	}
	if(index >= LP_MAX_PLUGIN){
		std::cerr << "LP_ladspa_plugin::get_plugin_ID: index is to big, max is: " << LP_MAX_PLUGIN-1 << std::endl;
		return 0;
	}

	const LADSPA_Descriptor *descriptor = 0;

	// give plugin name if aviable
	if((descriptor = pv_LDF(index)) != 0){
		return descriptor->UniqueID;
	} else {
		return 0;
	}

	delete descriptor;
}

/// Get the state (active or not)
bool LP_ladspa_plugin::get_active_state()
{
	if((pv_has_activate == TRUE)&&(pv_active == TRUE)){
		return TRUE;
	}
	if((pv_has_activate == TRUE)&&(pv_active == FALSE)){
		return FALSE;
	}
	if(pv_has_activate == FALSE){
		return TRUE;
	}
}

/***** private functions *****/

// Cette fonction est appele par le constructeur
int LP_ladspa_plugin::pv_obtain_desriptor_function(char *file_path)
{
	if(file_path == 0){
		std::cerr << "LP_ladspa::obtain_desriptor_function: file_path is NULL\n";
		return -1;
	}

	if(strlen(file_path) < 1){
		std::cerr << "LP_ladspa::obtain_desriptor_function: file_path is empty\n";
		return -1;
	}

	pv_LDF = 0;
	char *dl_err_output = 0;

	// Try open file passed
	pv_lib_handler = dlopen(file_path, RTLD_LAZY);
	if(pv_lib_handler == 0){
		std::cout << "LP_ladspa::obtain_desriptor_function: failed to open shard lib " << file_path << std::endl;
		std::cout << "\tError was " << dlerror() << std::endl;
		return -1;
	}

	// It's a shared library
	pv_LDF = (LADSPA_Descriptor_Function) dlsym(pv_lib_handler, "ladspa_descriptor");

	// Test dlerror() output and pv_LDF
	if((dl_err_output = dlerror()) != 0){
		std::cout << "LP_ladspa_plugin::obtain_desriptor_function: failed to load " << file_path << std::endl;
		std::cout << "\tError was " << dl_err_output << std::endl;
		return -1;
	}
	// dlsym ok, test if it's LADSPA library
	if(!pv_LDF) {
		// it was a lib, but not LADSPA.
		dlclose(pv_lib_handler);
		std::cout << "LP_ladspa_plugin::obtain_desriptor_function: " << file_path << " is not a LADSPA library\n";
		return -1;		
	}
	// We are here: ok, it's a LADSPA library
	return 0;
}

// init plugin's vars
int LP_ladspa_plugin::pv_init_plugin()
{
	// test variables
	if(pv_lib_handler == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument pv_lib_handler can't be NULL\n";
		return -1;
	}
	if(pv_LDF == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument Descriptor_Function can't be NULL\n";
		return -1;
	}


	return 0;
}

/// Connect plugin's ports
int LP_ladspa_plugin::pv_connect_ports()
{
	// If we are not ready, simply return
	if(pv_plugin_ready == FALSE){ return 0; }

	if(pv_LD->connect_port == 0){
		std::cerr << "LP_ladspa_plugin::pv_connect_ports: connect_port function is Null\n";
		return -1;
	}

	unsigned long port = 0;
	int iIN = 0, iOUT = 0;

	for(port = 0; port < pv_LD->PortCount; port++){
	//std::cout << "Connect: scan...port no " << port << std::endl;
		// If the port is a control input
		if((LADSPA_IS_PORT_INPUT (pv_LD->PortDescriptors[port])) && (LADSPA_IS_PORT_CONTROL(pv_LD->PortDescriptors[port]))) {
			// Connect input ctl port
			//pv_in_ctl_value[port];
			//std::cout << "Connexion controle, handle: " << pv_ladspa_handle << " , port: " << port << " , Addresse: " << &pv_in_ctl[port] << " , valeur: " << pv_in_ctl[port] << std::endl;
			(pv_LD->connect_port)(pv_ladspa_handle, port, &pv_in_ctl_value[port]);
			std::cout << "Connecte ctl in - port " << port << std::endl;
			// If second handle is active (mono plugins -> 2 channles)
			if(pv_ladspa_handle2 != 0){
				pv_in_ctl_value2[port] = pv_in_ctl_value[port];
				(pv_LD->connect_port)(pv_ladspa_handle2, port, &pv_in_ctl_value2[port]);
				std::cout << "Connecte ctl (handle 2) in - port " << port << std::endl;
			}
			// get all ctl port's value
		}
		// If the port is a control output
		if((LADSPA_IS_PORT_OUTPUT (pv_LD->PortDescriptors[port])) && (LADSPA_IS_PORT_CONTROL(pv_LD->PortDescriptors[port]))) {
			(pv_LD->connect_port)(pv_ladspa_handle, port, &pv_out_ctl_value[port]);
			// If second handle is active (mono plugins -> 2 channles)
			if(pv_ladspa_handle2 != 0){
				// Dont't write to the same port ! Better loose info...
				(pv_LD->connect_port)(pv_ladspa_handle2, port, &pv_out_ctl_fake[port]);
			}
		}

		// Audio input ports
		if(LADSPA_IS_PORT_INPUT (pv_LD->PortDescriptors[port]) &&
			(LADSPA_IS_PORT_AUDIO(pv_LD->PortDescriptors[port])) ) {
			// Connect audio in port
			switch(pv_run_mode) {
				case normal:
					if(iIN == 1) {
						// Right audio port
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_in_R_buffer);
					}
					if(iIN == 0) {
						// Left audio port
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_in_L_buffer);
						iIN++;
					}
					break;
				case splitted:
					(pv_LD->connect_port)(pv_ladspa_handle, port, pv_in_L_buffer);
					(pv_LD->connect_port)(pv_ladspa_handle2, port, pv_in_R_buffer);
					break;
				case merge_output:
					(pv_LD->connect_port)(pv_ladspa_handle, port, pv_in_L_buffer);
					(pv_LD->connect_port)(pv_ladspa_handle2, port, pv_in_R_buffer);
			std::cout << "Connect audio in to port " << port << std::endl;
					break;
				case no_in_normal:
					break;
				case no_in_splitted:
					break;
				default:
					std::cerr << "LP_ladspa_plugin::pv_connect_ports: unable to make audio input connections\n";
					return -1;
					break;
			}

		}
		// Audio output ports
		if(LADSPA_IS_PORT_OUTPUT (pv_LD->PortDescriptors[port]) &&
			(LADSPA_IS_PORT_AUDIO(pv_LD->PortDescriptors[port])) ) {
			// Connect audio out port
			switch(pv_run_mode) {
				case normal:
					if(iOUT == 1) {
						// Right audio port
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_R_buffer);
					}
					if(iOUT == 0) {
						// Left audio port
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_L_buffer);
						iOUT++;
					}
					break;
				case splitted:
					(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_L_buffer);
					(pv_LD->connect_port)(pv_ladspa_handle2, port, pv_out_R_buffer);
					break;
				case merge_output:
					if(iOUT == 1) {
						// Right part
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_R_buffer);
						(pv_LD->connect_port)(pv_ladspa_handle2, port, pv_out_R_buffer2);
			std::cout << "Connect audio R to port " << port << std::endl;
					}
					if(iOUT == 0) {
						// Left part
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_L_buffer);
						(pv_LD->connect_port)(pv_ladspa_handle2, port, pv_out_L_buffer2);
			std::cout << "Connect audio L to port " << port << std::endl;
						iOUT++;
					}
					break;
				case no_in_normal:
					if(iOUT == 1) {
						// Right audio port
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_R_buffer);
					}
					if(iOUT == 0) {
						// Left audio port
						(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_L_buffer);
						iOUT++;
					}
					break;
				case no_in_splitted:
					(pv_LD->connect_port)(pv_ladspa_handle, port, pv_out_L_buffer);
					(pv_LD->connect_port)(pv_ladspa_handle2, port, pv_out_R_buffer);
					break;
				default:
					std::cerr << "LP_ladspa_plugin::pv_connect_ports: unable to make audio output connections\n";
					return -1;
					break;
			}
		}
	}
	return 0;
}

// Set the run mode (normal, splitted, etc...)
int LP_ladspa_plugin::pv_set_run_mode()
{
	if((pv_nb_audio_in == pv_nb_channels)&&(pv_nb_audio_out == pv_nb_channels)) {
		pv_run_mode = normal;
	} else if ((pv_nb_channels == 2)&&(pv_nb_audio_in == 1)&&(pv_nb_audio_out == 1)) {
		pv_run_mode = splitted;
	} else if ((pv_nb_channels == 2)&&(pv_nb_audio_in == 1)&&(pv_nb_audio_out == 2)) {
		pv_run_mode = merge_output;
	} else if ((pv_nb_audio_in == 0)&&(pv_nb_audio_out == pv_nb_channels)) {
		pv_run_mode = no_in_normal;
	} else if ((pv_nb_channels == 2)&&(pv_nb_audio_in == 0)&&(pv_nb_audio_out == 1)) {
		pv_run_mode = no_in_splitted;
	} else {
		pv_run_mode = unsupported;
	}

	return 0;
}

// init plugin's ports
int LP_ladspa_plugin::pv_init_ports()
{
	// test variables
	if(pv_lib_handler == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument pv_lib_handler can't be NULL\n";
		return -1;
	}
	if(pv_LDF == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument Descriptor_Function can't be NULL\n";
		return -1;
	}


	return 0;
}

int LP_ladspa_plugin::pv_describe_plugin(unsigned long unique_ID)
{
	// test variables
	if(unique_ID == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument unique_ID can't be 0\n";
		return -1;
	}
	if(pv_lib_handler == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument pv_lib_handler can't be NULL\n";
		return -1;
	}
	if(pv_LDF == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument Descriptor_Function can't be NULL\n";
		return -1;
	}

	unsigned long index = 0, found = 0;
	const LADSPA_Descriptor *descriptor = 0;

	// list aviables plugins
	for(index=0; (descriptor = pv_LDF(index)) != 0; index++){
		if(descriptor->UniqueID == unique_ID){
			std::cout << "Ok, trouvé plugin: " << descriptor->Name << std::endl;
			found++;
			// description etc... ici
		}
	}
	// If found is 0 , nothing found, return error
	if(found == 0){
		std::cerr << "LP_ladspa_plugin::pv_describe_plugin: pluginID " << unique_ID << " not found\n";
		/* On referme le fichier de plugins ici */
		dlclose(pv_lib_handler);
		return -1;
	}
	if(found > 1){
		std::cerr << "LP_ladspa_plugin::pv_describe_plugin: more that one plugin found ! (ID: " << unique_ID << ")\n";
		/* On referme le fichier de plugins ici */
		dlclose(pv_lib_handler);
		return -1;
	}

	delete descriptor;
	return 0;
}


int LP_ladspa_plugin::pv_describe_plugin(char *label)
{
	// test variables
	if((label == 0)||(strlen(label) < 1)){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument label is not set\n";
		return -1;
	}
	if(pv_lib_handler == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument pv_lib_handler can't be NULL\n";
		return -1;
	}
	if(pv_LDF == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument Descriptor_Function can't be NULL\n";
		return -1;
	}

	unsigned long index = 0, found = 0;
	const LADSPA_Descriptor *descriptor = 0;

	// list aviables plugins
	for(index=0; (descriptor = pv_LDF(index)) != 0; index++){
		if(strcmp(descriptor->Label, label) == 0){
			std::cout << "Ok, trouvé plugin: " << descriptor->Name << std::endl;
			found++;
			// description etc... ici
		}
	}
	// If found is 0 , nothing found, return error
	if(found == 0){
		std::cerr << "LP_ladspa_plugin::pv_describe_plugin: label " << label << " not found\n";
		/* On referme le fichier de plugins ici */
		dlclose(pv_lib_handler);
		return -1;
	}
	if(found > 1){
		std::cerr << "LP_ladspa_plugin::pv_describe_plugin: more that one plugin found ! (label: " << label << ")\n";
		/* On referme le fichier de plugins ici */
		dlclose(pv_lib_handler);
		return -1;
	}

	delete descriptor;
	return 0;
}


int LP_ladspa_plugin::pv_list_plugins()
{
	if(pv_lib_handler == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument pv_lib_handler can't be NULL\n";
		return -1;
	}
	if(pv_LDF == 0){
		std::cout << "LP_ladspa_plugin::describe_plugin: argument Descriptor_Function can't be NULL\n";
		return -1;
	}

	unsigned long index = 0;
	const LADSPA_Descriptor *descriptor = 0;

	std::cout << "Aviables plugins:\n";
	for(index=0; (descriptor = pv_LDF(index)) != 0; index++){
		std::cout << descriptor->Name << " ID unqiue: " << descriptor->UniqueID << std::endl;
	}

	delete descriptor;
	return 0;
}

///********** The plugin UI ************/
/*
 *  Constructs a plugin_dlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
LP_ladspa_plugin_dlg::LP_ladspa_plugin_dlg( QWidget* parent, const char* name, WFlags fl, const char *lib_path )
    : QVBox( parent, name, fl )
{
    if ( !name )
	setName( "plugin_dlg" );

	// Label of plugin
	ql_plug_name = new QLabel( this, "ql_plug_name" );	
	pv_hbox = new QHBox(this);

	// Store path
	pv_plug_path = strdup(lib_path);

	// Tests ********/
	//pb_test = new QPushButton(pv_hbox, "pb_test");
	//connect( pb_test, SIGNAL(clicked()), this, SLOT(pb_test_run())  );

	// Set this ptrs to 0 (we verify things with ptr != 0)
	for(unsigned long i=0; i<LP_MAX_PORT; i++){
		pv_port_ui[i] = 0;
	}

	languageChange();	
	clearWState( WState_Polished );

	//pv_plugin = plugin;
}

/*
 *  Destroys the object and frees any allocated resources
 */
LP_ladspa_plugin_dlg::~LP_ladspa_plugin_dlg()
{
    	// no need to delete child widgets, Qt does it all for us

	// Delete all port instance if set
	for(unsigned long i=0; i<LP_MAX_PORT; i++){
		if(pv_port_ui[i] != 0){
			pv_port_ui[i] = 0;
		}
	}

}

// This get an instance of plugin class, init it and draw the UI
int LP_ladspa_plugin_dlg::init_plugin(LP_ladspa_plugin *plugin/*unsigned long plug_id, int channels, int sample_rate, int buf_len*/)
{
	unsigned long port; //, ul;
	// get plugin class instance, init it and get ports structure
//	pv_plugin = new LP_ladspa_plugin(pv_plug_path);
	pv_plugin = plugin;
//	pv_plugin->init_plugin(plug_id, channels, sample_rate, buf_len);
	pv_nb_ports = pv_plugin->get_ports_count();
	if(pv_nb_ports >= LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin_dlg::init_plugin: illegal ports count\n";
		return -1;
	}
	pv_ports = pv_plugin->get_ports();

	// Draw the UI
	set_plug_name(pv_plugin->get_plug_name());
	// Add ports
	for(port = 0; port < pv_nb_ports; port++){
		//add_port_ctl(port, pv_ports[port].name, pv_ports[port].is_toggle, FALSE, pv_ports[port].is_int);
		if(pv_ports[port].is_in_ctl == TRUE){
			add_port_ctl(port, pv_ports[port].name, pv_ports[port].is_toggle, FALSE, pv_ports[port].is_int, FALSE);
			set_range(port, pv_ports[port].low_bound, pv_ports[port].high_bound, pv_ports[port].is_logarithmic);
			set_default_value(port, pv_ports[port].default_value);
		}
		if(pv_ports[port].is_out_ctl == TRUE){
			add_port_ctl(port, pv_ports[port].name, FALSE, FALSE, FALSE, TRUE);
		}
	}

	show();
	return 0;
}

/***** Public fonctions *****/

int LP_ladspa_plugin_dlg::set_plug_name(char *name)
{
	if((name == 0) && (strlen(name) < 1)) {
		std::cerr << "LP_ladspa_plugin_dlg::set_plug_name: argument name is not defined\n";
		return -1;
	}

	ql_plug_name->setText(name);
	setCaption(name);

	return 0;
}

int LP_ladspa_plugin_dlg::add_port_ctl(unsigned long index, char *port_name, bool toggel, bool dial, bool is_int, bool is_out_ctl)
{
	// Verify index
	if(index > LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin_dlg::add_port_ctl: index is too big, max is: " << LP_MAX_PORT << std::endl;
		return -1;
	}

	pv_port_ui[index] = new LP_ladspa_port_dlg(pv_hbox, port_name, 0, pv_plugin, toggel, dial, is_int, index, is_out_ctl);

	return 0;
}

// Set port range
int LP_ladspa_plugin_dlg::set_range(unsigned long index, LADSPA_Data min, LADSPA_Data max, bool is_log)
{
	// Verify index
	if(index > LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin_dlg::set_range: index is too big, max is: " << LP_MAX_PORT << std::endl;
		return -1;
	}
	if(pv_port_ui[index] == 0){
		std::cerr << "LP_ladspa_plugin_dlg::set_range: pv_port_ui[" << index << "] is Null" << std::endl;
		return -1;
	}
	// Set the port range
	pv_port_ui[index]->set_range(min, max, is_log);

	return 0;
}

int LP_ladspa_plugin_dlg::set_default_value(unsigned long index, LADSPA_Data val)
{
	// Verify index
	if(index > LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin_dlg::set_default_value: index is too big, max is: " << LP_MAX_PORT << std::endl;
		return -1;
	}
	if(pv_port_ui[index] == 0){
		std::cerr << "LP_ladspa_plugin_dlg::set_default_value: pv_port_ui[" << index << "] is Null" << std::endl;
		return -1;
	}
	pv_port_ui[index]->set_default_value(val);

	return 0;
}

LADSPA_Data LP_ladspa_plugin_dlg::get_port_value(unsigned long index)
{
	// Verify index
	if(index > LP_MAX_PORT){
		std::cerr << "LP_ladspa_plugin_dlg::set_default_value: index is too big, max is: " << LP_MAX_PORT << std::endl;
		return 0;
	}
	if(pv_port_ui[index] == 0){
		std::cerr << "LP_ladspa_plugin_dlg::set_default_value: pv_port_ui[" << index << "] is Null" << std::endl;
		return 0;
	}
	return pv_port_ui[index]->get_value();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void LP_ladspa_plugin_dlg::languageChange()
{
    setCaption( tr( "Plugin" ) );
}


///***** The plugin's port UI ****/

// Contructor
LP_ladspa_port_dlg::LP_ladspa_port_dlg( QWidget *parent , const char *name , 
					WFlags fl, LP_ladspa_plugin *plugin, 
					bool toggel, bool dial, bool is_int,
					unsigned long port_index, bool out_ctl )
{
	if(port_index >= LP_MAX_PORT){
		std::cerr << "LP_ladspa_port_dlg::LP_ladspa_port_dlg: WARNING! port index is too big !\n";
	}else{
		pv_port_index = port_index;
	}
	// Set pointers to Null
	ql_port_name = 0;
	qle_value = 0;
	qsl_value = 0;
	qsl_int_value = 0;
	qdl_value = 0;
	pb_value = 0;

	pv_is_int = is_int;

	// Store plugin instance
	pv_plugin = plugin;

	// Set the int_coef to 1 as default value
	//pv_int_coef = 1;

	QVBox *vbox = new QVBox(parent);

	ql_port_name = new QLabel( vbox, "qle_port_name" );
	if( (name != 0) && (strlen(name) > 0) ){
		ql_port_name->setText(name);
	}
	// If in ctl port, draw a slider or so
	if(out_ctl == FALSE){
		if(toggel == FALSE){
			// Is the port integer values ?
			if(pv_is_int == TRUE){
				qsl_int_value = new QSlider( vbox, "qsl_int_value");
			}else{
				// Is it a dial or a slider ?
				if(dial == TRUE){
					qdl_value = new QwtKnob( vbox, "qsl_value" );
				}else{
					// Draw a slider
					qsl_value = new QwtSlider( vbox, "qsl_value", Qt::Vertical, QwtSlider::Left, QwtSlider::BgSlot );
					//qsl_value->setTickmarks( QSlider::Above);
				}
			}
		} else {
			pb_value = new QPushButton( vbox, "pb_value" );
			pb_value->setToggleButton( TRUE );
			pb_value->setText(tr("Off"));
		}
		if(qsl_value != 0){
			connect (qsl_value, SIGNAL( valueChanged(double)) , this, SLOT(pv_set_value(double)) );
		}
		if(qdl_value != 0){
			connect (qdl_value, SIGNAL( valueChanged(double)), this, SLOT(pv_set_value(double)) );
		}
		if(qsl_int_value != 0){
			connect (qsl_int_value, SIGNAL( valueChanged(int)), this, SLOT(pv_set_value(int)) );
		}
	}
	qle_value = new QLineEdit( vbox, "qle_value" );

	if(out_ctl == TRUE){
		// TODO disable qle_value
		qle_value->setText("NOT implemented yet...");
	}
}

// Destructor
LP_ladspa_port_dlg::~LP_ladspa_port_dlg()
{

}

// Public functions

int LP_ladspa_port_dlg::set_range(LADSPA_Data min, LADSPA_Data max, bool is_log)
{
	// Set privates vars
	pv_is_log = is_log;

	//std::cout << "min: " << min << " , imin: " << imin << " - max: " << max << " , imax: " << imax << std::endl;
	if(qsl_value != 0){
		if(pv_is_log == TRUE){
			qsl_value->setScale((double)min, (double)max, 1);
			min = (float)20*(log10(min));
			max = (float)20*(log10(max));
			qsl_value->setRange((double)min, (double)max);
		}else{
			qsl_value->setRange((double)min, (double)max);
		}
	}
	if(qdl_value != 0){
		if(pv_is_log == TRUE){
			qdl_value->setScale((double)min, (double)max, 1);
			min = (float)20*(log10(min));
			max = (float)20*(log10(max));
			qdl_value->setRange((double)min, (double)max);
		}else{
			qdl_value->setRange((double)min, (double)max);
			
		}
	}
	// Integer values
	if(qsl_int_value != 0){
		qsl_int_value->setRange((int)min, (int)max);
	}

	return 0;
}

int LP_ladspa_port_dlg::set_default_value(LADSPA_Data val)
{
	if(qsl_value != 0){
		//qsl_value->setValue((double)val);
		qsl_value->setValue((double)val);
	}
	if(qdl_value != 0){
		qdl_value->setValue((double)val);
	}
	// Integer values
	if(qsl_int_value != 0){
		qsl_int_value->setValue((int)val);
	}
	pv_plugin->set_port_value(pv_port_index, (LADSPA_Data)val);
//	std::cout << "OK, set default to " << (double)val << std::endl;
	return 0;
}

LADSPA_Data LP_ladspa_port_dlg::get_value()
{
	return pv_value;
}

// Private functions
void LP_ladspa_port_dlg::pv_set_value(double val)
{
	// Convert float to QString: (thanks to: http://lists.trolltech.com/qt-interest/2001-08/msg00156.html)
	QString str;
	float f_val = 0;

	if(pv_is_log == TRUE){
		val = pow(10, val/20);
		//pv_value = (LADSPA_Data)val;//NOTE type
		pv_plugin->set_port_value(pv_port_index, (LADSPA_Data)val);
	}else{
		pv_plugin->set_port_value(pv_port_index, (LADSPA_Data)val);
	}

	f_val = (float)val;
	str.sprintf("%f", f_val);

	if(qle_value != 0){
		qle_value->setText(str);
	}else{
		std::cerr << "LP_ladspa_port_dlg::pv_display_test: qle_value is NULL\n";
	}
}

void LP_ladspa_port_dlg::pv_set_value(int val)
{
	if(qsl_int_value != 0){
		qle_value->setText(QString::number(val));
	}
}
