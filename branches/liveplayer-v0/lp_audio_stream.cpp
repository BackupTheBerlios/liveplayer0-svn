#include "lp_audio_stream.h"
//#include <iostream>

lp_audio_stream::lp_audio_stream()
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  pb_il_buffer = 0;
  pv_ringbuffers = 0;
  pv_capacity = 0;
  pv_nb_channels = 0;
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
}

lp_audio_stream::~lp_audio_stream()
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  if(pb_il_buffer != 0){
	delete[] pb_il_buffer;
  }
  if(pv_ringbuffers != 0){
	delete[] pv_ringbuffers;
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
}

int lp_audio_stream::init(const int nb_channels, const long int capacity)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  pv_nb_channels = nb_channels;
  pv_capacity = capacity;
  // Allocate the public buffer
  pb_il_buffer = new lp_data_type[pv_capacity*pv_nb_channels];
  if(pb_il_buffer == 0){
	// TODO Error handling here
	pb_il_buffer = 0;
	#ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
	 pv_mutex.unlock();
	#endif
	return -1;
  }
  // Create an array of ringbuffers and init them
  pv_ringbuffers = new lp_ringbuffer[pv_nb_channels];
  if(pv_ringbuffers == 0){
	// TODO Error handling here
	pv_ringbuffers = 0;
	#ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
	 pv_mutex.unlock();
	#endif
	return -1;
  }
  int i=0, err=0, errs=0;
  for(i=0; i<pv_nb_channels; i++){
	err = pv_ringbuffers[i].init(pv_capacity);
	if(err < 0){
		// TODO Error handling here
		errs = -1; // On problem, store it
	}
  }
  if(errs < 0){
	// TODO Error handling here
	delete[] pv_ringbuffers;
	pv_ringbuffers = 0;
	#ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
	 pv_mutex.unlock();
	#endif
	return -1;
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
  // Return allocated capacity in frames
  return pv_capacity;
}

long int lp_audio_stream::store_interleaved_data(const long int nb_frames, const lp_data_type *buffer)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  int i;
  long int frames_wrote=0;
  for(i=0; i<pv_nb_channels; i++){
	if(buffer != 0){
		frames_wrote = pv_ringbuffers[i].put_data(buffer, nb_frames, pv_nb_channels, i);
	}else{
		frames_wrote = pv_ringbuffers[i].put_data(pb_il_buffer, nb_frames, pv_nb_channels, i);
	}
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
  return frames_wrote;  // We return the last channel. Each channel must allways have the same num. of frames...
}

long int lp_audio_stream::store_one_channel_data(const long int nb_frames, const int channel, const lp_data_type *buffer)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  long int frames_wrote=0;
	if(buffer != 0){
		frames_wrote = pv_ringbuffers[channel].put_data(buffer, nb_frames);
	}else{
		frames_wrote = pv_ringbuffers[channel].put_data(pb_il_buffer, nb_frames);
	}
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
  return frames_wrote;  // We return the last channel. Each channel must allways have the same num. of frames...
}

long int lp_audio_stream::get_interleaved_data(const long int nb_frames, lp_data_type *buffer)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  int i;
  long int frames_wrote=0;
  for(i=0; i<pv_nb_channels; i++){
	if(buffer != 0){
		frames_wrote = pv_ringbuffers[i].get_data(buffer, nb_frames, pv_nb_channels, i);
	}else{
		frames_wrote = pv_ringbuffers[i].get_data(pb_il_buffer, nb_frames, pv_nb_channels, i);
	}
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
  return frames_wrote;  // We return the last channel. Each channel must allways have the same num. of frames...
}

long int lp_audio_stream::get_one_channel_data(const long int nb_frames, const int channel, lp_data_type *buffer)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  long int frames_wrote=0;
	if(buffer != 0){
		frames_wrote = pv_ringbuffers[channel].get_data(buffer, nb_frames);
	}else{
		frames_wrote = pv_ringbuffers[channel].get_data(pb_il_buffer, nb_frames);
	}
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
  return frames_wrote;  // We return the last channel. Each channel must allways have the same num. of frames...
}

long int lp_audio_stream::put_blanck_data(const long int nb_frames)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  int i;
  long int frames_wrote=0;
  for(i=0; i<pv_nb_channels; i++){
	frames_wrote = pv_ringbuffers[i].put_blanck_data(pb_il_buffer, nb_frames);
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
  return frames_wrote;  // We return the last channel. Each channel must allways have the same num. of frames...
}

void lp_audio_stream::set_in_gain_factor(float factor)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  int i;
  for(i=0; i<pv_nb_channels; i++){
	pv_ringbuffers[i].set_in_gain_factor(factor);
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
}
void lp_audio_stream::set_out_vol_factor(float factor)
{
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.lock();
  #endif
  int i;
  for(i=0; i<pv_nb_channels; i++){
	pv_ringbuffers[i].set_out_vol_factor(factor);
  }
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   pv_mutex.unlock();
  #endif
}

long int lp_audio_stream::get_data_aviable_size()
{
  // NOTE Arbitraire !!
  return pv_ringbuffers[0].get_data_aviable_size();
}

long int lp_audio_stream::get_free_aviable_size()
{
  // NOTE Arbitraire !!
  return pv_ringbuffers[0].get_free_aviable_size();
}
