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

#include "LP_global_var.h"


/* Initialisation donnees audio globales */
int LP_global_audio_data::lp_audio_global_init() {

	/* Donnes audio initiales */
	/* Give the player audio setup here */
	rate = 44100;
	nb_channel = 8;				// soundcard output channels
	sample_format = SND_PCM_FORMAT_FLOAT;
	nb_players = 2;				// numbers of players
	nb_frames = 1024;
	/* Give the file read buffer size, it's a factor of internal buffer (2x seems a good factor) */
	hd_size = 2;	// On definis x * it_size pour le buffer disque
	/* output routing (multitrack soundcard) */
	bus_out[1] = 1;	// output 1 et 2
	bus_out[2] = 3;	// output 3 et 4
	bus_out[3] = 5;	// output 5 et 6
	bus_out[4] = 7;	// output 7 et 8

	return 0;
}

/* Allocation memoire: doit etre lance apres alsa_init (nb_channel peut changer !) */
/* Allocate globals memory. Muste be run After alsa_init ! */
int LP_global_audio_data::lp_audio_global_alloc() {

	/* Taille des buffers */
	ot_size = nb_frames * nb_channel;
	it_size = nb_frames * nb_channel * hd_size;
	bus_buf_size = nb_frames * 2 * hd_size;

	/* Verification que les tailles soient definies */
	if(ot_size < 1) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: ot_size is not defined\n";
		return -1;
	}
	if(it_size < 1) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: it_size is not defined\n";
		return -1;
	}
	if(bus_buf_size < 1) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: bus_buf_size is not defined\n";
		return -1;
	}

	std::cout << "global_alloc: ot_size: " << ot_size << ", it_size: " << it_size << ", bus_buf_size: " << bus_buf_size << std::endl;
	float latence = (nb_frames * hd_size *1000 * 2) / rate;
	std::cout << "*** " << nb_frames << " frames, tampon disque: " << hd_size << "x (" << nb_frames * 2 * hd_size * sizeof(float) << " o), rate: " << rate << "Hz, 2 periodes ***\n";
	std::cout << "Latence: " << latence << "ms" << std::endl;


	/* Allocations memoire */
	//ot_buffer = (float *)malloc(ot_size * sizeof(float));
	ot_buffer = new float[ot_size];
	if(ot_buffer == NULL) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: cannot allocate memory for 'ot_buffer'\n";
		return -1;
	}
	//bus_buffer_1 = (float *)malloc(bus_buf_size * sizeof(float));
	bus_buffer_1 = new float[bus_buf_size*2];
	if(bus_buffer_1 == NULL) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: connot allocate memory for 'bus_buffer_1'\n";
		return -1;
	}
	//bus_buffer_2 = (float *)malloc(bus_buf_size * sizeof(float));
	bus_buffer_2 = new float[bus_buf_size];
	if(bus_buffer_2 == NULL) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: connot allocate memory for 'bus_buffer_2'\n";
		return -1;
	}
	//bus_buffer_3 = (float *)malloc(bus_buf_size * sizeof(float));
	bus_buffer_3 = new float[bus_buf_size];
	if(bus_buffer_3 == NULL) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: connot allocate memory for 'bus_buffer_3'\n";
		return -1;
	}
	//bus_buffer_4 = (float *)malloc(bus_buf_size * sizeof(float));
	bus_buffer_4 = new float[bus_buf_size];
	if(bus_buffer_4 == NULL) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: connot allocate memory for 'bus_buffer_4'\n";
		return -1;
	}

	//mixed_buffer = (float *) malloc(it_size * sizeof(float));
	mixed_buffer = new float[it_size];
	if(mixed_buffer == NULL) {
		std::cerr << "LP_global_audio_data::lp_audio_global_alloc: connot allocate memory for 'mixed_buffer'\n";
		return -1;
	}
std::cout << "Global alloc: OK\n";
	return 0;
}

/* fermeture et liberations */
int LP_global_audio_data::lp_audio_global_close() {

	/* On libere la memoire */
	delete[] ot_buffer;
	delete[] bus_buffer_1;
	delete[] bus_buffer_2;
	delete[] bus_buffer_3;
	delete[] bus_buffer_4;
	delete[] mixed_buffer;

	return 0;
}

/* Definition des variables audio globales */
/* Global vars */
int LP_global_audio_data::hd_size;				// Taille tampon disque, facteur (nbre de x)
pthread_mutex_t LP_global_audio_data::bus_buffer_mutex;		// Mutex d'ecriture sur les bus
int LP_global_audio_data::it_size;
pthread_t LP_global_audio_data::it_ot_buffer;			// Thread it_to_ot_buffer
pthread_t LP_global_audio_data::th_play;			// Thread de lecture
pthread_mutex_t LP_global_audio_data::it_ot_buffer_mutex;	// Pour sychroniser les threads it_to_ot_buffer() et la sortie audio (lp_alsa_play() pour le moment)
int LP_global_audio_data::ot_size;				// La taille du buffer transmis entre read_to_buffer() et la fonction de sortie audio
int LP_global_audio_data::ot_full;				// On met a 1 pour dire que ot_buffer est pret
int LP_global_audio_data::ot_frames_to_read;			// Nbre de frames pretes dans ot_buffer
float *LP_global_audio_data::ot_buffer;				// buffer de sortie de it_to_ot_buffer() (OuTput)
int LP_global_audio_data::nb_channel;				// Nbre de cannaux sortie audio
unsigned int LP_global_audio_data::rate;
int LP_global_audio_data::sample_format;
snd_pcm_uframes_t LP_global_audio_data::nb_frames;
int LP_global_audio_data::it_to_ot_ready;			// si it_to_ot_buffer est pret elle vaut 1 (problème lors du lancement des threads)
int LP_global_audio_data::play_ready;				// si play est pret elle vaut 1 (problème lors du lancement des threads)
snd_pcm_t *LP_global_audio_data::pcm_handle;			// Le handle pour alsa
int LP_global_audio_data::rd_to_buf_ready;			// si ot_to_buffer est pret elle vaut 1 (problème lors du lancement des threads)
int LP_global_audio_data::init_ok;
/* section mixage / routage */
int LP_global_audio_data::bus_buf_size;				// Taille des buffer de bus
float *LP_global_audio_data::bus_buffer_1;			// Le buffer du bus 1
float *LP_global_audio_data::bus_buffer_2;
float *LP_global_audio_data::bus_buffer_3;
float *LP_global_audio_data::bus_buffer_4;
int LP_global_audio_data::bus_out[5];				// sortie bus
int LP_global_audio_data::nb_players;				// Nombre de players (surtout utile pour fixer le volume avant mixage)
int LP_global_audio_data::play_buf_full[10];			// Identifiant player (instance read_to_buffer pour le moment)
int LP_global_audio_data::player_mode[10];			// Gestion des attentes entre players
float *LP_global_audio_data::mixed_buffer;			// Buffer pret, c'est lui qui sera transmis a it_buffer)
