#ifndef SOLVER_HEADER
#define SOLVER_HEADER

typedef struct {
  int n;       // number of towns
  int cols;    // number of columns
  int *points; // array of vertex numbers
  double *distances;
} Problem;

#endif
