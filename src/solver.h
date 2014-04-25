#ifndef SOLVER_HEADER
#define SOLVER_HEADER

typedef struct {
  int num;
  float x, y;
} Town;

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

// fetches the numbers of the two points involved in an edge
// iden: the base variable name, the nums will be put in iden1 and iden2
// ps:   the points array
// i:    index position of the first num, is followed by the second
#define get_points(iden, ps, i) iden ## 1 = (ps)[i*2] + 1;\
                                iden ## 2 = (ps)[i*2+1] + 1;
                               
// checks if any of the point numbers match in 2 pairs
#define match_points(idenA, idenB) idenA ## 1 == idenB ## 1 ||\
                                   idenA ## 1 == idenB ## 2 ||\
                                   idenA ## 2 == idenB ## 1 ||\
                                   idenA ## 2 == idenB ## 2

#endif
