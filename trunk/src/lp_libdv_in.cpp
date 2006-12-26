#include "lp_libdv_in.h"
#include <iostream>

lp_libdv_in::lp_libdv_in()
{
	int i;
	pv_fd = 0;
	pv_file_buffer = 0;
	pv_decoder = 0;
	for(i=0; i<4; i++){
		pv_dv_audio_buffers[i] = 0;
	}
	pv_audio_ready_buffer = 0;
	pv_audio_num_ready = 0;
//	pv_pixels[0] = 0;
	set_color_space(LP_COLOR_SPACE_YUV);
	pv_audio_sampling_factor = 0.0;
	pv_audio_samplerate = 0;

	pv_sdl_out = 0;
}

lp_libdv_in::~lp_libdv_in()
{
	int i;
	if(pv_decoder != 0){
		dv_decoder_free(pv_decoder);
	}
	if(pv_file_buffer != 0){
		free(pv_file_buffer);
	}
	for(i=0; i<4; i++){
		if(pv_dv_audio_buffers[i] != 0){
			free(pv_dv_audio_buffers[i]);
		}
	}
	if(pv_audio_ready_buffer != 0){
		delete[] pv_audio_ready_buffer;
	}
/*	if(pv_pixels[0] != 0){
		free(pv_pixels);
	}
*/
	if(pv_sdl_out != 0){
		delete pv_sdl_out;
	}
}

int lp_libdv_in::init(int audio_buffer_len, int nb_audio_channels)
{
	int i;

	pv_decoder = dv_decoder_new(TRUE, FALSE, FALSE);
	if(pv_decoder == 0){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot initialise the dv decoder\n";
		return -1;
	}

	// we init buffers here - make this else later ?? -> Ok for file buffer

	// File buffer - is the size OK ?
	pv_file_buf_size = 150000;
	pv_file_buffer = (uint8_t*)malloc(pv_file_buf_size * sizeof(uint8_t));
	if(pv_file_buffer == 0){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot allocate memory for file buffer\n";
		return -1;
	}

	// Audio
	for(i=0; i<4; i++){
		pv_dv_audio_buffers[i] = (int16_t*)malloc(DV_AUDIO_MAX_SAMPLES * sizeof(int16_t));
		if(pv_dv_audio_buffers[i] == 0){
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot allocate memory for dv audio buffers\n";
			return -1;
		}
	}
	// Alloc audio ready data buffer for 2 total data length (for storage)
	pv_audio_ready_buffer = new int16_t[audio_buffer_len * 4 * 2];
	if(pv_audio_ready_buffer == 0){
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot allocate memory for pv_audio_ready_buffer\n";
			return -1;
	}
	pv_audio_channels = nb_audio_channels;

	// Debug - infos
	if(dv_format_wide(pv_decoder)){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": Format video 16/9\n";
	}
	if(dv_format_normal(pv_decoder)){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": Format video 4/3\n";
	}

	return 0;
}

int lp_libdv_in::set_color_space(int color_space)
{
	switch(color_space){
		case LP_COLOR_SPACE_YUV:
			pv_color_space = e_dv_color_yuv;
			break;
		case LP_COLOR_SPACE_RGB:
			pv_color_space = e_dv_color_rgb;
			break;
		case LP_COLOR_SPACE_BGR0:
			pv_color_space = e_dv_color_bgr0;
			break;
		default:
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot set color space\n";
			return -1;
	}

	return 0;
}

int lp_libdv_in::open_file(QString file)
{
	// Close opened file
	if(pv_fd > 0){
		if(close(pv_fd)<0){
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot close previous file\n";
			return -1;
		}
	}

	// convert QString to char*
	qstring_char qsc(file);
	char *tmp_path = 0;
	tmp_path = qsc.to_char();

	pv_fd = open(tmp_path, O_RDONLY);
	if(pv_fd < 0){
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot open file: " << tmp_path << std::endl;
			print_err_open(pv_fd);
			return -1;
	}

	// Parse the header 
	if((pv_file_readen = read(pv_fd, pv_file_buffer, 120000))<1){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot parse video header - file read error\n";
		return -1;
	}
	if(dv_parse_header(pv_decoder, pv_file_buffer)<0){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot parse video header - decoding error\n";
		return -1;
	}

	// Video formats
	pv_width = pv_decoder->width;
	pv_height = pv_decoder->height;

	// YUV format - RGB ? etc...
	switch(pv_decoder->sampling){
		case e_dv_sample_411:	// YUY2
			pv_video_format = lp_sample_411;
			break;
		case e_dv_sample_420:	// YV12 
			pv_video_format = lp_sample_420;
			break;
		case e_dv_sample_422:	// YUY2
			pv_video_format = lp_sample_422;
			break;
		default:
			fprintf(stderr, "Format YUV introuvable!\n");
			break;
	}

	// Qualité dv
	dv_set_quality(pv_decoder, DV_QUALITY_BEST);

	// NOTE: move this later...
	if(pv_sdl_out == 0){
		pv_sdl_out = new lp_sdl_out;
		if(pv_sdl_out == 0){
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot instantiate sdl output\n";
			return -1;
		}
	}
	if(pv_sdl_out->init(pv_width, pv_height, pv_video_format)<0){
			std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": cannot initialize sdl output\n";
			return -1;
	}

	// Audio parameters
	pv_dv_audio_channels = pv_decoder->audio->num_channels;
	if(pv_dv_audio_channels > 2){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": dv audio stream has more than 2 channels !\n";
	}
	if(pv_dv_audio_channels < 2){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": dv audio stream has less than 2 channels !\n";
	}
	pv_audio_num_ready = 0;
	pv_audio_decoder_start = 0;
	pv_audio_consumer_start = 0;

	// DEBUG
	std::cout << "Ouverture de " << tmp_path << std::endl;

	if(tmp_path != 0){
		free(tmp_path);
	}

	return 0;
}

