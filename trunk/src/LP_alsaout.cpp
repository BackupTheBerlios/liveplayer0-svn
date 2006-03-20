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

#include "LP_alsaout.h"

/* initialisation alsa */
/* Initialize alsa */
int LP_alsa::lp_alsa_init() {

	/* donnees audio */
	/* audio data */
	LP_global_audio_data audio_config;

	/* tests des parametres (notamment nbre de cannaux max) */
	/* test some hardware parametres (max channels aviable) */
	if((err = LP_alsa::test_hw_params()) <0) {
		std::cerr << "LP_alsa::lp_alsa_init(): failed to test hw_params\n"
			<< "Alsa returned: " << snd_strerror(err) << ", code No: " << err << std::endl;
		return err;
	}

	/* ouverture device */
	/* Open device */
	if((err = snd_pcm_open(&pcm_handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		std::cerr << "lp_alsa_init: failed to open device 'plughw:0,0\n";
		return -1;
	}

	/* Passer le handle global */
	/* Give the handle to the global pcm_handle */
	audio_config.pcm_handle = pcm_handle;

	/* ouverture sortie infos */
	/* Attach a output for snd_dump and other alsa debuging tools. */
	if((err = snd_output_stdio_attach(&output, stdout, 0)) < 0) {
		std::cerr << "lp_alsa_init: failed to create info output\n";
		return -1;
	}

	/* appel a set_hw_params */
	/* call the function  set_hw_params */
	if((err = LP_alsa::set_hw_params()) <0) {
		std::cerr << "LP_alsa::lp_alsa_init(): failed to set hw_params\n"
			<< "Alsa returned: " << snd_strerror(err) << ", code No: " << err << std::endl;
		return err;
	}

	/* appel a sw_params */
	/* call the funtion set_sw_params */
	if((err = LP_alsa::set_sw_params()) < 0){
		std::cerr << "LP_alsa::lp_alsa_init(): failed to set sw_params\n"
			<< "Alsa returned: " << snd_strerror(err) << ", code No: " << err << std::endl;
		return err;
	}

	/* debug info... */
	snd_pcm_dump(pcm_handle, output);

	/* lancement du thread */
	/* initialise the alsa_out thread */
	if((err = lp_alsa_thread_init(&thread_id)) < 0) {
		std::cerr << "LP_alsa: lp_alsa_thread_init failed\n";
		return err;
	}

	std::cout << "config carte son OK\n";

	return 0;
}

/* fermeture du device */
/* close the device */
int LP_alsa::lp_alsa_close() {

	/* On attends la fin du thread */
	/* Waiting until the thread finish */
	lp_alsa_thread_join(thread_id);
	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);

	return 0;
}

/* Tests parametres carte son */
/* make some tests on audio device */
int LP_alsa::test_hw_params() {

	/* donnees audio */
	/* audio datas... */
	LP_global_audio_data audio_config;

	/* ouverture device en hw:0,0 */
	/* open the hw:0,0 device, this returns errors if params are bad (see below) */
	if((err = snd_pcm_open(&pcm_handle, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			std::cerr << "LP_alsa::test_hw_params(): failed to open device 'plughw:0,0\n";
			return err;
	}

	/* Ouverture et config globale du device (initialisation du pointeur hw_params) */
	/* open and configure a global params space */
	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr,"LP_alsa::set_hw_params(): failed to allocate hwparams structure\n");
		return err;
	}

	if((err = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0) {
		fprintf(stderr,"LP_alsa::set_hw_params(): failed to get a global config\n");
		return err;
	}

	/* test nb_channels_max */
	/* Get the numbers of channels on this device (I now, very bad method, but dont know how...) */
	nb_max_channel = 1;
	while((err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, nb_max_channel)) < 0) {
		nb_max_channel++;
		/* si on arrive a plus de 128 -> erreur */
		if(nb_max_channel > 128) {
		std::cerr << "LP_alsa::test_hw_params(): failed to set nb_max_channel: " << nb_max_channel << std::endl;
		return err;
		}
	}

	/* sample rate */
	dir = 0;
	if((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &audio_config.rate, &dir)) < 0) {
		std::cerr << "LP_alsa::test_hw_params(): failed to set sample rate: " << audio_config.rate << std::endl;
		return err;
	}	

	/* liberation du pointeur */
	/* free... */
	snd_pcm_hw_params_free(hw_params);

	if((err = snd_pcm_close(pcm_handle)) < 0) {
		std::cerr << "LP_alsa::test_hw_params(): failed to close device: " << audio_config.nb_channel << std::endl;
		return err;
	}

	return 0;
}

