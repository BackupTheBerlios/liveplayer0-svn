/***************************************************************************
 *   Copyright (C) 2006 by Philippe   *
 *   nel230@gmail.ch   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "LP_audiofile.h"

/* Constantes privees */
const int LP_player::nb_channel = 2;

LP_player::LP_player(int init_player_ID, LP_ladspa_manager *_llm) {

	/* Variables */
	LP_global_audio_data audio_data;
	int err;

	/// Ladspa manager
	llm = _llm;

	/* player settings */

	/* Player ID - must stay all the player's life */
	player_ID = init_player_ID;

	/* audio_info (libsndfile) */
	audio_info = new SF_INFO;

	/* vorbisfile data structure */
	vf = new OggVorbis_File;

	/* stream from opened file with fopen */
	fds = new FILE;

	/* new file event detection - default: no event (0) */
	mfile = 0;

	/* Bus de sortie: par defaut 1 */
	/* output bus, default is 1 (see LP_global_var.cpp) */
	mbus = 1;

	/* Enable SoundTouch processing by default */
	mSoundTouch = LP_ON;

	/* default speed - if 0, player never start */
	mSpeed = 1.0;

	/* Default play direction */
	mDirection = LP_PLAY_FORWARD;


	/* Setup the buffers */

	/* rd_buffer PS: nb_channel prive, defini a 2 */
	/* rd_buffer is the buffer readen from a file, only 2ch files supported for the moment */
	rd_size = audio_data.nb_frames * nb_channel * audio_data.hd_size;
	if(rd_size < 1) {
		std::cout << "LP_player::LP_player(): cannot define rd_size\n";
		exit(-1);
	}

	/* This buffer is given to the read() function.
	   must have memory to store more sample that given in rd_size
           wenn we use resampling functions
	 */
	rd_buffer = new float[rd_size * 2];
	if(rd_buffer == 0) {
		std::cout << "LP_player::LP_player(): cannot allocate memory for rd_buffer\n";
	}

	/* This buffer recives a copy of rd_buffer. When playing reverse, must
	   copy reverse here, and then we can resample. If we copy resampled buffers
	   reverse, drops occured
	*/
	tmp_buffer = new float[rd_size * 2];
	if(tmp_buffer == 0) {
		std::cout << "LP_player::LP_player(): cannot allocate memory for rd_buffer\n";
	}

	/// Vumetre
	vu = new vu_meter(0, "test_vu", 1);
	vu->alloc_mem(rd_size *2);
	vu->set_samplerate((int)rate);
	vu->show();


	/* The buffer wich recieve the resampled data */
	sampled_buffer = new float[rd_size + 1000];
	if(sampled_buffer == 0) {
		std::cout << "LP_player::LP_player(): cannot allocate memory for sampled_buffer\n";
	}

	/* The ogg/vorbis buffer */
//	vorbis_buffer[0] = new float[rd_size * 2];
//	vorbis_buffer[1] = new float[rd_size * 2];
	//LP_utils LPu;
	vorbis_buffer = LPu.lp_fmalloc_2d_array( 2, (rd_size/2) * 2 );

	/* Run the player thread */
	err = pthread_create(&thread_id, 0, lp_player_thread, (void *)this);
	if(err != 0) {
		std::cerr << "player_thread_init: failed to create thread for player_ID " << player_ID << std::endl;
	}

	/* Attente que tout soit pret avant de retourner */
	/* while the new player thread isn't ready we don't return */
	while(this->player_ready != 1) {
		usleep(1000);
	}

	std::cout << "LP_player - init OK, rd_size: " << rd_size << std::endl;
}

LP_player::~ LP_player() {
	/* variables */
	int err;

	/* Waiting the end of the thread */
	err = pthread_join(thread_id, 0);
	if(err != 0) {
		std::cerr << "player_thread_join: failed to join thread, thread_id: " << thread_id << std::endl;
	}

	/* Liberation memoire */
	delete audio_info;
	delete vf;
	delete fds;
	delete[] rd_buffer;
	delete[] tmp_buffer;
	delete[] audio_info;
	delete[] sampled_buffer;
	delete[] vorbis_buffer;
}

