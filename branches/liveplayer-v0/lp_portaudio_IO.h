#ifndef LP_PORTAUDIO_IO_H
#define LP_PORTAUDIO_IO_H

#include <portaudio.h>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>

#include "lp_audio_stream.h"

class lp_portaudio_IO
{
public:
  int init(const int nb_inputs, const int nb_outputs, 
			const int sample_rate,
			const long int frames_per_buffer,
			lp_audio_stream *as_instance,
			QWaitCondition *signal,
			QSemaphore *wait_ready,
			int num_players);
  float *get_buffer();
  long int get_frames(long int num_frames, float *buffer = 0);
  void wait_data_ready();
  void send_signal();
  void start_stream();
private:
  lp_audio_stream *pv_audio_stream;
  PaStream *pv_pa_stream;
  PaError pv_pa_err;
  QWaitCondition *pv_signal;
  QSemaphore *pv_wait_ready;
  float *pv_il_out_buffer;
  int pv_num_players;
};


  /* This routine will be called by the PortAudio engine when audio is needed.
  ** It may called at interrupt level on some machines so don't do anything
  ** that could mess up the system like calling malloc() or free().
  */ 
  int lp_portaudio_IO_callback( const void *inputBuffer, void *outputBuffer,
			unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void *userData );

#endif
