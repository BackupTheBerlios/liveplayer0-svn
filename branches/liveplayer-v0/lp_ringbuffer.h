#ifndef LP_RINGBUFFER_H
#define LP_RINGBUFFER_H

/// Define the datatype needed here
typedef float lp_data_type;

#ifndef LP_RINGBUFFER_USE_FLOAT
#define LP_RINGBUFFER_USE_FLOAT
  /// Define LP_RINGBUFFER_USE_FLOAT_TRUE *only* if you use float format.
  /// if this define is set, you can use set_gain(), set_out_vol() and put_blanck_data()
  #define LP_RINGBUFFER_USE_FLOAT_TRUE
#endif

class lp_ringbuffer
{
 public:
  lp_ringbuffer();
  ~lp_ringbuffer();
 /**
   Note on frames:
	The frame terminology is used for interleaved buffer extracting/writing.
	If you have a stereo frame, and you need each channel on a separate rinbuffer,
	you will use following functions below and give num. frames that are to read/write
	from/to the interleaved buffer. It's important to understand that num. frames readen
	from an interleaved buffer EQUALS to num. items wrote in the ringbuffer. In this case
	you simply have num. channels ringbuffer instances.. 
	If you need to store the interleaved buffer without any extraction, 
	then consider num. frames = num. items (read bellow)
	So, next, 1 element can be equal to 1 frame or 1 item
	If you work with interleaved buffers, take attention with num. elements (=frames)
	-> num. element / il_channels must be integer (element % il_channels = 0)
 **/

  /// Call this first, and give request capacity (in frames or items, see above)
  /// Return 0 on success or <0 on error
  int init(const long int capacity);

 /**
   Next you have 2 functions to put and get data.
   il_buffer is a pointer to your iterleaved data buffer. This function can extract the needed channel
   il_frames is num. of frames you need to read/write to/from il_buffer.
   il_channels is the number of channels count in the il_buffer (stereo buffer = 2, for example)
   channel: in put_data(), you set wich channel is to extract from the il_buffer, from 0 to lesser than il_channels
	    in get_data(), you set wich channel is to write to the il_buffer, from 0 to lesser than il_channels
   if the buffer isn't interleaved, simply omit the 2 last parameters
   The return value is the num. frames realy wrote/extracted
 **/
  /// Use this function to put data: it returns num. element wrote or <0 on error
  long int put_data(const lp_data_type *il_buffer, const long int il_frames, const int il_channels = 1, const int channel = 0);
  /// Use this function to get data: it returns num. element readen or <0 on error
  long int get_data(lp_data_type *il_buffer, const long int il_frames, const int il_channels = 1, const int channel = 0);
  /// Returns the num. of element you can put before ringbuffer is full
  long int get_data_aviable_size();
  /// Returns the num. of element aviable in ringbuffer
  long int get_free_aviable_size();
  /// Set the readind direction: this influe on get_data()
  /// Set normal reading (forward..)
  void set_direction_normal();
  /// Set reverse reading (backward..)
  void set_direction_reverse();
  #ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
   /// set the volume change factor (1.0 = no change, <1.0 vol down)
   void set_in_gain_factor(float factor);
   void set_out_vol_factor(float factor);
   /// Store with blanck data (0.0f)
   /// You need to give a buffer for working process (data will be overwrite)
   long int put_blanck_data(lp_data_type *il_buffer, const long int il_frames);
  #endif

 private:
  lp_data_type *pv_buffer;
  long int pv_capacity;
  long int pv_buffer_end;
  long int pv_head;		// head of data
  long int pv_end;		// end of data
  long int pv_cursor;
  long int pv_data_aviable;	// Size of aviable data
  long int pv_place_aviable;	// Free place
  long int pv_tmp_size;	// Temprary size
  long int pv_iterator;
  bool pv_read_reverse;	// If true, get_data() copies data in reverse sense
  #ifdef LP_RINGBUFFER_USE_FLOAT_TRUE
   float pv_in_gain_factor;    // gain volume factor
   float pv_out_vol_factor; // output volume
  #endif
};

#endif
