#include "lp_sndfile_in.h"
#include <stdlib.h>
#include <iostream>

lp_sndfile_in::lp_sndfile_in()
{
	pv_snd_fd = 0;
}

lp_sndfile_in::~lp_sndfile_in()
{

}
 
int lp_sndfile_in::open_file(QString file)
{
	// Close opened files
	if(pv_snd_fd == 0){
		if(sf_close(pv_snd_fd) != 0){
			std::cerr << "lp_sndfile_in::" << __FUNCTION__ << ": cannot close previous file\n";
			std::cerr << "lp_sndfile_in::" << __FUNCTION__ << ": \tlibsndfile said: " << sf_strerror(pv_snd_fd) << "\n";
			return -1;
		}
	}

	// convert QString to char*
	qstring_char qsc(file);
	char *tmp_path = 0;
	tmp_path = qsc.to_char();

	// Init the pv_sf_info struct - must set format field to Null
	pv_sf_info.format = 0;

	pv_snd_fd = sf_open(tmp_path, SFM_READ, &pv_sf_info);
	if(pv_snd_fd == 0){
		std::cerr << "lp_sndfile_in::" << __FUNCTION__ << ": cannot open file " << tmp_path << "\n";
		std::cerr << "\tlibsndfile said: " << sf_strerror(pv_snd_fd) << "\n";
		return -1;
	}

	// debug
	std::cout << "lp_sndfile_in::open_file: frames: " << pv_sf_info.frames << "\n";
	std::cout << "lp_sndfile_in::open_file: samplearte: " << pv_sf_info.samplerate << "\n";
	std::cout << "lp_sndfile_in::open_file: channels: " << pv_sf_info.channels << "\n";

	pv_tot_frames = (long int)pv_sf_info.frames;

	if(tmp_path != 0){
		free(tmp_path);
	}

	return 0;
}

int lp_sndfile_in::get_channels()
{
	return pv_sf_info.channels;
}

int lp_sndfile_in::get_samplerate()
{
	return pv_sf_info.samplerate;
}

long int lp_sndfile_in::get_frames_count()
{
	return pv_tot_frames;
}

long int lp_sndfile_in::get_samples_count()
{
	return pv_tot_frames * (long int)pv_sf_info.channels;
}

long int lp_sndfile_in::read_samples(float *buffer, long int len)
{
	pv_sf_len = (sf_count_t)(len / (long int)pv_sf_info.channels);
	pv_sf_len = sf_read_float(pv_snd_fd, buffer, pv_sf_len);
	pv_sf_pos = pv_sf_pos + pv_sf_len;
	pv_len = ((long int)pv_sf_len * (long int)pv_sf_info.channels);
	return pv_len;
}

long int lp_sndfile_in::read_frames(float *buffer, long int len)
{
	pv_sf_len = (sf_count_t)len;
	pv_sf_len = sf_read_float(pv_snd_fd, buffer, pv_sf_len);
	pv_sf_pos = pv_sf_pos + pv_sf_len;
	return (long int)pv_sf_len;
}

long int lp_sndfile_in::seek_sample(long int len, int from_pos)
{
	if(from_pos == LP_SEEK_SET){
		pv_whence = SEEK_SET;
	}else if(from_pos == LP_SEEK_CUR){
		pv_whence = SEEK_CUR;
	}else if(from_pos == LP_SEEK_END){
		pv_whence = SEEK_END;
	}else{
		std::cerr << "lp_sndfile_in::" << __FUNCTION__ << ": seek from_pos not defined\n";
		return -1;
	}
	pv_sf_len = (sf_count_t)(len / (long int)pv_sf_info.channels);
	pv_sf_len = sf_seek(pv_snd_fd, pv_sf_len, pv_whence);
	pv_sf_pos = pv_sf_pos + pv_sf_len;
	pv_len = ((long int)pv_sf_len * (long int)pv_sf_info.channels);
	return pv_len;
}

long int lp_sndfile_in::seek_frame(long int len, int from_pos)
{
	if(from_pos == LP_SEEK_SET){
		pv_whence = SEEK_SET;
	}else if(from_pos == LP_SEEK_CUR){
		pv_whence = SEEK_CUR;
	}else if(from_pos == LP_SEEK_END){
		pv_whence = SEEK_END;
	}else{
		std::cerr << "lp_sndfile_in::" << __FUNCTION__ << ": seek from_pos not defined\n";
		return -1;
	}
	pv_sf_len = (sf_count_t)len;
	pv_sf_len = sf_seek(pv_snd_fd, pv_sf_len, pv_whence);
	pv_sf_pos = pv_sf_pos + pv_sf_len;
	return (long int)pv_sf_len;
}