void lp_libdv_in::set_audio_samplerate(int samplerate)
{
	if(pv_decoder == 0){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": pv_decoder is Null - You must call init first !\n";
		return;
	}
	pv_audio_samplerate = samplerate;
}

double lp_libdv_in::get_audio_sampling_factor()
{
	if(pv_audio_samplerate == 0){
		std::cerr << "lp_libdv_in::" << __FUNCTION__ << ": You must call set_audio_samplerate first !\n";
		return 1.0;
	}
	pv_audio_sampling_factor = (double)pv_audio_samplerate / (double)pv_decoder->audio->frequency;
	return pv_audio_sampling_factor;
}

long int lp_libdv_in::read_samples(float *buffer, long int len)
{
	int audio_decoded = 0, file_readen = 0;
	int i, channel;
//	std::cerr << "lp_libdv_in::" << __FUNCTION__ << " called\n";

	// TESTS
	len = len/2;

  /* Interleave the audio into a single buffer */
/*  for (i = 0, samples = dv_get_num_samples (dv), channels = dv_get_num_channels (dv);
       i < samples; i++) {
    for (ch = 0; ch < channels; ch++) {
      oss -> buffer [j++] = out [ch] [i];
    }
  }
*/

	// Check if decoding is requierd
	while(pv_audio_num_ready <= len){
		file_readen = read(pv_fd, pv_file_buffer, 144000); // FIXME: len to read ?
		// decode a video frame and display - NOTE this part must move to another class later...
		dv_decode_full_frame(pv_decoder, pv_file_buffer, pv_color_space, pv_sdl_out->pb_pixels, pv_sdl_out->pb_pitches);
		pv_sdl_out->display();
		// decode audio
		dv_decode_full_audio(pv_decoder, pv_file_buffer, pv_dv_audio_buffers);
		audio_decoded = dv_get_num_samples(pv_decoder);
		// Copy to ready buffer
		
		for(i=0; i<audio_decoded; i++){
			for(channel = 0; channel < pv_audio_channels; channel++){
				// write to ready buffer from start position
				pv_audio_ready_buffer[pv_audio_decoder_start+channel+i*pv_audio_channels] = pv_dv_audio_buffers[channel][i];
			}
		}
		pv_audio_num_ready = pv_audio_num_ready + audio_decoded;
		// update start pos
		pv_audio_decoder_start = pv_audio_decoder_start + audio_decoded;
	}

	// Copy needed to output buffer
	for(i=0; i<len; i++){
		//buffer[i] = (float)(pv_audio_ready_buffer[pv_audio_decoder_start+i] / 32768);
		buffer[i] = (float)(pv_audio_ready_buffer[pv_audio_decoder_start+i] / 4000);
	}
	// update start pos
	pv_audio_consumer_start = pv_audio_decoder_start + len;

	// Calcule samples consomés
	pv_audio_num_ready = pv_audio_num_ready - len;

	// On déplace le reste au début de pv_audio_ready buffer
	if(pv_audio_num_ready > 0){
		for(i=0; i<pv_audio_num_ready; i++){
			pv_audio_ready_buffer[i] = pv_audio_ready_buffer[pv_audio_decoder_start+i];
		}
		// reset positions
		pv_audio_decoder_start = 0;
		pv_audio_consumer_start = 0;
	}

	std::cout << "Décodés: " << pv_audio_num_ready << " - consommés: " << len << " - prêts: " << pv_audio_num_ready << "\n";
//	std::cout << "Start decodeur: " << pv_audio_decoder_start << " - start consumer: " << pv_audio_consumer_start << "\n\n";

	// THIS is false !
	//return file_readen;
	return len;
}

void lp_libdv_in::print_err_open(int fd)
{
	char *msg = 0;

	switch(fd){
/*		case EACCES:
			msg = strdup("access denied");
			break;
		case ENAMETOOLONG:
			msg = strdup("path name is too long");
			break;
*/
		default:
			msg = strdup("unknow error");
			break;
	}

	if(msg != 0){
		std::cerr << msg << std::endl;
		free(msg);
	}
}
