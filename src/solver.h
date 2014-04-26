#ifndef SOLVER_HEADER
#define SOLVER_HEADER

typedef struct {
  int id, num;
  float x, y;
} Town;

typedef struct {
  Town i, j;
} Pair;

typedef struct {
  int n;       // number of towns
  int cols;    // number of columns
  double *distances;
  Pair *pairs; // each potential pairing of towns
} TSP;

// Contains all relevant variables for a CPLEX instance
typedef struct {
  CPXENVptr env; // CPLEX environment
  CPXLPptr lp;   // problem pointer
  int *status;
} CPLEX;

// Contains the length of a subtour and a list of towns
typedef struct {
  int n;
  int *tour;
} Subtour;

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

Solution cplex_solve(TSP, Solution, CPLEX);

void cplex_constrain(Solution, CPLEX);

void cycle_free(Solution);

int shortest(const void *, const void *);

Subtour *next_subtour(TSP tsp, int *vars);

void insert_subtour(Subtour *, Subtour **, int *);

#endif
