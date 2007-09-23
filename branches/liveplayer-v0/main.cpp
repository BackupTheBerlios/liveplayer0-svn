/***************************************************************************
 *   Copyright (C) 2007 by Philippe   *
 *   nel230@gmail.ch   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include "lp_audio_stream.h"
#include "lp_libsndfile_in.h"
#include "lp_portaudio_IO.h"

#include <iostream>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>

int main(int argc, char **argv)
{
  lp_libsndfile_in file_in;
  if(argv[1] != 0){
	file_in.new_file(argv[1]);
  //	file_in.new_file(argv[1]);
  }

  long int n_frames = 1024;

  lp_audio_stream as;
  as.init(2, n_frames);

  QMutex sig_mutex;
  QWaitCondition sig_cond;
  int num_players = 1;
  QSemaphore wait_ready(1);

  lp_portaudio_IO aout;
  aout.init(0, 2, 44100, n_frames/2, &as, &sig_cond, &wait_ready, num_players);
  aout.start_stream();

  long int readen_frames = 1;
  while(readen_frames > 0){
	readen_frames = file_in.read_frames(as.pb_il_buffer, as.get_free_aviable_size());
	as.store_interleaved_data(readen_frames);
	//std::cout << "Readen: " << readen_frames << "\n";
	wait_ready.release();
	sig_mutex.lock();
	sig_cond.wait(&sig_mutex);
	std::cout << "-> Ok, working netx\n";
	sig_mutex.unlock();
  }

  return 0;
}
