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
#include "LP_global_var.h"
#include <samplerate.h>

/* Les players individuels seront issu de cette classe */
class LP_player {

	public:
		LP_player();
		~LP_player();
		int test_LP(int d);
		int get_file(char *file);
		int player_ID;
		int player_ready;
		SNDFILE *snd_fd;		// descripteur fichier (libsndfile)
		int rd_size;
		float *rd_buffer;
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
	private:
		static const int nb_channel;	// Nb de cannaux du fichier, fixe a 2
		unsigned int rate;		// sample rate du fichier
		SF_INFO *audio_info;		// Infos fournis par libsndfile
};


/* lance un nouveau player et donne un objet LP_player */
int lp_player_thread_init(LP_player *player, int player_ID, pthread_t *thread_id);
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

/* Fonction de nettoyage des buffers de bus */
int clear_bus_buffers(int bus);

/* Fonction de nettoyage de mixed_buffer */
int clear_mixed_buffer();

#endif //#ifndef LP_AUDIOFILE_H
