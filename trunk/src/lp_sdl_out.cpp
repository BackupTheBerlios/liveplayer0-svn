#include "lp_sdl_out.h"

#include <stdlib.h>

#include <iostream>

lp_sdl_out::lp_sdl_out()
{
	pv_screen = 0;
	pv_overlay = 0;
	pv_video_info = 0;
}

lp_sdl_out::~lp_sdl_out()
{
	SDL_LockYUVOverlay(pv_overlay);
	if(pv_overlay != 0){
		SDL_FreeYUVOverlay(pv_overlay);
		pv_overlay = 0;
	}
	if(pv_screen != 0){
		SDL_FreeSurface(pv_screen);
		pv_screen = 0;
	}
}

int lp_sdl_out::init(int screen_width, int screen_height, lp_video_sample_t format)
{
	// Initialise SDL
	if( SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cerr << "lp_sdl_out::" << __FUNCTION__ << ": SDL_Init() failed\n\t-> " << SDL_GetError();
		return -1;
	}
	// Close SDL by exit
	atexit(SDL_Quit);

	// Get system pixel depth
	pv_video_info = SDL_GetVideoInfo();
	if(pv_video_info->vfmt->BitsPerPixel == 32){
		pv_pixel_depth = 32;
	}else{
		pv_pixel_depth = 16;
	}
	std::cout << "lp_sdl_out: pixel depth: " << pv_pixel_depth << "\n";

	pv_screen_height = screen_height;
	pv_screen_width = screen_width;

	// Initialise the display
	pv_screen = SDL_SetVideoMode(pv_screen_width, pv_screen_height, pv_pixel_depth, SDL_HWSURFACE);
	if(pv_screen == 0){
		std::cerr << "lp_sdl_out::" << __FUNCTION__ << ": could not set video mode:"
			<< screen_width << " x " << screen_height << " x " << pv_pixel_depth
			<< "bit depth\n\t-> " << SDL_GetError() << "\n";
		return -1;
	}

	// YUV format
	Uint32 sdl_format;
	if((format == lp_sample_411)||(format == lp_sample_422)){
		sdl_format = SDL_YUY2_OVERLAY;
		std::cout << "YUY2 choisi\n";
	}else if(format == lp_sample_420){
		sdl_format = SDL_YV12_OVERLAY;
		std::cout << "YV12 choisi\n";
	}else{
		std::cerr << "lp_sdl_out::" << __FUNCTION__ << ": no video format set\n";
		return -1;
	}
	pv_format = format;
	// force le format...
	sdl_format = SDL_YUY2_OVERLAY;

	// Initialise the SDL_Overlay surface
	pv_overlay = SDL_CreateYUVOverlay(pv_screen_width, pv_screen_height, sdl_format, pv_screen);
	if(pv_overlay == 0){
		std::cerr << "lp_sdl_out::" << __FUNCTION__ << ": could not create overlay:"
			<< screen_width << " x " << screen_height << " x " << pv_pixel_depth
			<< "bit depth\n\t-> " << SDL_GetError() << "\n";
		return -1;
	}

	// pb_pitches and pb_pixels are
	// directly changed while reading, so
	// lock the overlay now
	SDL_LockYUVOverlay(pv_overlay);

	// Get overlay parameters...
	pb_pixels[0] = pv_overlay->pixels[0];
	pb_pixels[1] = pv_overlay->pixels[1];
	pb_pixels[2] = pv_overlay->pixels[2];

	pb_pitches[0] = pv_overlay->pitches[0];
	pb_pitches[1] = pv_overlay->pitches[1];
	pb_pitches[2] = pv_overlay->pitches[2];

	// Pixels et pitches selon forma YUV FIXME
	if((pv_format == lp_sample_411)||(pv_format == lp_sample_422)){
		pb_pitches[0] = pv_screen_width * 2;
	}else{
		pb_pixels[1] = pb_pixels[0] + (pv_screen_width * pv_screen_height);
		pb_pixels[2] = pb_pixels[1] + (pv_screen_width * pv_screen_height / 4);
		pb_pitches[0] = pv_screen_width * 2;
		pb_pitches[1] = pv_screen_width / 2;
		pb_pitches[2] = pv_screen_width / 2;
	}


/*	pb_pitches[0] = pv_screen_width * 2;
	pb_pitches[1] = pv_screen_width / 2;
	pb_pitches[2] = pv_screen_width / 2;

	std::cout << "pitches[0]: " << pb_pitches[0] << "\n";
	std::cout << "pitches[1]: " << pb_pitches[1] << "\n";
	std::cout << "pitches[2]: " << pb_pitches[2] << "\n";

	pb_pixels[1] = pb_pixels[0] + (pv_screen_width * pv_screen_height);
	pb_pixels[2] = pb_pixels[1] + (pv_screen_width * pv_screen_height / 4);
*/

	// SDL rect
	pv_overlay_rect.x = 0;
	pv_overlay_rect.y = 0;
	pv_overlay_rect.w = pv_overlay->w;
	pv_overlay_rect.h = pv_overlay->h;

	SDL_WM_SetCaption("test", 0);
	//SDL_WM_ToggleFullScreen(pv_screen);

	// Let SDL thread finish init (must look for something nicer..)
	sleep(1);

	return 0;
}

void lp_sdl_out::display()
{
	// Check sdl events -> NOTE: must run in separated thread for next version !!
	if(SDL_PollEvent(&pv_sdl_event)){
		if(pv_sdl_event.type == SDL_QUIT){
			exit(0);
		}
	}

	SDL_UnlockYUVOverlay(pv_overlay);
	SDL_DisplayYUVOverlay(pv_overlay, &pv_overlay_rect);
	SDL_LockYUVOverlay(pv_overlay);
}

