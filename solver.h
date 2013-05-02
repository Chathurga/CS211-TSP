#ifndef SOLVER_HEADER
#define SOLVER_HEADER

typedef struct {
  int n;       // number of towns
  int cols;    // number of columns
  int *points; // array of vertex numbers
  double *distances;
} Problem;

typedef struct {
  CPXENVptr env; // CPLEX environment
  CPXLPptr lp;   // problem pointer
  int *status;
} CPLEX;

typedef struct {
  int i;              // pass number
  double distance;    // total distance of subtours
  int work_time;      // how long the non-cplex code took
  int cplex_time;     // how long the cplex code took
  
  int n;              // the number of subtours generated
  Subtour **subtours; // list of subtours sorted by length
} PassOutput;

#endif
