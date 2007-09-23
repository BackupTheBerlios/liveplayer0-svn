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

#include "lp_portaudio_IO.h"

#include <iostream>

int lp_portaudio_IO::init(const int nb_inputs, const int nb_outputs, 
				const int sample_rate,
				const long int frames_per_buffer,
				lp_audio_stream *as_instance,
				QWaitCondition *signal,
				QSemaphore *wait_ready,
				int num_players)
{
  // Variables...
  pv_audio_stream = as_instance;
  pv_il_out_buffer = new float[frames_per_buffer*nb_outputs];
  pv_signal = signal;
  pv_wait_ready = wait_ready;
  pv_num_players = num_players;
  // Init Pa
  pv_pa_err = Pa_Initialize();
  if(pv_pa_err != paNoError){
	std::cerr << "lp_portaudio_IO::" << __FUNCTION__ << ": " << Pa_GetErrorText << std::endl;
	return -1;
  }
  if(pv_pa_err != paNoError){
	std::cerr << "lp_portaudio_IO::" << __FUNCTION__ << ": " << Pa_GetErrorText << std::endl;
	return -1;
  }

  pv_pa_err = Pa_OpenDefaultStream( &pv_pa_stream,
					nb_inputs, nb_outputs,
					paFloat32,
					(double)sample_rate,
					(long int)frames_per_buffer,
					lp_portaudio_IO_callback,
					this);
  if(pv_pa_err != paNoError){
	std::cerr << "lp_portaudio_IO::" << __FUNCTION__ << ": " << Pa_GetErrorText << std::endl;
	return -1;
  }

  // some infos...
  std::cout <<  "lp_portaudio_IO::" << __FUNCTION__ << ": portaudio v:" << Pa_GetVersionText() << "\n";

  return 0;
}

float *lp_portaudio_IO::get_buffer()
{
  return pv_il_out_buffer;
}

long int lp_portaudio_IO::get_frames(long int num_frames, float *buffer)
{
  return pv_audio_stream->get_interleaved_data(num_frames, buffer);
}

void lp_portaudio_IO::send_signal()
{
  pv_signal->wakeAll();
}

void lp_portaudio_IO::wait_data_ready()
{
  int i;
  for(i=0; i<pv_num_players; i++){
	pv_wait_ready->acquire();
  }
}

void lp_portaudio_IO::start_stream()
{
  pv_pa_err = Pa_StartStream(pv_pa_stream);
  if(pv_pa_err != paNoError){
	std::cerr << "lp_portaudio_IO::" << __FUNCTION__ << ": " << Pa_GetErrorText << std::endl;
	return;
  }
}


// Friend function - Callback
int lp_portaudio_IO_callback( const void *inputBuffer, void *outputBuffer,
                           	unsigned long framesPerBuffer,
                           	const PaStreamCallbackTimeInfo* timeInfo,
                           	PaStreamCallbackFlags statusFlags,
                           	void *userData )
{
  // Prevent warnings on unused variables
  (void) inputBuffer;
  // Cast data
  lp_portaudio_IO *io_data = (lp_portaudio_IO*)userData;
  //outputBuffer = (float*)io_data->get_buffer();
  float *output = (float*)outputBuffer;

  io_data->wait_data_ready();
  // Get data
  io_data->get_frames((long int)framesPerBuffer, output);

  // Signal HERE
  io_data->send_signal();
  std::cout << "Signal\n";
  return 0; // Ok ?
}
