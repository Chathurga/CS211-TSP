#include <time.h>

int timer_diff(struct timespec *end, struct timespec *start) {
  int factor = 1000 * 1000;
  
  int ndiff = (end->tv_nsec / factor) - (start->tv_nsec / factor);
  int sdiff = end->tv_sec - start->tv_sec;
  
  return (sdiff * 1000) + ndiff;
}

struct timespec * timer_start() {
  struct timespec *start = malloc(sizeof(struct timespec));
  
  clock_gettime(CLOCK_MONOTONIC, start);
  
  return start;
}

int timer_end(struct timespec *start) {
  struct timespec *end = malloc(sizeof(struct timespec));
  
  clock_gettime(CLOCK_MONOTONIC, end);
  
  return timer_diff(end, start);
}
