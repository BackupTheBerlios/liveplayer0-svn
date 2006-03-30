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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "LP_global_var.h"


using namespace std;

int main(int argc, char *argv[])
{
  int err;
  pthread_t thread_id;

  /* on mets les parametres audio */
  LP_global_audio_data audio_globals;
  audio_globals.lp_audio_global_init();

  /* appel a alsa_init */
  LP_alsa aout;
  aout.lp_alsa_init();

  /* Allocation buffer audio globaux */
  if((err = audio_globals.lp_audio_global_alloc()) < 0) {
	std::cerr << "main: globals memory allocation fails\n";
	return -1;
  }

  lp_it_ot_thread_init(&thread_id);

  LP_player *player_1 = new LP_player(0);
	if(player_1 == 0) { std::cout << "Probleme init player_1\n"; 	return -1;}

  LP_player *player_2 = new LP_player(1);
	if(player_2 == 0) { std::cout << "Probleme init player_2\n"; 	return -1;}


  player_1->mplay_mode = LP_PLAY_MODE_PAUSE;
  player_2->mplay_mode = LP_PLAY_MODE_PAUSE;

  player_1->mbus = 1;

  player_2->mbus = 2;

  player_1->mfile = argv[1];
  player_2->mfile = argv[2];



/* Decompte */
int i;
for (i=2; i>0; i--){
	sleep(1);
	std::cout << "Decompte: " << i << std::endl;
}

  player_1->mplay_mode = LP_PLAY_MODE_PLAYING;	// mode play
  player_2->mplay_mode = LP_PLAY_MODE_PLAYING;

/* Some brutal speed variations.... */
usleep(100000);
player_1->setSpeed(0.9);
usleep(1000000);
player_1->setSpeed(1.2);
player_2->setSpeed(1.2);
usleep(1000000);
player_1->setSpeed(0.9);
usleep(1000000);
player_1->setSpeed(1.05);
usleep(1000000);
player_1->setSpeed(1.1);
usleep(1000000);
player_1->setSpeed(1.0);
player_1->setSpeed(0.5464);
usleep(1000000);
player_1->setSpeed(1.05314);
usleep(1000000);
player_1->setSpeed(1.453431);
player_2->setSpeed(1.0);
usleep(1000000);
player_1->setSpeed(0.9);
usleep(100000);
player_1->setSpeed(0.91);
player_2->setSpeed(1.0);
usleep(100000);
player_1->setSpeed(0.91);
usleep(100000);
player_1->setSpeed(0.93);
usleep(100000);
player_1->setSpeed(0.95);
usleep(100000);
player_1->setSpeed(0.97);
usleep(100000);
player_1->setSpeed(0.99);
usleep(100000);
player_1->setSpeed(1.0);
usleep(100000);
player_1->setSpeed(1.02);
usleep(100000);
player_1->setSpeed(1.04);
usleep(100000);
player_1->setSpeed(1.06);
usleep(100000);
player_1->setSpeed(1.08);
usleep(100000);
player_1->setSpeed(1.09);
usleep(1000000);
player_1->setSpeed(1.1);

/* The destructor waits the end of thread */
delete player_1;
delete player_2;

  lp_it_ot_thread_join(thread_id);

  audio_globals.lp_audio_global_close();
  aout.lp_alsa_close();


  return EXIT_SUCCESS;
}
