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

#include "lp_ringbuffer.h"
#include <iostream>

lp_ringbuffer::lp_ringbuffer()
{
  pv_buffer = 0;
  pv_capacity = 0;
  pv_read_reverse = false;
  #ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
	pv_in_gain_factor = 1.0f;
	pv_out_vol_factor = 1.0f;
  #endif
}

lp_ringbuffer::~lp_ringbuffer()
{
  if(pv_buffer != 0){
	delete[] pv_buffer;
	pv_buffer = 0;
  }
}

int lp_ringbuffer::init(const long int capacity)
{
  // Set initial head position to zero
  pv_head = 0;
  pv_end = 0;
  pv_data_aviable = 0;
  pv_buffer = 0;
  pv_buffer = new lp_data_type[capacity];
  if(pv_buffer == 0){
	// Err
	pv_capacity = 0;
	return -1;
  }
  pv_capacity = capacity;
  pv_buffer_end = pv_capacity - 1;
  pv_place_aviable = pv_capacity;
  return 0;
}

long int lp_ringbuffer::put_data(const lp_data_type *il_buffer, const long int il_frames, const int il_channels, const int channel)
{
  // Check free place
  if((il_frames + pv_data_aviable) > pv_capacity){
	// Store the maximum possible
	pv_tmp_size = pv_place_aviable;
  }else{
	pv_tmp_size = il_frames;
  }
  // Set the cursor position
  pv_cursor = pv_head + pv_data_aviable;
  // Set the new end position
  pv_end = ((pv_head + pv_tmp_size) % pv_capacity);
  if(pv_end > 0){
	pv_end--;
  }else{
	pv_end = pv_buffer_end;
  }
  // Copy data
  for(pv_iterator=0; pv_iterator<pv_tmp_size; pv_iterator++){
	if(pv_cursor > pv_buffer_end){
		pv_cursor = 0;
	}
	#ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
	  pv_buffer[pv_cursor] = il_buffer[pv_iterator*il_channels+channel] * pv_in_gain_factor;
	#else
	  pv_buffer[pv_cursor] = il_buffer[pv_iterator*il_channels+channel];
	#endif
//	std::cout << "Writing: buffer[" << pv_iterator*il_channels+channel << "] to pv_buffer[" << pv_cursor << "]" << std::endl;
//	std::cout << "Storing value: " << pv_buffer[pv_cursor] << std::endl;
	pv_cursor++;
	pv_data_aviable++;
	pv_place_aviable--;
  }
  std::cout << "lp_ringbuffer::put_data: wrote: " << pv_tmp_size << " - Data aviable: " << pv_data_aviable << " - Place aviable: " << pv_place_aviable << std::endl;
  return pv_tmp_size;
}

//int lp_ringbuffer::get_data(lp_data_type *buffer, const int size)
long int lp_ringbuffer::get_data(lp_data_type *il_buffer, const long int il_frames, const int il_channels, const int channel)
{
  // Check data aviable
  if(il_frames > pv_data_aviable){
	pv_tmp_size = pv_data_aviable;
  }else{
	pv_tmp_size = il_frames;
  }
  // Set cursor position
  pv_cursor = pv_head;
  // Copy data
  if(pv_read_reverse == true){
	for(pv_iterator=pv_tmp_size-1; pv_iterator>=0; pv_iterator--){
		if(pv_cursor > pv_buffer_end){
			pv_cursor = 0;
		}
		#ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
		il_buffer[pv_iterator*il_channels+channel] = pv_buffer[pv_cursor] * pv_out_vol_factor;
		#else
		il_buffer[pv_iterator*il_channels+channel] = pv_buffer[pv_cursor];
		#endif
	//	std::cout << "Reading: buffer[" << pv_iterator*il_channels+channel << "] from pv_buffer[" << pv_cursor << "]" << std::endl;
	//	std::cout << "Reading value: " << pv_buffer[pv_cursor] << std::endl;
		pv_cursor++;
		pv_data_aviable--;
		pv_place_aviable++;
	}
  }else{
	for(pv_iterator=0; pv_iterator<pv_tmp_size; pv_iterator++){
		if(pv_cursor > pv_buffer_end){
			pv_cursor = 0;
		}
		#ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
		il_buffer[pv_iterator*il_channels+channel] = pv_buffer[pv_cursor] * pv_out_vol_factor;
		#else
		il_buffer[pv_iterator*il_channels+channel] = pv_buffer[pv_cursor];
		#endif
	//	std::cout << "Reading: buffer[" << pv_iterator*il_channels+channel << "] from pv_buffer[" << pv_cursor << "]" << std::endl;
	//	std::cout << "Reading value: " << pv_buffer[pv_cursor] << std::endl;
		pv_cursor++;
		pv_data_aviable--;
		pv_place_aviable++;
	}
  }
  std::cout << "lp_ringbuffer::get_data: readen: " << pv_tmp_size << " - Data aviable: " << pv_data_aviable << " - Place aviable: " << pv_place_aviable << std::endl;
  // Set the new head position
  pv_head = (pv_head + pv_tmp_size) % pv_capacity;
  return pv_tmp_size;
}

long int lp_ringbuffer::get_data_aviable_size()
{
  return pv_data_aviable;
}

long int lp_ringbuffer::get_free_aviable_size()
{
  return pv_place_aviable;
}

void lp_ringbuffer::set_direction_normal()
{
  pv_read_reverse = false;
}

void lp_ringbuffer::set_direction_reverse()
{
  pv_read_reverse = true;
}

#ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
 void lp_ringbuffer::set_in_gain_factor(float factor)
 {
	pv_in_gain_factor = factor;
 }
 void lp_ringbuffer::set_out_vol_factor(float factor)
 {
	pv_out_vol_factor = factor;
 }
long int lp_ringbuffer::put_blanck_data(lp_data_type *il_buffer, const long int il_frames)
 {
	for(pv_iterator=0; pv_iterator<il_frames; pv_iterator++){
		il_buffer[pv_iterator] = 0.0f;
	}
	put_data(il_buffer, il_frames);
	return il_frames;
 }
#endif

