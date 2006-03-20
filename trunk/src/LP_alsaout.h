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
#ifndef LP_ALSAOUT_H
#define LP_ALSAOUT_H

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <iostream>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "LP_global_var.h"


/* Defini la classe LP_alsa */
class LP_alsa {

	public:
		//LP_alsa();
		//~LP_alsa();
		int lp_alsa_init();
		int lp_alsa_close();
		unsigned short int lp_alsa_playing;	// OK, lecteur lance
		int nb_max_channel;	// nombre maximum de la carte son

	private:
		snd_output_t *output;	// sortie infos
		int err;
		snd_pcm_sframes_t frame_readen, frames_to_deliver;
		snd_pcm_t *pcm_handle;
		snd_pcm_hw_params_t *hw_params;
		snd_pcm_sw_params_t *sw_params;
		snd_pcm_format_t pcm_format;
		int dir;		// Pour approcher fech
		int test_hw_params();
		int set_hw_params();
		int set_sw_params();
		int lp_alsa_stop;	// status stop = 1
		pthread_t thread_id;
};

/* lancement thread Alsa */
/* run the alsa thread */
int lp_alsa_thread_init(pthread_t *thread_id);

/* attente fin thread Alsa */
/* wait the end of alsa thread and quit */
int lp_alsa_thread_join(pthread_t thread_id);

/* Fonction appelee dans un thread:
   Merci a : http://www.csd.uwo.ca/courses/CS402b/Assign/faq.c++
*/
/* alsa output called in the thread */
extern "C" void *lp_alsa_thread(void *fake);


#endif // #ifndef LP_ALSAOUT_H
