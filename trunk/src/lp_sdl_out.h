#ifndef LP_SDL_OUT_H
#define LP_SDL_OUT_H

#include <SDL/SDL.h>

/// Video sample format
typedef enum lp_video_sample_e {
	lp_sample_none = 0,
	lp_sample_411,
	lp_sample_420,
	lp_sample_422,
} lp_video_sample_t;


class lp_sdl_out
{
public:
	lp_sdl_out();
	~lp_sdl_out();
	/// At first, you must call this init function
	int init(int screen_width, int screen_height, lp_video_sample_t format);

	/// Write/read to this two vars directly
	int pb_pitches[3];
	Uint8 *pb_pixels[3];
	/// Then call this function to display
	void display();

private:
	SDL_Surface *pv_screen;
	SDL_Overlay *pv_overlay;
	SDL_Rect pv_overlay_rect;
	const SDL_VideoInfo *pv_video_info;
	lp_video_sample_t pv_format;
	SDL_Event pv_sdl_event;
	int pv_screen_height;
	int pv_screen_width;
	int pv_pixel_depth;
};

#endif
