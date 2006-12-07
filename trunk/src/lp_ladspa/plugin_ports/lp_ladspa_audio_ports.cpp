#include "lp_ladspa_audio_ports.h"

#include <iostream>

lp_ladspa_audio_ports::lp_ladspa_audio_ports(QObject *parent)
	: QThread(parent)
{
	pv_descriptor = 0;
	pv_in_data = 0;
	pv_out_data = 0;
	pv_tmp_data = 0;
	pv_nb_tmp = 0;
	pv_routing = UNSUPPORTED;
	pv_instances = 0;
}

lp_ladspa_audio_ports::~lp_ladspa_audio_ports()
{
	if(pv_out_data != 0){
		delete[] pv_in_data;
		pv_in_data = 0;
	}
	if(pv_in_data != 0){
		delete[] pv_out_data;
		pv_out_data = 0;
	}
	if(pv_tmp_data != 0){
		delete[] pv_tmp_data;
		pv_tmp_data = 0;
	}
} 

int lp_ladspa_audio_ports::init(const LADSPA_Descriptor *descriptor, int channels, int buf_size, int samplerate)
{
	//mutex.lock();
	if(descriptor == 0){
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": descriptor is Null\n";
		return -1;
	}

	// Only 1 and 2 channels are possible
	if((channels < 1)||(channels > 2)){
		std::cerr << "lp_ladspa_ctl_port::" << __FUNCTION__ << ": only 1 or 2 channels are supported\n";
		return -1;
	}
	pv_channels = channels;

	if(buf_size == 0){
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": buffer size is not set\n";
		return -1;
	}

	if(samplerate == 0){
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": samplerate is not set\n";
		return -1;
	}
	pv_descriptor = descriptor;

	// Count audio ports
	unsigned long pcount, i;
	int nb_in, nb_out;
	pcount = pv_descriptor->PortCount;

	nb_in = 0;
	nb_out = 0;
	for(i=0; i<pcount; i++){
		if((LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i]))&&(LADSPA_IS_PORT_INPUT(descriptor->PortDescriptors[i]))){
			nb_in++;
		}
		if((LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i]))&&(LADSPA_IS_PORT_OUTPUT(descriptor->PortDescriptors[i]))){
			nb_out++;
		}
	}
	pv_nb_in = nb_in;
	pv_nb_out = nb_out;
	std::cout << "Audio in: " << pv_nb_in << " - out: " << pv_nb_out << std::endl;


	// Search the best routing type
	if(channels == 1){ // 1 channel mode
		if((nb_in == 1)&&(nb_out == 1)){
			pv_routing = MONO_NORMAL;	// One handle - 1 I/O port
			pv_handles_count = 1;
			pv_nb_in = 1;
			pv_nb_out = 1;
		}else if((nb_in == 2)&&(nb_out == 2)){
			pv_routing = MONO_HALF_USED;	// One handle - 2 I/O ports
			pv_handles_count = 1;
			pv_nb_in = 2;
			pv_nb_out = 2;
		}else if((nb_in == 1)&&(nb_out == 2)){
			pv_routing = MONO_1H_MERGE_OUTPUT; // One handle - One I port - 2 O ports
			pv_handles_count = 1;
			pv_nb_in = 1;
			pv_nb_out = 2;
		}else if((nb_in == 2)&&(nb_out == 1)){
			pv_routing = MONO_1H_SPLIT_INPUT; // One handle - 2 I ports - One O port
			pv_handles_count = 1;
			pv_nb_in = 2;
			pv_nb_out = 1;
		}else if((nb_in == 0)&&(nb_out == 1)){
			pv_routing = MONO_1H_NO_INPUT;	// One handle - 0 I port - 1 O port
			pv_handles_count = 1;
			pv_nb_in = 0;
			pv_nb_out = 1;
		}else if((nb_in == 0)&&(nb_out == 2)){
			pv_routing = MONO_1H_NO_INPUT_MERGE_OUTPUT; // One handle - 0 I port - 2 O ports
			pv_handles_count = 1;
			pv_nb_in = 0;
			pv_nb_out = 2;
		}else{
			pv_routing = UNSUPPORTED;
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": cannot find audio routing\n";
		}
	}else if(channels == 2){ // 2 channels mode
		if((nb_in == 2)&&(nb_out == 2)){
			pv_routing = STEREO_NORMAL;		// One handle - 2 I/O ports
			pv_handles_count = 1;
			pv_nb_in = 2;
			pv_nb_out = 2;
		}else if((nb_in == 1)&&(nb_out == 1)){
			pv_routing = STEREO_2H;			// Two handles - 2 I/O ports
			pv_handles_count = 2;
			pv_nb_in = 2;
			pv_nb_out = 2;
		}else if((nb_in == 1)&&(nb_out == 2)){
			pv_routing = STEREO_2H_MERGE_OUTPUT;	// Two handles - 2 I ports - 4 O ports
			pv_handles_count = 2;		
			pv_nb_in = 2;
			pv_nb_out = 4;
			pv_nb_tmp = 2;
		}else if((nb_in == 2)&&(nb_out == 1)){
			pv_routing = STEREO_2H_MERGE_INPUT;	// Two handles - 4 I ports - 2 O port
			pv_handles_count = 2;
			pv_nb_in = 4;
			pv_nb_out = 2;
		}else if((nb_in == 0)&&(nb_out == 1)){
			pv_routing = STEREO_2H_NO_INPUT;	// Two handles - 0 I port - 2 O ports
			pv_handles_count = 2;
			pv_nb_in = 0;
			pv_nb_out = 2;
		}else if((nb_in == 0)&&(nb_out == 2)){
			pv_routing = STEREO_1H_NO_INPUT;	// One handle - 0 I port - 2 O ports
			pv_handles_count = 1;
			pv_nb_in = 0;
			pv_nb_out = 2;
		}else{
			pv_routing = UNSUPPORTED;
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": cannot find audio routing\n";
		}
	}else{
			pv_routing = UNSUPPORTED;
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": cannot find audio routing\n";
	}

	// Allocate in and out tabs NOTE: review buffer size ! To big in stereo mode !
	if(pv_nb_in > 0){
		pv_in_data = new lp_ladspa_audio_data[pv_nb_in];
		if(pv_in_data == 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": memory allocation failed\n";
			return -1;
		}
		for(i=0; i<(unsigned long)pv_nb_in; i++){
			if(pv_in_data[i].alloc(buf_size)<0){
				return -1;
			}
			pv_in_data[i].set_descriptor(pv_descriptor);
		}
	}
	if(pv_nb_out > 0){
		pv_out_data = new lp_ladspa_audio_data[pv_nb_out];
		if(pv_out_data == 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": memory allocation failed\n";
			return -1;
		}
		for(i=0; i<(unsigned long)pv_nb_out; i++){
			if(pv_out_data[i].alloc(buf_size)<0){
				return -1;
			}
			pv_out_data[i].set_descriptor(pv_descriptor);
		}
	}
	if(pv_nb_tmp > 0){
		pv_tmp_data = new lp_ladspa_audio_data[pv_nb_tmp];
		if(pv_tmp_data == 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": memory allocation failed\n";
			return -1;
		}
		for(i=0; i<(unsigned long)pv_nb_tmp; i++){
			if(pv_tmp_data[i].alloc(buf_size)<0){
				return -1;
			}
		}
	}

	//mutex.unlock();
	return 0;
}