/* setup carte son */
int LP_alsa::set_hw_params() {

	/* donnees audio */
	LP_global_audio_data audio_config;

	/* Configs */
	pcm_format = SND_PCM_FORMAT_FLOAT;

	/* Ouverture et config globale du device (initialisation du pointeur hw_params) */
	/* open and setup a global params space */
	if((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr,"LP_alsa::set_hw_params(): failed to allocate hwparams structure\n");
		return err;
	}

	if((err = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0) {
		fprintf(stderr,"LP_alsa::set_hw_params(): failed to get a global config\n");
		return err;
	}

	if((err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr,"LP_alsa::set_hw_params():filed to define acces type (rw_interleaved)\n");
		return err;
	}

	if((err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, pcm_format)) < 0) {
		std::cerr << "LP_alsa::set_hw_params(): failed to set format\n";
		return err;
	}
	/* test du sbit */
	if((err = snd_pcm_hw_params_get_sbits(hw_params)) < 0) {
		std::cerr << "LP_alsa::set_hw_params(): failed to get sbit format\n";
		return err;
	}	

	/* Nombre de cannaux du lecteur: si plus grand que nb_max_channel -> =nb_max_channel || si < 2 -> =2 */
	/* Set channels for the player: if bigger than hardware can, set to hw max, if < 2 -> set to 2 */
	if(audio_config.nb_channel > nb_max_channel) {
		audio_config.nb_channel = nb_max_channel;
		std::cout << "LP_alsa::set_hw_params(): nb_channel fixed to " << audio_config.nb_channel << std::endl;
	}
	if(audio_config.nb_channel < 2) {
		audio_config.nb_channel = 2;
		std::cout << "LP_alsa::set_hw_params(): nb_channel fixed to 2\n";
	}
	if((err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, audio_config.nb_channel)) < 0) {
		std::cerr << "LP_alsa::set_hw_params(): failed to set nb_channel: " << audio_config.nb_channel << std::endl;
		return err;
	}

	/* sample rate */
	dir = 0;
	if((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &audio_config.rate, &dir)) < 0) {
		std::cerr << "LP_alsa::set_hw_params(): failed to set sample rate: " << audio_config.rate << std::endl;
		return err;
	}

	/* applications des parametres */
	/* apply params */
	if((err = snd_pcm_hw_params(pcm_handle, hw_params)) < 0) {
		std::cerr << "LP_alsa::set_hw_params(): failed to set parameters\n";
		return err;
	}

	/* liberation du pointeur */
	/* free... */
	snd_pcm_hw_params_free(hw_params);
	
	/* OK */
	return 0;
}

/* Parametre logiciels pour alsa */
/* software params for alsa */
int LP_alsa::set_sw_params() {

	/* donnees audio */
	LP_global_audio_data audio_config;

	if((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		std::cerr << "LP_alsa::set_sw_params(): failed to allocate sw_params\n";
		return err;
	}

	if((err = snd_pcm_sw_params_current(pcm_handle, sw_params)) < 0) {
		std::cerr << "LP_alsa::set_sw_params(): failed set current parametres\n";
		return err;
	}

	if((err = snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, audio_config.nb_frames)) < 0){
		std::cerr << "LP_alsa::set_sw_params(): failed to set avail_min\n";
		return err;
	}

	if((err = snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, 0U)) < 0){
		std::cerr << "LP_alsa::set_sw_params(): failed to set start threshold\n";
		return err;
	}

	/* application des parametres */
	if((err = snd_pcm_sw_params(pcm_handle, sw_params)) < 0) {
		std::cerr << "LP_alsa::set_sw_params(): failed to set sw_params\n";
		return err;
	}

	/* liberation du pointeur */
	snd_pcm_sw_params_free(sw_params);

	/* OK */
	return 0;
}