/* Enable / Disable SoundTouch procesing */
int LP_player::setSoundTouch(int state) {
	if(state == LP_ON) {
		mSoundTouch = LP_ON;
		return 0;
	}
	if(state == LP_OFF) {
		mSoundTouch = LP_OFF;
		return 0;
	}
	if((state != LP_ON) && (state != LP_OFF)){
		std::cout << "LP_player::SetSoundTout: Illegal value\n";
		return -1;
	}
	return 0;
}

/* get SoundTouch processing state (ON or OFF) */
int LP_player::getSoundTouch(){
	return mSoundTouch;
}

/* Set player speed */
int LP_player::setSpeed(double speed){
	/* variables */
	double min, max;
//	LP_global_audio_data audio_data;

	/* Check if SoundTouch is enable */
	if(mSoundTouch == LP_ON) {
		/* Limits */
		min = 0.000001;		// See the reaylity in SoundTouch 
		max = 2.0;		// See the reaylity in SoundTouch
	
		if((speed <= min) || (speed >= max)) {
			std::cout << "LP_player::setSpeed: illegal speed settings\n";
			return -1;
		}
		mSpeed = speed;
		return 0;
	} else {
		mSpeed = 1.0;
		std::cout << "LP_player::setSpeed: SoundTouch is disabled\n";
		return 0;
	}
}

double LP_player::getSpeed(){
	/* Check if SoundTouch precessing is enabled, else return 1.0 */
	if(mSoundTouch == LP_ON) {
		return mSpeed;
	} else {
		return 1.0;
	}
}

/* Play direction */
int LP_player::setDirection(int direction) {
	/* test values */
	switch (direction){
		case LP_PLAY_FORWARD: mDirection = direction; 
			break;
		case LP_PLAY_REVERSE: mDirection = direction;
			break;
		default:
			std::cout << "LP_player::setDirection(): illegal parameter - set standard (forward)\n";
			mDirection = LP_PLAY_FORWARD;
			break;
	}

	return 0;
}

int LP_player::getDirection(){
	return mDirection;
}

/* Seek in file */
int LP_player::setSeek(int frames, int position) {
	/* test args - if ok, define vars */
	
	mSeekOffset = frames;
	
	switch(position) {
		case SEEK_SET: mSeekRefPos = position;
			mSeekEvent = 1;
			break;
		case SEEK_CUR: mSeekRefPos = position;
			mSeekEvent = 1;
			break;
		case SEEK_END: mSeekRefPos = position;
			mSeekEvent = 1;
			break;
		default: std::cout << "LP_player::setSeek: illegal parameter - possible are: SEEK_SET, SEEK_CUR, SEEK_END\n";
			mSeekEvent = 0;
	}
	return 0;
}

