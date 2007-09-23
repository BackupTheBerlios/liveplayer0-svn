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
