#include "lp_libsndfile_in.h"

#include <iostream>

lp_libsndfile_in::lp_libsndfile_in()
{
  pv_snd_fd = 0;
}

lp_libsndfile_in::~lp_libsndfile_in()
{

}

int lp_libsndfile_in::new_file(const char *path)
{
  // Close previous opened file
  if(close_file() <0){
	return -1;
  }

  // Init the SF_INFO structure. format must be set to 0
  pv_sf_info.format = 0;
  pv_snd_fd = sf_open(path, SFM_READ, &pv_sf_info);
  if(pv_snd_fd == 0){
	std::cerr << "lp_libsndfile_in::" << __FUNCTION__ << ": cannot open file " << path << "\n";
	std::cerr << "\tlibsndfile said: " << sf_strerror(pv_snd_fd) << "\n";
	return -1;
  }
  return 0;
}

int lp_libsndfile_in::close_file()
{
  if(pv_snd_fd != 0){
	if(sf_close(pv_snd_fd) != 0){
		std::cerr << "lp_libsndfile_in::" << __FUNCTION__ << ": cannot close previous file\n";
		std::cerr << "lp_libsndfile_in::" << __FUNCTION__ << ": \tlibsndfile said: " << sf_strerror(pv_snd_fd) << "\n";
		return -1;
	}
  }
  return 0;
}

long int lp_libsndfile_in::read_frames(float *buffer, long int len)
{
  pv_sf_len = (sf_count_t)len;
  pv_sf_len = sf_readf_float(pv_snd_fd, buffer, pv_sf_len);
  return (long int)pv_sf_len;
}