/*int LP_player::getSeek(){
	return mSeekEvent;
	mSeekEvent = 0;
}
*/
/* Ouverture d'un fichier */
/* open a file */
int LP_player::get_file(char *file) {

	/* Verifications */
	if(file == 0) {
		std::cerr << "LP_player::get_file(): no file given\n";
		return -1;
	}

	/* TEST Formats (ogg, mp3) */

	/* infos fichier - mise a 0 du champs format (requis pour ouverture en lecture) */
	/* file infos - set field 'format' to 0 (describe in API doc of libsndfile */
	audio_info->format = 0;

	/* Ouverture du fichier avec libsndfile */
	snd_fd = sf_open(file, SFM_READ, audio_info);
	if(snd_fd != 0) {
		/* Ok, tell that the lib to use is sndfile and return 0 */
		mRead_lib = LP_LIB_SNDFILE;
		return 0;
	}

	/* Ogg/Vorbis with vorbisfile
	   At first, we test if the file is in ogg/vorbis format
	 */
	if((fds = fopen(file, "r")) == 0) {
		std::cerr << "LP_player::get_file(): could not open file: " << file << std::endl;
		return -1;
	}
	if(ov_test(fds, vf, 0, 0) == 0) {
		/* it's Ogg/Vorbis file: finish open the file */
		if(ov_test_open(vf) != 0) {
			std::cerr << "LP_player::get_file(): unable to open the Ogg/Vorbis file: " << file << std::endl;
		}
		/* Ok, tell that the lib to use is vorbisfile and return 0 */
		mRead_lib = LP_LIB_VORBIS;
		return 0;
	}

	/* Open with the mad callback (mpg file) */
	mad_cb = new LP_mad(rd_size);
	if(mad_cb == 0) {
		std::cerr << "LP_player::get_file: unable to call LP_mad \n";
	} else {
		if(mad_cb->open_file(file)<0) {
			std::cerr << "LP_player::get_file: unable to open file: " << file << std::endl;
		}else {
			/* Ok, tell that the lib to use is libmad and return 0 */
			mRead_lib = LP_LIB_MAD;
			return 0;
		}
	}

	/* wenn arriving here, the file is not supported */
	std::cerr << "LP_player::get_file(): failed to open file '" << file << "'\n";
	return -1;
}

/* Recherche les variables changees et reagis
   Si un evenement s'est produit, renvois 1
   Si une erreur est survenue, renvois < 0
   Si rien ne s'est produit, renvois 0
*/
/* Look in some public vars for changes and get event. If it is,
   return 1 if an event was, 0 if no event was detected, negative code on error 
*/
int LP_player::get_event() {

	/* Variables */
	LP_global_audio_data audio_data;
	int err;
	int ret = 0;

	/* fichier a ouvrir */
	/* file to open */
	if(mfile != 0) {
		if((err = get_file(mfile)) < 0){
			std::cout << "LP_play::get_event: get_file() failed\n";
			mfile = 0;
			return err;
		}
		mfile = 0;
	ret = 1;
	}

	/* Mode de lecture: mplay_mode;	0: rien, 1: play,2: pause */
	/* player mode: 0 - nothing, 1: play - 2: pause */
	if(mplay_mode != 0) {
		switch(mplay_mode) {
			case 1: 
				audio_data.player_mode[player_ID] = 1;
			//	mplay_mode = 0;
				ret = 1; break;
			case 2: 
				audio_data.player_mode[player_ID] = 2;
			//	mplay_mode = 0;
				ret = 1; break;
			default:
			//	mplay_mode = 0;
				ret = 1; break;
		}
	}

	/* OK, on retourne  */
	return ret;
}

/* Fonction de lecture
   I take the sf_count_t which is a type long long on ix86
 */
sf_count_t LP_player::lp_read(sf_count_t samples) {

	int ret = 0;

	/* Select the read routine and read */
	switch (mRead_lib){
		case LP_LIB_SNDFILE:
			return sf_read_float(snd_fd, rd_buffer, samples);
			break;
		case LP_LIB_VORBIS:
		 int i,y, pos, pos2;	
			y=0; ret=0; pos = 0; pos2=0;
			//printf("OV  lire %d samples\n", samples);
/*			while(pos < (samples/2)){
				ret = ov_read_float(vf, &pcm, samples, &vf_current_section);
			printf("RET - %d -- current: %d\n", ret, vf_current_section);
			pos = pos + ret;
			printf("EOB - pos: %d\n", pos);
			}
*/
			ret = ov_read_float(vf, &vorbis_buffer, samples, &vf_current_section);
			LPu.lp_fcopy_2d_1d_array(vorbis_buffer, rd_buffer, 2, rd_size/2);
			return ret;
			break;
		case LP_LIB_MAD:
			ret = mad_cb->read(rd_buffer, rd_size);
			return rd_size;
			break;
		default:
			std::cerr << "LP_player::lp_read: unable to find wich lib to use for reading\n";
			return 0;
			break;
	}
}

