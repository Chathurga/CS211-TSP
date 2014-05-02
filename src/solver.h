#ifndef SOLVER_HEADER
#define SOLVER_HEADER

// A town node
typedef struct {
  int id;     // The town number from 0
  int num;    // Whatever the town was numbered in the file
  float x, y; // x,y co-ords
} Town;

// Simple tuple that represents a pairing of towns
// A pairing means a journey from i to j or j to i
typedef struct {
  int id;
  Town i, j;
} Pair;

// Info about the current TSP problem
typedef struct {
  int n;             // Number of towns
  int cols;          // Number of unique pairing of towns
  double *distances; // Distance between each pair of towns
  Pair *pairs;       // List of the pairs of towns
} TSP;

// CPLEX instance vars
typedef struct {
  CPXENVptr env; // CPLEX environment
  CPXLPptr lp;   // Problem pointer
  int *status;
} CPLEX;

// Subtour wrapper
typedef struct {
  int n;     // Number of towns in subtour
  Pair *tour; // List of towns visited
} Subtour;

// Various info recorded about a TSP solve cycle
typedef struct {
  int i;             // Pass number
  double distance;   // Total distance of subtours
  int work_time;     // How long the non-cplex code took
  int cplex_time;    // How long the cplex code took
  
  int n;             // The number of subtours generated
  Subtour *subtours; // List of subtours sorted by length
} Solution;

CPLEX cplex_start();

Solution cplex_solve(TSP, Solution, CPLEX);

void cplex_constrain(Solution, CPLEX);

void cycle_free(Solution);

int shortest(const void *, const void *);

int pair_pos(int, int, int);

int subtour_exists(TSP tsp, int *vars);
Subtour subtour_get(TSP tsp, int *vars);
void subtour_insert(Subtour, Subtour *, int *);

#endif