/* lancement thread Alsa */
/* run the alsa_thread */
int lp_alsa_thread_init(pthread_t *thread_id){

	/* On lance un thread */
	int err;

	/* Alsa */
	err = pthread_create(thread_id, 0, lp_alsa_thread, 0);
	if(err != 0) {
		std::cerr << "alsa_thread_init: failed to create thread " << std::endl;
		return err;
	}

	return 0;
}

/* attente fin du thread alsa */
/* wait until thread finish */
int lp_alsa_thread_join(pthread_t thread_id) {

	int err;

	err = pthread_join(thread_id, 0);
	if(err != 0) {
		std::cerr << "alsa_thread_init: failed to join thread" << std::endl;
		return err;
	}

	return 0;
}

/* Fonction appelee dans un thread:
   Merci a : http://www.csd.uwo.ca/courses/CS402b/Assign/faq.c++
*/
/* This function is called by alsa_thread: thanks for the example to
   http://www.csd.uwo.ca/courses/CS402b/Assign/faq.c++
*/
extern "C" void *lp_alsa_thread(void *fake) {
	
	/* fake est NULL */
	/* threaded function need absolutly an argument ?? (I had problems without...) */
	fake = NULL;

	/* Obtention des variables globales */
	/* obtain a global vars pointer */
	LP_global_audio_data audio_data;

	/* Variables internes */
	/* internal vars... */
	snd_pcm_uframes_t frames_to_deliver = 0, frames_readen = 0;
	int err;
	
	/* on indique qu'on est pret */
	/* We are ready */
	audio_data.play_ready = 1;

	/* On attends sur it_ot_buffer */
	/* Must wait until it_ot_buffer thread is ready */
	while(audio_data.it_to_ot_ready == 0) {
		usleep(1000);
	}

	/* Boucle des lecture audio */
	/* read data : FIXME: proper ending of software is not implemented */
	while(0<1) {
		/* On attends que le buffer soit plein */
		/* wait until buffer is full */
		while(audio_data.ot_full != 1) {
			usleep(1000);
		//	printf("ALSA - ATTENTE\n");
		}

		/* Verouillage du mutex et lecture du buffer */
		pthread_mutex_lock(&audio_data.it_ot_buffer_mutex);

		/* frames to read */
		frames_to_deliver = audio_data.ot_frames_to_read;

		//printf("ALSA: recus %d frames, handle: %d\n", frames_to_deliver, audio_data.pcm_handle);

		/* Lecture */
		frames_readen = snd_pcm_writei(audio_data.pcm_handle, audio_data.ot_buffer, frames_to_deliver);


		/* On annonce que le buffer est lu */
		/* frame is empty */
		audio_data.ot_full = 0;

		pthread_mutex_unlock(&audio_data.it_ot_buffer_mutex);

		/* Probleme de lecture */
		/* write problems */
		switch (frames_readen) {
			case -EAGAIN :
					fprintf(stderr, "lp_alsa_thread: ringbuffer is full (-EGAIN signal)\n");
					continue;
					break;

			case -EPIPE :
					fprintf(stderr, "lp_alsa_thread: --- X underrun --- (-EPIPE signal)\n");
					snd_pcm_prepare(audio_data.pcm_handle);
					continue;
					break;

			default :
						//fprintf(stderr, "lp_alsa_thread: unknow transfert error, retval: %d\n", frames_readen);
						//pthread_exit(&frames_readen);
						break;
			//printf("ALSA: %d frames lues\n", frames_readen);
		}

		/* On attends que l'interface soit prete, ou qu'1 seconde se soit eoulee */
		/* Waiting on interface, or until 1 second expierd.. */
		if((err = snd_pcm_wait(audio_data.pcm_handle, 1000)) < 0) {
			fprintf(stderr,"lp_alsaplay: poll failed\n");
		//	break;
		}

	} // boucle 
	// mettre ici les procedures de fin (pthread_exit(0), etc...
	// Here I must implement a proper ending....
}