/* Fonction de recherche de position dans le fichier */
sf_count_t LP_player::lp_seek(sf_count_t samples, int ref_pos){

	switch (mRead_lib){
		case LP_LIB_SNDFILE:
			return sf_seek(snd_fd, samples, ref_pos);
			break;
		default:
			std::cout << "LP_player::lp_read: unable to find wich lib to use for reading\n";
			return 0;
			break;
	}
}


/* Fonction player */
extern "C" void *lp_player_thread(void *p_data) {

	/* obtenir une instance de LP_player */
	/* get an instance of LP_player class */
	LP_player *data = (LP_player *)p_data;
	int ev_ret, err, i, y;
	sf_count_t rd_readen = 0;	// Taille renvoyee par sf_read_float()
	sf_count_t to_read = 0;
	int nSampled = 0;

	/* Variables audio globales */
	/* get the global vars */
	LP_global_audio_data audio_data;

	/* Speed/pitch/tempo with SoundTouch */
	double factor;
	factor = data->getSpeed();

	/* Temp vars */
	double d_tmp = 0;

	soundtouch::SoundTouch *pSoundTouch = new soundtouch::SoundTouch;

	/* Parametres soundtouch */
	/* Initial rate */
	pSoundTouch->setSampleRate(audio_data.rate);
	/* Player channels (alwways 2) */
	pSoundTouch->setChannels(2);
	pSoundTouch->setRate(factor);
	/* tests SoundTouch prcessing options */
	pSoundTouch->setSetting(SETTING_SEQUENCE_MS, 82);	// For tempo optimisation (default: 82)
	pSoundTouch->setSetting(SETTING_SEEKWINDOW_MS, 28);	// default 28
	pSoundTouch->setSetting(SETTING_OVERLAP_MS, 12);	// default 12
	pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 0);	// With 1, faster but degrade sound quality

	/// Premiers test ladspa_cpp
	data->llm->set_audio_params( 2, audio_data.rate, data->rd_size );

	/* Attendre que it_to_ot_ready == 1 */
	/* waiting until it_ot_buffer thread is ready */
	while(audio_data.it_to_ot_ready != 1){
		usleep(1000);
	}

	/* On annonce qu'on est pret (a lp_player_thread_init()) */
	/* we are ready */
	data->player_ready = 1;

	/* Attente du primier fichier */
	/* waiting on first file given */
	while(data->mfile == 0) {
		sleep(1);
	}

	/* Attente du premier evenemet */
	/* waiting on first event */
	while((ev_ret = data->get_event() != 1)) {
		if(ev_ret < 0) {
			fprintf(stderr, "lp_player_thread: get_event returned fail code\n");
			sleep(1);
		}
		printf("player %d attente event...\n", data->player_ID);
		usleep(100);
	}

	/* Samples to read: outsize (it's rd_size) * resampling factor (factor) */
	d_tmp =  data->rd_size * factor;
	to_read = (int)d_tmp;

	/* Si to_read / nb_channels (cad: 2) n'est pas entier -> corriger */
	if((to_read % 2) > 0) {
		if(to_read > (data->rd_size / factor)) {
			to_read = to_read - 1;
		} else {
			to_read = to_read + 1;
		}
	}

	/* Lecture du fichier */
	//while((rd_readen = sf_read_float(data->snd_fd, data->rd_buffer, to_read)) > 0){
	while((rd_readen = data->lp_read(to_read)) > 0){

		//rd_readen = sf_read_float(data->snd_fd, data->rd_buffer, data->rd_size);
		std::cout << "Lecture " << rd_readen << " samples, ID " << data->player_ID << std::endl;

		/* Copy rd_buffer to tmp_buffer - if reverse playing, do the reverse copy here */
		if(data->getDirection() == LP_PLAY_FORWARD) {
			/* copie, sens normal */
			for(i=0; i<rd_readen; i++) {
				data->tmp_buffer[i] = data->rd_buffer[i];
			}
		}else if(data->getDirection() == LP_PLAY_REVERSE) {
			printf("mix - REVERSE\n");
			/* copie, sens inverse */
			y = rd_readen - 1;
			for(i=0; i<rd_readen; i++){
				data->tmp_buffer[y] = data->rd_buffer[i];
				y--;
			}
		}

		/****** Simuler un delais *******/
		usleep(1000);

		/* If SoundTouch processing is enabled */
		if(data->getSoundTouch() == LP_ON){
			/* SoundTouch (rate) */
			pSoundTouch->putSamples(data->tmp_buffer, rd_readen/2);
			
			nSampled = pSoundTouch->receiveSamples(data->sampled_buffer, data->rd_size/2);
			std::cout << "Nbre processes: " << nSampled * data->nb_channel << " (ID " << data->player_ID << ")" << std::endl;
		}

		/// Premiers test ladspa_cpp ET Vu_meter
		if(data->getSoundTouch() == LP_ON) {
			data->llm->run_plugins(data->sampled_buffer);
			data->vu->run_buffer(data->sampled_buffer, nSampled);
		} else {
			data->llm->run_plugins(data->tmp_buffer);
			data->vu->run_buffer(data->sampled_buffer, rd_readen);
		}


		/* Tant que play_buf_full != 0 --> buffer plein, on attends*/
		/* witing until buffer is empty */
		while(audio_data.play_buf_full[data->player_ID] != 0) {
			usleep(1000);
				/* Attente d'evenemets */
				while((ev_ret = data->get_event() != 1)) {
					if(ev_ret < 0) {
						fprintf(stderr, "lp_player_thread: get_event returned fail code\n");
						sleep(1);
					}
					usleep(1000);
				}
		//	printf("Player %d: attente que play_buf_full == 0\n", data->player_ID);
		}

		/* Ecriture */
		/* write out - If SoundTouch is ON, send sampled_buffer, else rd_buffer */
		if(data->getSoundTouch() == LP_ON) {
			mix_out(data->sampled_buffer, (nSampled * data->nb_channel), data->mbus);
		} else {
			mix_out(data->tmp_buffer, rd_readen, data->mbus);
		}

		/* On indique que le buffer est plein */
		/* buffer is written, we say it */
		audio_data.play_buf_full[data->player_ID] = 1;

		/* Correction to_read selon revois nSampled - Seulement si rd_readen == to_read (sinon c'est que EOF) */
		if(rd_readen == to_read){
			/* If SoundTouch processing is enabled */
			if(data->getSoundTouch() == LP_ON){
				/* New speed factor */
				factor = data->getSpeed();
				pSoundTouch->setRate(factor);
				/* Samples to read: outsize (it's rd_size) * resampling factor (factor) */
				d_tmp =  data->rd_size * factor;
				to_read = (int)d_tmp;
	
				if((nSampled * data->nb_channel) < data->rd_size) {
					to_read = to_read + ((data->rd_size - (nSampled * data->nb_channel)) *  (int)factor) + 1;
					std::cout << "CORR Positiv\n";
				}
				if((nSampled * data->nb_channel) > data->rd_size) {
					to_read = to_read - (((nSampled * data->nb_channel) - data->rd_size) * (int)factor) - 1;
					std::cout << "CORR Negativ\n";
				}
			} else {
				/* Correct samples to read (can be false when enable/disable SoundTouch by last calculed to_read value)*/
				if(rd_readen < data->rd_size) {
					to_read = to_read + (data->rd_size - rd_readen);
					std::cout << "CORR Positiv\n";
				}
				if(rd_readen > data->rd_size) {
					to_read = to_read - (rd_readen - data->rd_size);
					std::cout << "CORR Negativ\n";
				}				
			}
	
			/* Si to_read / nb_channels (cad: 2) n'est pas entier -> corriger */
			if((to_read % 2) > 0) {
				if(to_read > (data->rd_size / factor)) {
					to_read = to_read - 1;
				} else {
					to_read = to_read + 1;
				}
			}
			/* Direction */
			if(data->getDirection() == LP_PLAY_REVERSE) {
				err = data->lp_seek( -(to_read), SEEK_CUR);
				if(err < 0) { printf("ERR SEEK - err no: %d\n", err); }
				std::cout << "REV - SEEK " << -(to_read) << std::endl;
			}
			/* Seek */
			if(data->mSeekRefPos > 0) {
				data->lp_seek(data->mSeekOffset, data->mSeekRefPos);
				data->mSeekRefPos = 0;
				data->mSeekOffset = 0;
			//	printf("SEEEEEEEK -------------- EVEBT\n");
			}
		}
	}

	/* Attente evenements */
	/* waiting on events */

	// status pause
	audio_data.player_mode[data->player_ID] = LP_PLAY_MODE_PAUSE;
	printf("Player %d END\n", data->player_ID);
	/* liberation */
	//pSoundTouch->clear();
	//delete(data);

	//return 0;
	/* Here I must implement proper ending, event waiting and any other things (player new file...) */
}