int lp_ladspa_audio_ports::needed_handles_count()
{
	if(pv_routing == UNSUPPORTED){
		return 0;
	}
	return pv_handles_count;
}

bool lp_ladspa_audio_ports::supported_routing()
{
	if(pv_routing != UNSUPPORTED){
		return true;
	}
	return false;
}

QString lp_ladspa_audio_ports::get_merge_mode_name()
{
	if(pv_routing == MONO_NORMAL){
		return tr("Mono normal");
	}else if(pv_routing == MONO_HALF_USED){
		return tr("Mono half used");
	}else if(pv_routing == MONO_1H_MERGE_OUTPUT){
		return tr("Mono merge output");
	}else if(pv_routing == MONO_1H_SPLIT_INPUT){
		return tr("Mono split input");
	}else if(pv_routing == MONO_1H_NO_INPUT){
		return tr("Mono no input");
	}else if(pv_routing == MONO_1H_NO_INPUT_MERGE_OUTPUT){
		return tr("Mono no input merge output");
	}else if(pv_routing == STEREO_NORMAL){
		return tr("Stereo normal");
	}else if(pv_routing == STEREO_2H){
		return tr("Stereo coupled");
	}else if(pv_routing == STEREO_2H_MERGE_OUTPUT){
		return tr("Stereo coupled output merge");
	}else if(pv_routing == STEREO_2H_MERGE_INPUT){
		return tr("Stereo coupled input merge");
	}else if(pv_routing == STEREO_1H_NO_INPUT){
		return tr("Stereo no input");
	}else if(pv_routing == STEREO_2H_NO_INPUT){
		return tr("Stereo coupled no input");
	}else{
		return tr("Unsupported");
	}
}

