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
  int *tour; // List of towns visited
} Subtour;

// Various info recorded about a TSP solve cycle
typedef struct {
  int i;              // Pass number
  double distance;    // Total distance of subtours
  int work_time;      // How long the non-cplex code took
  int cplex_time;     // How long the cplex code took
  
  int n;              // The number of subtours generated
  Subtour **subtours; // List of subtours sorted by length
} Solution;

CPLEX cplex_start();

Solution cplex_solve(TSP, Solution, CPLEX);

void cplex_constrain(Solution, CPLEX);

void cycle_free(Solution);

int shortest(const void *, const void *);

Subtour *next_subtour(TSP tsp, int *vars);

void insert_subtour(Subtour *, Subtour **, int *);

#endif
