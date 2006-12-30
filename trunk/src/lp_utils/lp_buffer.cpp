#include "lp_buffer.h"
#include <iostream>

lp_int16_t_buffer::lp_int16_t_buffer()
{
	pv_data = 0;
	pv_ready_start = 0;
	pv_ready_stop = 0;
	pv_w_start = 0;
	pv_r_start = 0;
}

lp_int16_t_buffer::~lp_int16_t_buffer()
{
	if(pv_data != 0){
		delete[] pv_data;
	}
}

int lp_int16_t_buffer::init(int buf_size)
{
	pv_data = new int16_t[buf_size];
	if(pv_data == 0){
		std::cerr << "lp_uint16_t_buffer::" << __FUNCTION__ << ": memory allocation failed for pv_data\n";
		return -1;
	}
	pv_buf_size = buf_size;
	pv_w_stop = buf_size;
	return 0;
}

int lp_int16_t_buffer::put(int16_t *data, int len)
{
	int i, y=0;

	// Capacity test
	if((pv_w_start + len) >= pv_w_stop){
		// We are at end of buffer, return to begin
		if(len >= pv_ready_start){	// FULL !
			std::cerr << "lp_uint16_t_buffer::" << __FUNCTION__ << ":buffer is FULL\n";
			return -1;
		}
		// Ok, reset to begin pos
		pv_w_start = 0;
		pv_ready_start = 0;
	}
	// Copy data
	for(i=pv_w_start; i<(pv_w_start+len); i++){
		pv_data[i] = data[y];
		y++;
	}
	// new write start
	pv_w_start = i;
	// Zone
	pv_ready_stop = i;

	return len;
}

int lp_int16_t_buffer::get(int16_t *data, int len)
{
	int i, y=0;

	// Test if enought data are present
	if((pv_ready_start+len) > pv_ready_stop){
		// Buffer is EMPTY
		return -1;
	}
	// Copy data
	for(i=pv_ready_start; i < (pv_ready_start+len); i++){
		data[y] = pv_data[i];
		y++;
	}
	// Zone
	std::cout << "\nZone lue: " << pv_ready_start << " to " << i - 1 << "\n\n";
	pv_ready_start = i;


	return len;
}

int lp_int16_t_buffer::left()
{
	return pv_ready_stop - pv_ready_start;
}
