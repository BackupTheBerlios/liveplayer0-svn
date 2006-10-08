#include "lp_peackmeter_core.h"

/* constructor */
lp_peackmeter_core::lp_peackmeter_core(QObject *parent, int _nb_channels, int _buf_size)
	: QThread(parent)
{
	// channels
	if((_nb_channels < 1)||(_nb_channels > MAX_CHANNELS)) {
		std::cerr << "lp_peackmeter_core::" << __FUNCTION__ << ": nb_channels is not in range\n\tSet to 1\n";
		nb_channels = 1;
	}else{
		nb_channels = _nb_channels;
	}

	// Default clip hold time
	clip_timeout = 200000;
	// Initialize clip system...
	int i;
	for(i=0; i<nb_channels; i++){
		clip_timer[i].set_timeout(clip_timeout);
		start_clip(i);
	}

	// Memory for buffer
	buffer = 0;
	if(_buf_size <= 0){
		std::cerr << "lp_peackmeter_core::" << __FUNCTION__ << ": buf_size is in bad value!\n";
		return;
	}
	buf_size = _buf_size;
	buffer = new float[buf_size];
	if(buffer == 0) {
		std::cerr << "lp_peackmeter_core::" << __FUNCTION__ << ": cannot allocate memory for buffer\n";
		buffer = 0;
		return;
	}

	mutex.lock();
	end_process = false;
	mutex.unlock();

	std::cout << "lp_peackmeter_core: buf_size = " << buf_size << std::endl;
}

void lp_peackmeter_core::set_range(float min, float max)
{
	range_low = min;
	if(max == 1.0f){
		range_high = max - 0.0001f;
	}else{
		range_high = max;
	}
}

/* destructor */
lp_peackmeter_core::~lp_peackmeter_core()
{
	mutex.lock();
	if(buffer != 0){
		delete[] buffer;
	}
	// Set end_process var to true, wake the thread and wait the end
	end_process = true;
	condition.wakeOne();
	mutex.unlock();
	wait();
}

/* data process function */
int lp_peackmeter_core::run_interlaced_buffer(float *_buffer, int _buf_len)
{
	// this function is allways called, so we do no verification - be safe !!
	// Copy buffer, wake up running thread and return
	QMutexLocker locker(&mutex);
	int i;

	buf_len = _buf_len;
	for(i=0; i<buf_len; i++){
		buffer[i] = _buffer[i];
	}
	if(!isRunning()){
		start(LowPriority);
		condition.wakeOne();
	}else{
		condition.wakeOne();
	}
	// debug
	//std::cout << "Run called... - i= " << i << std::endl;

	return i+1;
}

void lp_peackmeter_core::set_refresh_time(unsigned int u_secs)
{
	send_timer.set_timeout(u_secs);
}

/* Start clip detection */
void lp_peackmeter_core::start_clip(int _channel)
{
	if(_channel >= nb_channels){
		std::cerr << "lp_peackmeter_core::" << __FUNCTION__ << ": channel value is to high\n";
		return;
	}

	clip_timer[_channel].start();
	clip_to_show[_channel] = true;

}

bool lp_peackmeter_core::get_clip(int _channel)
{
	QMutexLocker locker(&mutex);
	if(_channel >= nb_channels){
		std::cerr << "lp_peackmeter_core::" << __FUNCTION__ << ": channel value is to high\n";
		return false;
	}

	if((clip_timer[_channel].is_timeout() == false)||(clip_to_show[_channel] == true)) {
		clip_to_show[_channel] = false;
		return true;
	}else{
		return false;
	}
}

/* This is the core runing in a separated thread */
void lp_peackmeter_core::run()
{
	int i, j;
	send_timer.start();

	while(end_process == false){
	//while(1){
		mutex.lock();
		condition.wait(&mutex);

		//std::cout << "Process data - chels: " << nb_channels << std::endl;
		for(i=0; i<(buf_len/nb_channels); i++){
			for(j=0; j<nb_channels; j++){
				// set negative to positive value
				if(buffer[i*nb_channels+j] < 0){
					buffer[i*nb_channels+j] = -buffer[i*nb_channels+j];
				}

				val[j] = buffer[i*nb_channels+j];

				// Clip detection
				if(val[j] >= range_high){
					val[j] = range_high;
					start_clip(j);
				}

				// Keep peack
				if(val[j] > last_val[j]){
					last_val[j] = val[j];
				}
				if(val[j]<= last_val[j]){
					val[j] = last_val[j];
				}

				// set low val to low range (fix the minimum val)
				if(val[j] < range_low){
					val[j] = range_low;
				}
			}
		}
		mutex.unlock();

		if(send_timer.is_timeout() == true){
			// Send the values
			emit get_val(&val[0]);
			send_timer.start();
		}

		// Clear buffer's peack
		for(j=0; j<nb_channels; j++){
			last_val[j] = 0.0f;
		}
	}
}
