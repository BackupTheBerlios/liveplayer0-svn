#ifndef LP_LADSPA_AUDIO_PORTS_H
#define LP_LADSPA_AUDIO_PORTS_H

#include <QString>
#include <QMutex>
#include <QThread>
#include <ladspa.h>
#include "lp_ladspa_handle.h"
#include "../../lp_utils/lp_array_utils.h"

class lp_ladspa_audio_data;

class lp_ladspa_audio_ports : public QThread
{
Q_OBJECT
public:
	lp_ladspa_audio_ports(QObject *parent = 0);
	~lp_ladspa_audio_ports();
	int init(const LADSPA_Descriptor *descriptor = 0, int channels = 1 ,int buf_size = 0, int samplerate = 0);
	int needed_handles_count();
	// set the routing and instantiate audio ports
	int instantiate(lp_ladspa_handle *instances);
	bool supported_routing();
	QString get_merge_mode_name();
	/// Runs a buffer of data - if plugin is unactiv, returns 0. if err, returns -1
	/// else, it returns the number of element processed (= tot_len)
	int run_interlaced_buffer(float *buffer, int tot_len);

private:
	// Give the ocurence needed (first = 0, second = 1, etc...)
	unsigned long get_lad_input_pos(int pos);
	unsigned long get_lad_output_pos(int pos);
	QMutex mutex;
	//LADSPA_Data pv_val;
	const LADSPA_Descriptor *pv_descriptor;
	lp_ladspa_handle *pv_instances;	// Array of lp_ladspa_handle
	int pv_handles_count;		// Store the number of handles needed
	enum routing_types{
		UNSUPPORTED,
		MONO_HALF_USED,
		MONO_NORMAL,
		MONO_1H_MERGE_OUTPUT,
		MONO_1H_SPLIT_INPUT,
		MONO_1H_NO_INPUT,
		MONO_1H_NO_INPUT_MERGE_OUTPUT,
		STEREO_NORMAL,
		STEREO_2H,
		STEREO_2H_MERGE_OUTPUT,
		STEREO_2H_MERGE_INPUT,
		STEREO_1H_NO_INPUT,
		STEREO_2H_NO_INPUT
	};
	routing_types pv_routing;
	int pv_channels, pv_nb_in, pv_nb_out, pv_nb_tmp; // nb_in and nb_out are plugins i/o
	//unsigned long pv_port;
	void run();
	void mix_data(LADSPA_Data *output, LADSPA_Data *in1, LADSPA_Data *in2, int len);
	int connect_ports();
	lp_ladspa_audio_data *pv_in_data;
	lp_ladspa_audio_data *pv_out_data;
	lp_ladspa_audio_data *pv_tmp_data;
	lp_array_utils pv_array_utils;
};

class lp_ladspa_audio_data
{
public:
	lp_ladspa_audio_data();
	~lp_ladspa_audio_data();
	int alloc(int buf_size);
	int len();
	void set_data_copy(float *data, int len);
	void set_data_left_copy(float *interlaced_buffer, int tot_len);
	void set_data_right_copy(float *interlaced_buffer, int tot_len);
	LADSPA_Data *data_ptr();
	void get_data_copy(float *data, int len);
	void get_data_left_copy(float *interlaced_buffer, int tot_len);
	void get_data_right_copy(float *interlaced_buffer, int tot_len);
	void set_port(unsigned long port);
	int set_descriptor(const LADSPA_Descriptor *descriptor);
	int set_instance(LADSPA_Handle instance);
	void connect_port();

private:
	LADSPA_Data *pv_buffer;
	unsigned long pv_port;	// The ladspa port
	int pv_buf_size;	// Size of the buffer (memory allocation)
	int pv_buf_len;		// Size of used data in the buffer
	const LADSPA_Descriptor *pv_descriptor;
	LADSPA_Handle pv_instance;
};


#endif
 
