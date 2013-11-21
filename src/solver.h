#ifndef SOLVER_HEADER
#define SOLVER_HEADER

struct Town {
  int num;
  float x, y;
};

typedef struct {
  int n;       // number of towns
  int cols;    // number of columns
  double *distances;
  int *points; // array of vertex numbers
} TSP;

// Contains all relevant variables for a CPLEX instance
typedef struct {
  CPXENVptr env; // CPLEX environment
  CPXLPptr lp;   // problem pointer
  int *status;
} CPLEX;

// Various information recorded about a TSP solve cycle
typedef struct {
  int i;              // pass number
  double distance;    // total distance of subtours
  int work_time;      // how long the non-cplex code took
  int cplex_time;     // how long the cplex code took
  
  int n;              // the number of subtours generated
  Subtour **subtours; // list of subtours sorted by length
} Solution;

CPLEX cplex_start();

Solution cplex_pass(TSP, Solution, CPLEX);

void cplex_constrain(Solution, CPLEX);

void cycle_free(Solution);

#endif