/* Thread it_to_ot_buffer */
int lp_it_ot_thread_init(pthread_t *thread_id) {

	/* On lance le thread it_ot_buffer */
	/* run the it_ot_buffer thread */
	int err;

	err = pthread_create(thread_id, 0, lp_it_to_ot_buffer, 0);
	if(err != 0) {
		std::cerr << "lp_it_ot_thread_init: failed to create thread" << std::endl;
		return err;
	}

	return 0;	
}

int lp_it_ot_thread_join(pthread_t thread_id) {

	int err;

	err = pthread_join(thread_id, 0);
	if(err != 0) {
		std::cerr << "lp_it_ot_thread_init: failed to join thread" << std::endl;
		return err;
	}

	return 0;
}

extern "C" void *lp_it_to_ot_buffer(void *fake) {

	/* variables internes */
	LP_global_audio_data audio_data;
	int i, m, y, ot_ite = 0;
	int err;

	/* buffer interne */
	if(audio_data.it_size < 1){
		fprintf(stderr, "lp_it_to_ot_buffer: it_size is not defined\n");
		pthread_exit(0);
	}
	float *buffer = (float *)malloc(audio_data.it_size * sizeof(float));
	if(buffer == NULL) {
		fprintf(stderr, "lp_it_to_ot_buffer: failed to allocate memory for buffer\n");
		pthread_exit(0);
	}

	/* mutex */
	if((err = pthread_mutex_init(&audio_data.it_ot_buffer_mutex, NULL)) != 0){
		fprintf(stderr, "lp_it_to_ot_buffer: unable to initialize it_ot_buffer_mutex\n");
		exit(-1);
	}

	/* On positionne play_buf_full[] a 1 */
	for(i=0; i<audio_data.nb_players; i++){
		audio_data.play_buf_full[i] = 1;
	}

	/* On indique qu'on est pret */
	audio_data.it_to_ot_ready = 1;

	while(1 > 0) {
		/* Pour chaque player */
		for(i=0; i<audio_data.nb_players; i++){
			/* Si le player est en pause, on passe */
			if(audio_data.player_mode[i] == LP_PLAY_MODE_PLAYING) {
				/* On donne le top (des que play_buf_full == 0 -> player va ecire) */		
				audio_data.play_buf_full[i] = 0;
				//printf("Top donne a %d\n", i);
	
				/* On attends que le buffer soit plein */
				while((audio_data.play_buf_full[i] != 1) && (audio_data.player_mode[i] == LP_PLAY_MODE_PLAYING)){
					usleep(1000);
				}
	
				/* copie du buffer */
				for(y=0; y<audio_data.it_size; y++) {
					buffer[y] = audio_data.mixed_buffer[y];
				}
			}
		}

		/* On calcule le Nbre d'iterations qu'il faut pour transmettre ot_buffer:
		   si rd_size / ot_size est non entier, il faut un tour de plus pour le reste */
		/* determin how many iterations we need to transmit frames to output */
		if((audio_data.it_size % audio_data.ot_size) != 0 ) {
			ot_ite = (audio_data.it_size / audio_data.ot_size) + 1;
		} else {
			ot_ite = audio_data.it_size / audio_data.ot_size;
		}

		/* On remplis ot_buffer */
		/* fill the buffer */
		m = 0;
		for(i = 0; i < ot_ite; i++) {
			while (audio_data.ot_full != 0) {
				usleep(1000);
			}
			pthread_mutex_lock(&audio_data.it_ot_buffer_mutex);

			for(y=0; y<(audio_data.ot_size); y++) {
				audio_data.ot_buffer[y] = buffer[y+m];
			}

			/* taille du buffer [frames] */
			audio_data.ot_full = 1;
			audio_data.ot_frames_to_read = audio_data.ot_size / audio_data.nb_channel;
			pthread_mutex_unlock(&audio_data.it_ot_buffer_mutex);

			/* avance dans it_buffer */
			m = m + audio_data.ot_size;
		}

		/* On nettoie les buffers de bus (sinon, echo lors du mixage!) */
		/* clean buffer (echo problems else) */
		clear_bus_buffers(0);

		/* clean buffer (else ringbuffer problems whenn passing in pause mode) */
		clean_buffer(audio_data.mixed_buffer, audio_data.it_size);
	}
	free(buffer);
}