int lp_ladspa_audio_ports::run_interlaced_buffer(float *buffer, int tot_len)
{
	if(pv_routing == MONO_NORMAL){
		// Copy interlaced to plugin input
		pv_in_data[0].set_data_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len);
		pv_out_data[0].get_data_copy(buffer, tot_len);
	}else if(pv_routing == MONO_HALF_USED){
		// Only use One I/O port, the others must simply benn allocated to run
		pv_in_data[0].set_data_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len);
		pv_out_data[0].get_data_copy(buffer, tot_len);
	}else if(pv_routing == MONO_1H_MERGE_OUTPUT){
		// Copy interlaced to plugin input
		pv_in_data[0].set_data_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len);
		// Mix the two outputs result to the passed buffer back
		mix_data(buffer, pv_out_data[0].data_ptr(), pv_out_data[1].data_ptr(), tot_len);
	}else if(pv_routing == MONO_1H_SPLIT_INPUT){
		// Copy interlaced to plugin input (same data to each)
		pv_in_data[0].set_data_copy(buffer, tot_len);
		pv_in_data[1].set_data_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len);
		pv_out_data[0].get_data_copy(buffer, tot_len);
	}else if(pv_routing == MONO_1H_NO_INPUT){
		// No input..
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len);
		pv_out_data[0].get_data_copy(buffer, tot_len);
	}else if(pv_routing == MONO_1H_NO_INPUT_MERGE_OUTPUT){
		// No input..
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len);
		mix_data(buffer, pv_out_data[0].data_ptr(), pv_out_data[1].data_ptr(), tot_len);
	}else if(pv_routing == STEREO_NORMAL){
		// Copy interlaced to plugin inputs
		pv_in_data[0].set_data_left_copy(buffer, tot_len);
		pv_in_data[1].set_data_right_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len/2);
		pv_out_data[0].get_data_left_copy(buffer, tot_len);
		pv_out_data[1].get_data_right_copy(buffer, tot_len);
	}else if(pv_routing == STEREO_2H){
		// Copy interlaced to plugin inputs
		pv_in_data[0].set_data_left_copy(buffer, tot_len);
		pv_in_data[0].set_data_right_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len/2);
		pv_instances[1].run(tot_len/2);
		pv_out_data[0].get_data_left_copy(buffer, tot_len);
		pv_out_data[0].get_data_right_copy(buffer, tot_len);
	}else if(pv_routing == STEREO_2H_MERGE_OUTPUT){
		// Copy interlaced to plugin inputs
		pv_in_data[0].set_data_left_copy(buffer, tot_len);
		pv_in_data[0].set_data_right_copy(buffer, tot_len);
		if(connect_ports() < 0){
			std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": connect_ports failed\n";
			return -1;
		}
		pv_instances[0].run(tot_len/2);
		pv_instances[1].run(tot_len/2);
		// Mix data 0 and 2, and store it temporary to passed buffer (half len)
		mix_data(buffer, pv_out_data[0].data_ptr(), pv_out_data[2].data_ptr(), tot_len/2);
		// Store buffer content to tmp 0
		pv_tmp_data[0].set_data_copy(buffer, tot_len/2);
		// The same with 1 and 3
		mix_data(buffer, pv_out_data[1].data_ptr(), pv_out_data[3].data_ptr(), tot_len/2);
		pv_tmp_data[1].set_data_copy(buffer, tot_len/2);
		// Copy tmp 1 and 2 to passed buffer - Ouf !
		pv_tmp_data[0].get_data_left_copy(buffer, tot_len);
		pv_tmp_data[0].get_data_right_copy(buffer, tot_len);
	}else{
		return -1;
	}

	return tot_len;
}

