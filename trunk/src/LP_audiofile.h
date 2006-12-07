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

#ifndef LP_AUDIOFILE_H
#define LP_AUDIOFILE_H

/* Fonctions pour LP_audiofile.cpp */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <sndfile.h>
#include <soundtouch/SoundTouch.h>
/* Utilities */
#include "LP_global_var.h"

#include "lp_peackmeter/lp_peackmeter.h"

#include "lp_ladspa/lp_ladspa_manager.h"

/* Play mode */
#define LP_PLAY_MODE_PLAYING	1
#define LP_PLAY_MODE_PAUSE 	2

/* Play direction */
#define LP_PLAY_FORWARD		1
#define LP_PLAY_REVERSE		2

/* Switchs */
#define LP_OFF 			0
#define LP_ON			1

/* Read libs */
#define LP_LIB_SNDFILE		1
#define LP_LIB_VORBIS		2
#define LP_LIB_MAD		3


/* Les players individuels seront issu de cette classe */
class LP_player {

	public:
		LP_player(int player_ID );
		~LP_player();
		/// ladspa
		lp_ladspa_manager *ladspa;
		// Enable / Disable SoundTouch processing
		int setSoundTouch(int state);	// can be LP_ON or LP_OFF
		int getSoundTouch();		// Return state (LP_ON or LP_OFF)
		// Speed settings
		int setSpeed(double speed);
		double getSpeed();
		// Play direction settings
		int setDirection(int direction);
		int getDirection();
		// Seek in file: set how many frames and from location you want count
		int setSeek(int frames, int position);
		int mSeekRefPos;		// Start from position: SEEK_SET (beginging of file), SEEK_CUR, SEEK_END
		int mSeekOffset;
		int get_file(char *file);
		/* To read a opened file */
		sf_count_t lp_read(sf_count_t samples);
		sf_count_t lp_seek(sf_count_t samples, int ref_pos);
		SNDFILE *snd_fd;		// descripteur fichier (libsndfile)
		int rd_size;
		float *rd_buffer;
		float *tmp_buffer;		// intermediaire
		float *sampled_buffer;
		int player_ID;
		int player_ready;
		static const int nb_channel;	// Nb de cannaux du fichier, fixe a 2
		/* Recherche les variables changees et reagis
		Si un evenement s'est produit, renvois 1
		Si une erreur est survenue, renvois < 0
		Si rien ne s'est produit, renvois 0
		*/
		int get_event();
		/* Variables parcourues par get_event() */
		char *mfile;	// fichier a lire
		int mbus;	// Bus de sortie
		int mplay_mode;	// 0, rien, 1: play,2: pause

		int volume;

		/// Peack meter
		lp_peackmeter *pv_pm;
		lp_peackmeter_core *pv_pmc;

	private:
		unsigned int rate;		// sample rate du fichier
		SF_INFO *audio_info;		// Infos fournis par libsndfile
		//OggVorbis_File *vf;		// Structure pour vorbisfile
		int vf_current_section;
		FILE *fds;			// file stream opened with fopen
		pthread_t thread_id;		// Thread ID for LP_player instance
		int mSoundTouch;		// Enable / disable SoundTouch processing
		double mSpeed;			// Resampling factor for speed
		int mDirection;			// Play direction
		int mSeekEvent;
		int mRead_lib;			// To now witch lib to use, can be LP_LIB_SNDFILE, LP_LIB_VORBIS or LP_LIB_MAD
};


/* lance un nouveau player et donne un objet LP_player */
//int lp_player_thread_init(LP_player *player, int player_ID, pthread_t *thread_id);
int lp_player_thread_join(pthread_t thread_id);

/* Fonction appelee dans n thread:
   Merci a : http://www.csd.uwo.ca/courses/CS402b/Assign/faq.c++
*/
extern "C" void *lp_player_thread(void *p_data);

/* Thread it_to_ot_buffer */
int lp_it_ot_thread_init(pthread_t *thread_id);
int lp_it_ot_thread_join(pthread_t thread_id);

extern "C" void *lp_it_to_ot_buffer(void *fake);

/* Mixage 
   On donne le buffer a mixer et router, le bus (mixage "virtuel")
   Le buffer doit etre de 2 cannaux (stereo)
*/
int mix_out(float *in_buffer, int in_buf_size, int bus);

/* Cette fonction extrait 2 cannaux d'un buffer:
   Donner le cannal a extraire (1, 3, 5, etc...), elle revois le canal donne et le suivant...
   Il faut aussi preciser le nombre de cannaux que contient le buffer, et sa longueur
*/
int extract_one_buffer(float *in_buffer,int in_buf_len , int in_nb_channels,float *out_buffer, int out_channel);

/* Cette fonction ajoute un buffer sur le buffer de sortie 
   On donne la sortie: 1, 3, 5 ou 7. Le tout travaille en stereo !
   Donc: in_buffer = 2 cannaux - Merci a Joel (un ami) pour la solution !
*/
int add_one_buffer(float *in_buffer, float *out_buffer, int in_buffer_len, int out_channels, int output);

/* Fonction nettoyage d'un buffer */
int clean_buffer(float *buffer, int buf_size);

/* Fonction de nettoyage des buffers de bus */
int clear_bus_buffers(int bus);

/* Fonction de nettoyage de mixed_buffer */
int clear_mixed_buffer();

#endif //#ifndef LP_AUDIOFILE_H
