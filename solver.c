#include "solver.h"
#include "read.c"

Problem * init_problem(char *name, CPLEX *cplex) {
  // load the TSP file
  int n = 0, cols = 0; // n = no. of towns. cols = no. of solution vars
  struct Town *towns = opentsp(name, &n, &cols);
  
  int *points = malloc(sizeof(int) * cols * 2);
  double *distances = problem(towns, n, cols, points, cplex);
  
  Problem *problem = malloc(sizeof(Problem));
  problem->n = n;
  problem->cols = cols;
  problem->points = points;
  problem->distances = distances;
  
  return problem;
}
