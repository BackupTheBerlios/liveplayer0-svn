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
/* Declaration des variables globales pour audioplayer */
#ifndef LP_GLOBAL_VAR_H
#define LP_GLOBAL_VAR_H

#include "LP_audiofile.h"
#include "LP_alsaout.h"

/* here are the global vars.
   For the moment some vars are not used, I must look this....
*/

/* Structure d'acces aux controles audio, parametre globaux, etc... */
class LP_global_audio_data {
	
	public:
	/* Initialisation donnees audio globales */
	int lp_audio_global_init();
	int lp_audio_global_alloc();
	int lp_audio_global_close();
//	LP_global_audio_data();
//	~LP_global_audio_data();

		static int hd_size;				// File read buffer factor
		static pthread_mutex_t bus_buffer_mutex;	// bus buffers mutex
		static int it_size;
		static pthread_t it_ot_buffer;			// Thread it_to_ot_buffer
		static pthread_t th_play;			// Thread de lecture
		static pthread_mutex_t it_ot_buffer_mutex;	// Pour sychroniser les threads it_to_ot_buffer() et la sortie audio (lp_alsa_play() pour le moment)
		static int ot_size;				// La taille du buffer transmis entre read_to_buffer() et la fonction de sortie audio
		static int ot_full;				// On met a 1 pour dire que ot_buffer est pret
		static int ot_frames_to_read;			// Nbre de frames pretes dans ot_buffer
		static float *ot_buffer;			// buffer de sortie de it_to_ot_buffer() (OuTput)
		static int nb_channel;				// Nbre de cannaux sortie audio
		static unsigned int rate;
		static int sample_format;
		static snd_pcm_uframes_t nb_frames;
		static int it_to_ot_ready;			// si it_to_ot_buffer est pret elle vaut 1 (problème lors du lancement des threads)
		static int play_ready;				// si play est pret elle vaut 1 (problème lors du lancement des threads)
		static snd_pcm_t *pcm_handle;			// Le handle pour alsa
		static int rd_to_buf_ready;			// si ot_to_buffer est pret elle vaut 1 (problème lors du lancement des threads)
		static int init_ok;
		/* section mixage / routage */
		static int bus_buf_size;			// Taille des buffer de bus
		static float *bus_buffer_1;			// Le buffer du bus 1
		static float *bus_buffer_2;
		static float *bus_buffer_3;
		static float *bus_buffer_4;
		static int bus_out[5];				// sortie bus
		static int nb_players;				// Nombre de players (surtout utile pour fixer le volume avant mixage)
		static int play_buf_full[10];			// Identifiant player (instance read_to_buffer pour le moment)
		static int player_mode[10];			// mode: 0 - rien, 1: play, 2: pause
		static float *mixed_buffer;			// Buffer pret, c'est lui qui sera transmis a it_buffer)
};


#endif
