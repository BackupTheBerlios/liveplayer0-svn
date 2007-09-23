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

#ifndef LP_AUDIO_STREAM_H
#define LP_AUDIO_STREAM_H

#include "lp_ringbuffer.h"

/// For multithreading apps using Qt4, set this define
/// By creating a instance of lp_audio_stream,
/// you need to wait until it is ready before any access...
#define LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE

#ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
  #include <QMutex>
#endif

/// This is a audio storage class
/// Somes details are documented in lp_ringbuffer.h
class lp_audio_stream
{
public:
  lp_audio_stream();
  ~lp_audio_stream();
  /// Call first: capacity is in frames.
  /// pb_il_buffer will be allocated nb_channels * capacity
  /// Returns num. capacity in frames on success, <0 on error
  int init(const int nb_channels, const long int capacity);
  /// This buffer is accessible to read/write data
  /// After writing, you need to call store_interleaved_data() or store_XXXX
  lp_data_type *pb_il_buffer;
  /// Stores nb_frames of data from the pb_il_buffer to internal ringbuffers
  /// Each channel will be extracted from the interleaved pb_il_buffer
  /// Optional it's possible to get data from another buffer
  long int store_interleaved_data(const long int nb_frames, const lp_data_type *buffer = 0);
  /// Stores data from one channel. The buffer must contain contigous data
  /// Here, num. samples EQUALS to num. frames (frames/nb_channels of pb_il_buffer or buffer is used)
  /// Optional it's possible to get data from another buffer
  long int store_one_channel_data(const long int nb_frames, const int channel, const lp_data_type *buffer = 0);
  /// Get nb_frames of data and store to the pb_il_buffer
  /// Each channel will be stored to the interleaved buffer
  /// Optional it's possible to get data from another buffer
  long int get_interleaved_data(const long int nb_frames, lp_data_type *buffer = 0);
  /// Get data from one channel. The buffer will be filled with contigous data
  /// Here, num. samples EQUALS to num. frames (frames/nb_channels of pb_il_buffer or buffer is used)
  /// Optional it's possible to get data from another buffer
  long int get_one_channel_data(const long int nb_frames, const int channel, lp_data_type *buffer = 0);
  /// Wenn data aviable (ex. EOF) is lesser than num. data needed, use this function to fill end of buffer with blanck data
  long int put_blanck_data(const long int nb_frames);
  /// Putting data in internal ringbuffers is done by copy. 
  /// So it's possible tu give a volume factor. (1.0f = no volume change, <1.0f = vol. down, etc...)
  void set_in_gain_factor(float factor);
  void set_out_vol_factor(float factor);
  long int get_data_aviable_size();
  long int get_free_aviable_size();
private:
  lp_ringbuffer *pv_ringbuffers;	// Pointer to an array of rinbuffers (1 per channel)
  int pv_nb_channels;			// Num. of channels in stream
  long int pv_capacity;			// Capacity in frames
  #ifdef LP_AUDIO_STREAM_USE_QT4_QMUTEX_TRUE
   QMutex pv_mutex;			// Lock between GUI and Processing context thread
  #endif
};

#endif