/* Mixage 
   On donne le buffer a mixer et router, le bus (mixage "virtuel")
   Le buffer doit etre de 2 cannaux (stereo)
*/
/* Mix out function: it mix buffers providing from the player and/or send it to output bus (multitrak soundcards)
*/
int mix_out(float *in_buffer, int in_buf_size, int bus)
{
	printf("mix_out: recus %d samples pour bus %d\n", in_buf_size, bus);
	/* variables internes */
	float *out_buffer = NULL;
	float tmp1, tmp2;
	int i;
	LP_global_audio_data audio_data;

	/* Test de in_buffer */
	if(in_buffer == NULL) {
		fprintf(stderr, "mix_out: in_buffer is NULL\n");
		return -1;
	}

	/* Buffer de bus */
	switch (bus) {
		case 1: out_buffer = audio_data.bus_buffer_1; break;
		case 2: out_buffer = audio_data.bus_buffer_2; break;
		case 3: out_buffer = audio_data.bus_buffer_3; break;
		case 4: out_buffer = audio_data.bus_buffer_4; break;
		default: fprintf(stderr, "mix_out: bus not aviable: %d\n", bus);
		return -1; break;
	}

	/* Si out_bus est NULL on renvois */
	if(out_buffer == NULL) {
		fprintf(stderr, "mix_out: out_buffer is NULL\n");
		return -1;
	}

	/* mixage */
	for(i=0; i<in_buf_size; i++) {
		tmp1 = out_buffer[i];
		tmp2 = in_buffer[i];
		out_buffer[i] = (tmp1) + (tmp2 / audio_data.nb_players);
	}

	/* Routing de sortie */
	add_one_buffer(out_buffer, audio_data.mixed_buffer, in_buf_size, audio_data.nb_channel, audio_data.bus_out[bus]);

	return 0;
}