void lp_ladspa_audio_ports::mix_data(LADSPA_Data *output, LADSPA_Data *in1, LADSPA_Data *in2, int len)
{
	LADSPA_Data tmp1, tmp2;
	int i;
	for(i=0; i<len; i++){
		tmp1 = in1[i]/2.0;
		tmp2 = in2[i]/2.0;
		output[i] = tmp1 + tmp2;
	}
}

int lp_ladspa_audio_ports::instantiate(lp_ladspa_handle *instances)
{
	if(instances == 0){
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": instances is Null\n";
		return -1;
	}
	pv_instances = instances;

	if(pv_routing == UNSUPPORTED){
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": routing type not supported\n";
		return -1;
	}

	if(pv_routing == MONO_NORMAL){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == MONO_HALF_USED){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_in_data[1].set_port(get_lad_input_pos(1));
		pv_in_data[1].set_instance(pv_instances[0].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[1].set_port(get_lad_output_pos(1));
		pv_out_data[1].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == MONO_1H_MERGE_OUTPUT){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[1].set_port(get_lad_output_pos(1));
		pv_out_data[1].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == MONO_1H_SPLIT_INPUT){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_in_data[1].set_port(get_lad_input_pos(1));
		pv_in_data[1].set_instance(pv_instances[0].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == MONO_1H_NO_INPUT){
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == MONO_1H_NO_INPUT_MERGE_OUTPUT){
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[1].set_port(get_lad_output_pos(1));
		pv_out_data[1].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == STEREO_NORMAL){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_in_data[1].set_port(get_lad_input_pos(1));
		pv_in_data[1].set_instance(pv_instances[0].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[1].set_port(get_lad_output_pos(1));
		pv_out_data[1].set_instance(pv_instances[0].get_instance());
	}else if(pv_routing == STEREO_2H){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_in_data[1].set_port(get_lad_input_pos(0));
		pv_in_data[1].set_instance(pv_instances[1].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[1].set_port(get_lad_output_pos(0));
		pv_out_data[1].set_instance(pv_instances[1].get_instance());
	}else if(pv_routing == STEREO_2H_MERGE_OUTPUT){
		pv_in_data[0].set_port(get_lad_input_pos(0));
		pv_in_data[0].set_instance(pv_instances[0].get_instance());
		pv_in_data[1].set_port(get_lad_input_pos(0));
		pv_in_data[1].set_instance(pv_instances[1].get_instance());
		pv_out_data[0].set_port(get_lad_output_pos(0));
		pv_out_data[0].set_instance(pv_instances[0].get_instance());
		pv_out_data[1].set_port(get_lad_output_pos(1));
		pv_out_data[1].set_instance(pv_instances[0].get_instance());
		pv_out_data[2].set_port(get_lad_output_pos(0));
		pv_out_data[2].set_instance(pv_instances[1].get_instance());
		pv_out_data[3].set_port(get_lad_output_pos(1));
		pv_out_data[3].set_instance(pv_instances[1].get_instance());
	}else{
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": routing type not implemented yet\n";
		return -1;
	}

	return 0;
}

int lp_ladspa_audio_ports::connect_ports()
{
	int i;
	//if((pv_routing == MONO_NORMAL)||(pv_routing == STEREO_NORMAL)||(pv_routing == STEREO_2H)){
		for(i=0; i<pv_nb_in; i++){
			pv_in_data[i].connect_port();
		}
		for(i=0; i<pv_nb_out; i++){
			pv_out_data[i].connect_port();
		}
/*	}else{
		std::cerr << "lp_ladspa_audio_ports::" << __FUNCTION__ << ": routing type not implemented yet\n";
		return -1;
	}
*/
	return 0;
}

unsigned long lp_ladspa_audio_ports::get_lad_input_pos(int pos)
{
	int in_count = 0;
	unsigned long i;
	for(i=0; i<pv_descriptor->PortCount; i++){
		if((LADSPA_IS_PORT_AUDIO(pv_descriptor->PortDescriptors[i]))&&(LADSPA_IS_PORT_INPUT(pv_descriptor->PortDescriptors[i]))){
			if(in_count == pos){
				return i;
			}
			in_count++;
		}
	}
	return -1;
}

unsigned long lp_ladspa_audio_ports::get_lad_output_pos(int pos)
{
	// Set the ports and descriptor
	int out_count = 0;
	unsigned long i;
	for(i=0; i<pv_descriptor->PortCount; i++){
		if((LADSPA_IS_PORT_AUDIO(pv_descriptor->PortDescriptors[i]))&&(LADSPA_IS_PORT_OUTPUT(pv_descriptor->PortDescriptors[i]))){
			if(out_count == pos){
				return i;
			}
			out_count++;
		}
	}


	return -1;
}

// Thread - unused
void lp_ladspa_audio_ports::run()
{
}

// --------------- audi data class ------------------ //

lp_ladspa_audio_data::lp_ladspa_audio_data()
{
	pv_buffer = 0;
	pv_buf_size = 0;
	pv_instance = 0;
	pv_descriptor = 0;
	pv_port = -1;
}

lp_ladspa_audio_data::~lp_ladspa_audio_data()
{
	if(pv_buffer != 0){
		delete[] pv_buffer;
	}
}

int lp_ladspa_audio_data::alloc(int buf_size)
{
	pv_buffer = new LADSPA_Data[buf_size];
	if(pv_buffer == 0){
		std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": memory allocation failed\n";
		return -1;
	}
	pv_buf_size = buf_size;
	return 0;
}

void lp_ladspa_audio_data::set_data_copy(float *data, int len)
{
	if(len > pv_buf_size){
			std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": len is to big\n";
		return;
	}
	int i;
	for(i=0; i<len; i++){
		pv_buffer[i] = (LADSPA_Data)data[i];
	}
	pv_buf_len = len;
}

void lp_ladspa_audio_data::set_data_left_copy(float *interlaced_buffer, int tot_len)
{
	if(tot_len > pv_buf_size){
			std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": len is to big\n";
		return;
	}
	int i;
	for(i=0; i<tot_len/2; i++){
		pv_buffer[i] = (LADSPA_Data)interlaced_buffer[i*2];
	}
	pv_buf_len = tot_len/2;
}

void lp_ladspa_audio_data::set_data_right_copy(float *interlaced_buffer, int tot_len)
{
	if(tot_len > pv_buf_size){
			std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": len is to big\n";
		return;
	}
	int i;
	for(i=0; i<tot_len/2; i++){
		pv_buffer[i] = (LADSPA_Data)interlaced_buffer[i*2+1];
	}
	pv_buf_len = tot_len/2;
}

LADSPA_Data *lp_ladspa_audio_data::data_ptr()
{
	return pv_buffer;
}

void lp_ladspa_audio_data::get_data_copy(float *data, int len)
{
	if(len > pv_buf_size){
			std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": len is to big\n";
		return;
	}
	int i;
	for(i=0; i<len; i++){
		data[i] = (float)pv_buffer[i];
	}
}

void lp_ladspa_audio_data::get_data_left_copy(float *interlaced_buffer, int tot_len)
{
	if(tot_len > pv_buf_size){
			std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": len is to big\n";
		return;
	}
	int i;
	for(i=0; i<tot_len/2; i++){
		interlaced_buffer[i*2] = (float)pv_buffer[i];
	}
}

void lp_ladspa_audio_data::get_data_right_copy(float *interlaced_buffer, int tot_len)
{
	if(tot_len > pv_buf_size){
			std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": len is to big\n";
		return;
	}
	int i;
	for(i=0; i<tot_len/2; i++){
		interlaced_buffer[i*2+1] = (float)pv_buffer[i];
	}
}


void lp_ladspa_audio_data::set_port(unsigned long port)
{
	std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ":" << port << "\n";
	pv_port = port;
}

int lp_ladspa_audio_data::set_descriptor(const LADSPA_Descriptor *descriptor)
{
	if(descriptor == 0){
		std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": descriptor is Null\n";
		return -1;
	}
	pv_descriptor = descriptor;
	return 0;
}

int lp_ladspa_audio_data::set_instance(LADSPA_Handle instance)
{
	if(instance == 0){
		std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": instance is Null\n";
		return -1;
	}
	pv_instance = instance;
	return 0;
}

void lp_ladspa_audio_data::connect_port()
{
	if(pv_descriptor == 0){
		std::cerr << "lp_ladspa_audio_data::" << __FUNCTION__ << ": pv_descriptor is Null\n";
		return;
	}
	//std::cout << "AUDIO(I/O) - instance " << pv_instance << " - connect " << pv_buffer << " to port no " << pv_port << "\n";
	pv_descriptor->connect_port(pv_instance, pv_port, pv_buffer);
}
