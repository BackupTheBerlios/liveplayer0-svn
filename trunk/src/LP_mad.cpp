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

#include "LP_mad.h"

/* contructor */
LP_mad::LP_mad(int buf_len) {

	/* verify buflen and allocate memory if OK */
	if(buf_len >= 500) { 	// FIXME  500: Arbitraire !!!!!!!
		buffer = new unsigned char [buf_len];
	} else {
		std::cerr << "LP_mad::LP_mad: buf_len not valid\n";
		exit(-1);
	}
	/* tests if buffer is valid */
	if(buffer == 0) {
		std::cerr << "LP_mad::LP_mad: unable to allocate memory for buffer\n";
		exit(-1);
	}

	/* The output buffer */
/*	struct LP_mad_buffer mad_buffer;
	mad_buffer.out_buffer = new float [buf_len * 20];	// FIXME taille Arbitraire !!! 
	if(out_buffer == 0) {
		std::cerr << "LP_mad::LP_mad: unable to allocate memory for out_buffer\n";
		exit(-1);
	}*/
}

LP_mad::~LP_mad() {

	delete[] buffer;
//	delete[] LP_mad_out_buffer;
}

/* open a file */
int LP_mad::open_file(char *file_path) {

	fds = fopen(file_path, "r");
	if(fds == 0) {
		std::cerr << "LP_mad::open_file: unable to open file: " << file_path << std::endl;
		return -1;
	}

	return 0;
}


/* read in file */
int LP_mad::read(float *out_buffer, int len) {

	/* TODO: Tests - No idea how to do... */
	//struct LP_mad_buffer data;
	fread(buffer, sizeof(char), len, fds);

	LP_mad_decode((unsigned char *)buffer, len, out_buffer);

}

/* this is the decode function */
int LP_mad_decode(unsigned char const *start, unsigned long length, float *out_buffer){

	//struct LP_mad_buffer buffer;
//	struct LP_mad_buffer *buffer = *data;
	LP_mad_buffer buffer;
	struct mad_decoder decoder;
	int result;

	/* initialize our private message structure */
	buffer.start = start;
	buffer.length = length;
	buffer.out_buffer = (float*)malloc(4000 * sizeof(float)); // FIXME  Len must not be fixed here !!!

	/* configure input, output and error functions */
//	mad_decoder_init(&decoder, &buffer, input, 0, 0, output, error, 0);
	mad_decoder_init(&decoder, &buffer, LP_mad_input, 0, 0, LP_mad_output, LP_mad_error, 0);

	/* start decoding */
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	/* release the decoder */
	mad_decoder_finish(&decoder);

	free(buffer.out_buffer);

	return result;
}


/* input callback */
enum mad_flow LP_mad_input(void *data, struct mad_stream *stream){

	struct LP_mad_buffer *buffer = (struct LP_mad_buffer*) data;

	if(buffer->length == 0) {
		return MAD_FLOW_STOP;
	}

	mad_stream_buffer(stream, buffer->start, buffer->length);

	buffer->length = 0;

	return MAD_FLOW_CONTINUE;
}


/* scale: convert mad_fixed_t (fixed 24 bit) to float (32) */
inline float LP_mad_scale(mad_fixed_t sample){

	float fsample;

	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if(sample >= MAD_F_ONE) {
		sample = MAD_F_ONE - 1;
	} else if(sample < -MAD_F_ONE) {
		sample = -MAD_F_ONE;
	}

	/* quantize */
	fsample = (float) (sample / (float) (1L << MAD_F_FRACBITS));

	return fsample;
}


/* output callback */
mad_flow LP_mad_output(void *data, struct mad_header const *header, struct mad_pcm *pcm){

	struct LP_mad_buffer *buffer = (struct LP_mad_buffer*)data;
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* copy values */
	nchannels =	pcm->channels;
	nsamples =	pcm->length;
	left_ch = 	pcm->samples[0];
	right_ch = 	pcm->samples[1];

	/* TODO: implement something to fill a out_buffer in LP_player format (float *) array */
	int i, y;
	for(i=0; i<nsamples/2; i++) {
		buffer->out_buffer[i] = LP_mad_scale(*left_ch);
		buffer->out_buffer[i+1] = LP_mad_scale(*right_ch);
	}

	return MAD_FLOW_CONTINUE;
}

/* error callback */
enum mad_flow LP_mad_error(void *data, struct mad_stream *stream, struct mad_frame *frame){

	struct LP_mad_buffer *buffer = (struct LP_mad_buffer *)data;

	fprintf(stderr, "LP_mad: docoding error 0x%04x (%s) at byte offset %u\n",
			stream->error, mad_stream_errorstr(stream), stream->this_frame - buffer->start);

}
