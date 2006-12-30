#ifndef LP_BUFFER_H
#define LP_BUFFER_H

// For uint16_t typedef
#include <libdv/dv_types.h>


class lp_int16_t_buffer
{
public:
	lp_int16_t_buffer();
	~lp_int16_t_buffer();
	int init(int buf_size);
	/// Enqueue data
	int put(int16_t *data, int len);
	/// Get data
	int get(int16_t *data, int len);
	/// Return avail data len
	int left();

private:
	int16_t *pv_data;
	int pv_buf_size;
	int pv_r_start;	// Read start pos
	int pv_w_start;	// Write start pos
	int pv_w_stop;	// Write limit pos
	// Delimitations for zone of ready data
	int pv_ready_start;
	int pv_ready_stop;

};

#endif
