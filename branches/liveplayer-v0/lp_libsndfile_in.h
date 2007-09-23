#ifndef LP_LIBSNDFILE_H
#define LP_LIBSNDFILE_H

#include <sndfile.h>

class lp_libsndfile_in
{
public:
  lp_libsndfile_in();
  ~lp_libsndfile_in();
  int new_file(const char *path);
  int close_file();
  long int read_frames(float *buffer, long int len);

private:
  SF_INFO pv_sf_info;	// Infos struct for sndfile
  SNDFILE *pv_snd_fd;	// file descriptor for sndfile
  sf_count_t pv_sf_len;	// temp var
};

#endif
