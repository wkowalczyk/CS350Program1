#include <stdio.h>
#include <sys/time.h>

struct timeval timer;
unsigned int start;
unsigned int stop;

unsigned int currentTime();

//Get the current time and convert to microseconds
unsigned int currentTime(){
	gettimeofday(&timer, NULL);
	return timer.tv_usec+(timer.tv_sec*1000000);
}

void startTiming();
void stopTiming();
unsigned int difference();

void startTiming(){
	gettimeofday(&timer, NULL);
	start = timer.tv_usec+(timer.tv_sec*1000000);
}

void stopTiming(){
	gettimeofday(&timer, NULL);
	stop = timer.tv_usec+(timer.tv_sec*1000000);
}

unsigned int difference(){
	return stop - start;
}
