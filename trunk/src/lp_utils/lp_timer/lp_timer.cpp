#include "lp_timer.h"

int lp_timer::start()
{
	gettimeofday(&start_time, 0);
	us_start_time = ((start_time.tv_sec * 1000000) + start_time.tv_usec);
	timer_started = true;
	return 0;
}

int lp_timer::stop()
{
	if(timer_started != true){
		std::cerr << "lp_timer::" << __FUNCTION__ << ": timer was not started\n";
		return 0;
	}
	gettimeofday(&stop_time, 0);
	us_stop_time = ((stop_time.tv_sec * 1000000) + stop_time.tv_usec);
//	timer_started = false;
	return 0;
}

bool lp_timer::is_timeout()
{
	if(timer_started != true){
		std::cerr << "lp_timer::" << __FUNCTION__ << ": timer was not started\n";
		return 0;
	}
	gettimeofday(&current_time, 0);
	us_current_time = ((current_time.tv_sec * 1000000) + current_time.tv_usec);

	if((us_current_time - us_start_time) >= us_timeout){
		return true;
	}else{
		return false;
	}
}

int lp_timer::set_timeout(unsigned int us_time)
{
	us_timeout = us_time;
	return 0;
}

unsigned int lp_timer::us_start_stop_time()
{
	if(us_start_time > us_stop_time){
		std::cerr << "lp_timer::us_start_stop_time:start time bigger than stop time !\n";
		return 0;
	}
	return (us_stop_time - us_start_time);
}

unsigned int lp_timer::us_instant_time()
{
	if(timer_started != true){
		std::cerr << "lp_timer::" << __FUNCTION__ << ": timer was not started\n";
		return 0;
	}
	gettimeofday(&current_time, 0);
	us_current_time = ((current_time.tv_sec * 1000000) + current_time.tv_usec);
	if(us_start_time > us_current_time){
		std::cerr << "lp_timer::us_start_stop_time: start time bigger than current time !\n";
		return 0;
	}
	return us_current_time - us_start_time;
}