/* Cette fonction ajoute un buffer sur le buffer de sortie 
   On donne la sortie: 1, 3, 5 ou 7. Le tout travaille en stereo !
   Donc: in_buffer = 2 cannaux - Merci a Joel (un ami) pour la solution !
*/
/* This function set the output routing (multitrack soundcards)
   Thank to Joel (a friend) for the solution !
*/
int add_one_buffer(float *in_buffer, float *out_buffer, int in_buffer_len, int out_channels, int output)
{
	int i;

	/* Debug */
	//printf("add_one_buffer: out_channels: %d, output: %d\n", out_channels, output);

	/* Test nbre de cannaux disponible <-> cannaux sortie demande */
	/* Verify if we have the output request - if not -> send error */
	if((output + 1) > out_channels) {
		fprintf(stderr, "add_one_buffer: not able to route to channel %d and %d, to less output channels aviables (%d)\n",
			output, output + 1, out_channels);
		exit(-1);
	}

	output = output - 1;

	/* test que buffer soit renseigne (buffer_1 et buffer_2 sont toujour utilises */
	if(in_buffer == NULL) { fprintf(stderr, "add_one_buffer: buffer_1 cant't be NULL !\n"); exit(-1); }

	for(i=0; i<in_buffer_len/2; i++) {
		out_buffer[out_channels*i+output]   = 	in_buffer[i*2];
		out_buffer[out_channels*i+(1+output)] =	in_buffer[i*2+1];
	}

	return 0;
}

