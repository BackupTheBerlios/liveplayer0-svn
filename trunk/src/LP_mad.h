/***************************************************************************
 *   Copyright (C) 2006 by Philippe   *
 *   nel230@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef LP_MAD_H
#define LP_MAD_H

#include <mad.h>
#include <stdio.h>
#include <iostream>

class LP_mad {

	public:
		LP_mad(int buf_len);
		~LP_mad();
		/* Functions */
		int open_file(char *file_path);
		int read(float *buffer, int len);

	private:
		unsigned char *buffer;
		FILE *fds;
		char *file;
};

	/* here is the buffer needed for mad */
	struct LP_mad_buffer {
		unsigned char const *start;
		unsigned long length;
		float *out_buffer;
	};
		/* Fonctions */

	/* this is the decode function */
	int LP_mad_decode(unsigned char const *start, unsigned long legth, float *out_buffer);
	
	/* input callback */
	enum mad_flow LP_mad_input(void *data, struct mad_stream *stream);
	
	/* scale: convert mad_fixed_t (fixed 24 bit) to float (32) */
	inline float LP_mad_scale(mad_fixed_t sample);
	
	/* output callback */
	mad_flow LP_mad_output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
	
	/* error callback */
	enum mad_flow LP_mad_error(void *data, struct mad_stream *stream, struct mad_frame *frame);


#endif
