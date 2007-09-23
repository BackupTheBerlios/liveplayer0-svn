/***************************************************************************
 *   Copyright (C) 2007 by Philippe   *
 *   nel230@gmail.ch   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