/* Cette fonction extrait 2 cannaux d'un buffer:
   Donner le cannal a extraire (1, 3, 5, etc...), elle revois le canal donne et le suivant...
   Il faut aussi preciser le nombre de cannaux que contient le buffer, et sa longueur
*/
/* can extract data from a buffer */
int extract_one_buffer(float *in_buffer,int in_buf_len , int in_nb_channels,float *out_buffer, int out_channel) {

	int i = 0;
	out_channel = out_channel - 1;

	for(i=0; i<(in_buf_len);i = i + in_nb_channels){
		out_buffer[i/2] = in_buffer[i+out_channel];
		out_buffer[(i/2)+1] = in_buffer[i+out_channel+1];
	}	

	return 0;
}

/* Fonction nettoyage d'un buffer */
int clean_buffer(float *buffer, int buf_size)
{
	int i = 0;

	if(buffer == NULL){
		fprintf(stderr, "clean_buffer: error - buffer pointer is NULL\n");
		return -1;
	}

	/* clean the buffer */
	for(i=0; i<buf_size; i++) {
		buffer[i] = 0;
	}

	return 0;
}


/* Fonction de nettoyage des buffers de bus */
/* To clear bus_buffers */
int clear_bus_buffers(int bus)
{
	int i;
	LP_global_audio_data audio_data;

	switch(bus) {
		case 0:	// Tous les bus
			for(i=0; i<audio_data.bus_buf_size; i++){
				audio_data.bus_buffer_1[i] = 0;
				audio_data.bus_buffer_2[i] = 0;
				audio_data.bus_buffer_3[i] = 0;
				audio_data.bus_buffer_4[i] = 0;
			}
		break;

		case 1:
			for(i=0; i<audio_data.bus_buf_size; i++){
				audio_data.bus_buffer_1[i] = 0;
			}
		break;

		case 2:
			for(i=0; i<audio_data.bus_buf_size; i++){
				audio_data.bus_buffer_2[i] = 0;
			}
		break;

		case 3:
			for(i=0; i<audio_data.bus_buf_size; i++){
				audio_data.bus_buffer_3[i] = 0;
			}
		break;

		case 4:
			for(i=0; i<audio_data.bus_buf_size; i++){
				audio_data.bus_buffer_4[i] = 0;
			}
		break;

	}

	return 0;
}

/* Fonction de nettoyage de mixed_buffer */
/* To clear the mixed_buffer */
int clear_mixed_buffer() {

	int i;
	LP_global_audio_data audio_data;

	for(i=0; i<audio_data.it_size; i++){
		audio_data.mixed_buffer[i] = 0;
	}

	return 0;
}
