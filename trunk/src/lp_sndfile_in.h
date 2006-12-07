#ifndef LP_SNDFILE_IN_H 
#define LP_SNDFILE_IN_H

//#include <stdio.h> // NOTE: needed ?
#include <sndfile.h>
#include <QString>
#include "lp_utils/qstring_char.h"

// NOTE: move this to lp_player.h - or somthing like
#define LP_SEEK_SET	0
#define LP_SEEK_CUR	1
#define LP_SEEK_END	2

class lp_sndfile_in
{
public:
	lp_sndfile_in();
	~lp_sndfile_in();
	int open_file(QString file);
	int get_channels();
	int get_samplerate();
	long int get_frames_count();
	long int get_samples_count();
	// Read len samples in the file
	long int read_samples(float *buffer, long int len);
	// Read len frames in the file. a frames is the sum of samples * channels
	long int read_frames(float *buffer, long int len);
	// seek samples - from_pos are declared NOTE:
	long int seek_sample(long int len, int from_pos);
	// seek frames - from_pos are declared NOTE:
	long int seek_frame(long int len, int from_pos);

private:
	SF_INFO pv_sf_info;	// Infos struct for sndfile
	SNDFILE *pv_snd_fd;	// file descriptor for sndfile
	sf_count_t pv_sf_len;	// temp var
	long int pv_len;	// temp var
	int pv_whence;		// temp var
	long int pv_tot_frames;	// total frames count in file
	sf_count_t pv_sf_pos;	// Position in file
};

#endif

