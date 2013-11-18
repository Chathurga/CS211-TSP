/*
 *  Simple timer functions
 */

#include <time.h>

// Starts a timer
struct timespec *timer_start() {
  struct timespec *start = malloc(sizeof(struct timespec));
  
  clock_gettime(CLOCK_MONOTONIC, start);
  
  return start;
}

// Calculates the time elapsed between some timer and now
// Returns the time in milliseconds
int timer_end(struct timespec *start) {
  struct timespec *end = timer_start();
  
  int factor = 1000 * 1000;
  
  int ndiff = (end->tv_nsec / factor) - (start->tv_nsec / factor);
  int sdiff = end->tv_sec - start->tv_sec;
  
  free(end);
  
  return (sdiff * 1000) + ndiff;
}
