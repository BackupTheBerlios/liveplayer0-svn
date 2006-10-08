#ifndef LP_TIMER_H
#define LP_TIMER_H

#include <iostream>
#include <sys/time.h>

class lp_timer
{
public:

	int start();
	int stop();
	int set_timeout(unsigned int us_time);	// set the timeout in microseconds
	bool is_timeout();
	unsigned int us_instant_time();		// returns the elapsed time in microseconds between start and now
	unsigned int us_start_stop_time();	// returns the elapsed between start and stop time in microseconds
private:
	unsigned int us_timeout;		// timeout in microseconds
	struct timeval start_time;
	unsigned int us_start_time;
	struct timeval stop_time;
	unsigned int us_stop_time;
	struct timeval current_time;
	unsigned int us_current_time;
	bool timer_started;
};

#endif
