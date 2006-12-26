#ifndef LP_LIBDV_IN_H
#define LP_LIBDV_IN_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <libdv/dv.h>
#include <libdv/dv_types.h>

#include <QString>
#include "lp_utils/qstring_char.h"

// NOTE: move this later...
#include "lp_sdl_out.h"

#ifndef LP_COLOR_SPACES
#define LP_COLOR_SPACE_YUV	0
#define LP_COLOR_SPACE_RGB	1
#define LP_COLOR_SPACE_BGR0	2
#define LP_COLOR_SPACE_LAST	3
#endif // ifndef LP_COLOR_SPACES


class lp_libdv_in
{
public:
	lp_libdv_in();
	~lp_libdv_in();
	/// Call init() and give the audio buffer total len needed (in samples)
	int init(int audio_buffer_len, int nb_audio_channels);
	int set_color_space(int color_space);
	/// Call this to open a file
	int open_file(QString file);
	int get_channels();
	int get_samplerate();
	void set_audio_samplerate(int samplerate);
	double get_audio_sampling_factor();
	long int get_frames_count();
	long int get_samples_count();
	/// Read len samples in the file
	long int read_samples(float *buffer, long int len);
	// Read len frames in the file. a frames is the sum of samples * channels
	long int read_frames(float *buffer, long int len);
	// seek samples - from_pos are declared NOTE:
	long int seek_sample(long int len, int from_pos);
	// seek frames - from_pos are declared NOTE:
	long int seek_frame(long int len, int from_pos);


private:
	int pv_fd;			// File descriptor
	ssize_t pv_file_readen;			// blocs readen in file
	ssize_t pv_file_buf_size;	// File read buffer len
	uint8_t *pv_file_buffer;	// File read buffer
	int16_t *pv_dv_audio_buffers[4];	// 4 audio buffers (4 channels) with dv decoded samples
//	uint8_t *pv_pixels[3];		// Video pixels buffers
	dv_decoder_t *pv_decoder;	// dv decoder
	dv_color_space_t pv_color_space; // color space, like RGB, YUV, etc...
	int pv_width, pv_height;	// Video w & h
	void print_err_open(int fd);	// Print out error occured by open()
	double pv_audio_sampling_factor; // The factor between needed samplerate and file's samplerate
	int16_t *pv_audio_ready_buffer; // Interlaced audio buffer contains decoded audio - interlaced
	int pv_audio_num_ready;		// len of decoded audio ready in pv_audio_ready_buffer
	int pv_audio_decoder_start;	// start position in pv_audio_ready_buffer
	int pv_audio_consumer_start;	// start position in pv_audio_ready_buffer
	int pv_dv_audio_channels;	// number of channels in dv audio stream
	int pv_audio_channels;		// number of channels in output stream

	// NOTE: move this later...
	lp_sdl_out *pv_sdl_out;
	lp_video_sample_t pv_video_format;
};

#endif
