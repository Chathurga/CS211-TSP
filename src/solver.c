/*
 * Contains functions that:
 *   Parse and process a well formatted file detailing a Travelling Salesman
 *   Problem
 *   Interact with IBM's CPLEX to solve the Travelling Salesman Problem using
 *   interger programming techniques
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "solver.h"
#include "distance.c"

char *strdup(const char *str) {
  int n = strlen(str) + 1;
  char *dup = malloc(n);
  strcpy(dup, str);
  return dup;
}

Town *tsp_open(char *path, int *n) {
  FILE *file = fopen(path, "r");
  
  if (file == NULL) {
    printf("Could not open TSP file\n");
    exit(1);
  }
  
  Town* towns = NULL;
  int offset = 0;
  int line_size = 100;
  char line[line_size];
  
  while(fgets(line, line_size, file)) {
    char *num, *x, *y;
    // points are in the form "num x y", split the line by spaces
    num = strtok(line, " ");
    x = strtok(NULL, " ");
    y = strtok(NULL, " ");
    
    if (x == NULL || y == NULL) continue; // line doesn't contain a point
    
    Town town = { *n, atoi(num), atof(x), atof(y) };
    // Expand the list every 100 towns
    if (*n % 100 == 0) {
      offset += 100;
      towns = realloc(towns, sizeof(town) * offset);
    }
    
    towns[*n] = town;
    (*n)++;
  }
  
  fclose(file);
  
  return towns;
}

TSP tsp_init(char *name) {
  // Load the TSP file
  int n = 0; // no. of towns
  Town *towns = tsp_open(name, &n);
  
  // The number of unique pairs of towns
  int npairs = (n * (n - 1)) / 2;
  
  Pair *pairs = malloc(sizeof(Pair) * npairs);
  double *dists = malloc(sizeof(double) * npairs);
  
  for (int i = 0; i < n; ++i) {
    if (i >= n - 1) continue;
    
    for (int j = i + 1; j < n; ++j) {
      if (i == j) continue;
      int pos = pair_pos(i, j, n);
      
      dists[pos] = hsine(towns[i].y, towns[i].x, towns[j].y, towns[j].x);
      Pair pair = { pos, towns[i], towns[j] };
      pairs[pos] = pair;
    }
  }
  
  TSP tsp = {n, npairs, dists, pairs};
  
  return tsp;
}

void tsp_cplex_end(TSP tsp, CPLEX cplex, Solution solution) {
  // Close CPLEX and free all memory associated with it
  CPXcloseCPLEX(&cplex.env);
  free(tsp.distances);
  free(tsp.pairs);
}

// Starts the CPLEX environment
CPLEX cplex_start() {
  int status;
  CPXENVptr env = CPXopenCPLEX(&status);
  
  // disable screen solution and data consistency checking for speed
  CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
  CPXsetintparam(env, CPX_PARAM_DATACHECK, CPX_OFF);
  
  CPXLPptr lp = CPXcreateprob(env, &status, "tsp");
  CPXchgprobtype(env, lp, CPXPROB_MILP); // mixed integer problem
  CPXchgobjsen(env, lp, CPX_MIN); // objective is minimization
  
  //CPXwriteprob(env, lp, "problem.lp", "LP");
  
  CPLEX cplex = {env, lp, &status};
  
  return cplex;
}

Solution cplex_init(TSP tsp, CPLEX cplex) {
  int n = tsp.n;
  int cols = tsp.cols;
  double *dists = tsp.distances;
  
  int *visits = malloc(sizeof(int) * n * n - n);
  double *rmatval = malloc(sizeof(double) * n * n - n);
  
  char **headers = malloc(sizeof(char *) * cols);
  double *lb = malloc(sizeof(double) * cols);
  double *ub = malloc(sizeof(double) * cols);
  char *ctype = malloc(sizeof(char) * cols);
  
  char **contraints = malloc(sizeof(char *) * n);
  double *rhs = malloc(sizeof(double) * n);
  char *senses = malloc(sizeof(char) * n);
  int *rmatbeg = malloc(sizeof(int) * n);
  
  char str[16];
  
  for (int i = 0; i < n; ++i) {
    sprintf(str, "visit(%d)", i+1);
    contraints[i] = strdup(str);
    rhs[i] = 2;
    senses[i] = 'E';
    rmatbeg[i] = i * (n-1);
  }
  
  for (int pos = 0; pos < cols; ++pos) {
    Pair pair = tsp.pairs[pos];
    
    int first  = (pair.i.id * (n-1)) + (pair.j.id-1);
    int second = (pair.j.id * (n-1)) + pair.i.id;
    visits[first] = visits[second] = pos;
    rmatval[first] = rmatval[second] = 1;
    
    sprintf(str, "x(%d,%d)", pair.i.num, pair.j.num);
    headers[pos] = strdup(str);
    
    lb[pos] = 0;
    ub[pos] = 1;
    ctype[pos] = CPX_BINARY; // set the var to binary (value of 1 or 0)
  }
  
  CPXnewcols(cplex.env, cplex.lp, cols, dists, lb, ub, ctype, headers);
  CPXaddrows(cplex.env, cplex.lp, 0, n, n*n-n, rhs, senses, rmatbeg,
             visits, rmatval, NULL, contraints);
  
  Solution solution;
  solution.i = 0;
  solution.n = -1;
  
  return solution;
}

Solution cplex_solve(TSP tsp, Solution prev, CPLEX cplex) {
  double distance;
  double *x = malloc(tsp.cols * sizeof(double));
  
  Solution solution;
  solution.i = prev.i + 1;
  solution.n = 0;
  solution.subtours = malloc(sizeof(Subtour *) * (tsp.n / 3));
  
  struct timespec cplex_start = timer_start();
  CPXmipopt(cplex.env, cplex.lp); // solve the next cycle
  CPXsolution(cplex.env, cplex.lp, NULL, &solution.distance, x,
              NULL, NULL, NULL);
  solution.cplex_time = timer_end(cplex_start);
  
  struct timespec code_start = timer_start();
  
  // Collect all active solution variables
  int count = 0;
  int *vars = malloc(sizeof(int) * tsp.n);
  for (int i = 0; i < tsp.cols; ++i) {
    if (x[i] == 0) continue;
    vars[count++] = i;
    if (count == tsp.n) break; // All active solution vars found
  }
  
  while (subtour_exists(tsp, vars)) {
    subtour_insert(subtour_get(tsp, vars), solution.subtours, &(solution.n));
  }
  
  free(vars);
  solution.work_time = timer_end(code_start);
  
  return solution;
}

void cplex_constrain(Solution solution, CPLEX cplex) {
  if (solution.n == 0) return;
  
  for (int i = 0, half = solution.n / 2; i < half; ++i) {
    Subtour subtour = solution.subtours[i];
    
    double rhs[1] = {subtour.n - 1};
    char senses[1] = {'L'};
    int rmatbeg[1] = {0};
    
    double *rmatval = malloc(sizeof(double) * subtour.n);
    for (int j = 0; j < subtour.n; ++j) {
      rmatval[j] = 1;
    }
    
    char str[16];
    sprintf(str, "pass(%d)", solution.i);
    char *contraints[1] = {strdup(str)};
    
    int *tour = malloc(sizeof(int) * subtour.n);
    for (int j = 0; j < subtour.n; ++j) {
      tour[j] = subtour.tour[j].id;
    }
    
    CPXaddrows(cplex.env, cplex.lp, 0, 1, subtour.n, rhs, senses,
               rmatbeg, tour, rmatval, NULL, contraints);
  }
}

// compares two subtours by checking their length
int shortest(const void *a, const void *b) {
  Subtour subtour_a = *((Subtour*)a);
  Subtour subtour_b = *((Subtour*)b);
  
  if (subtour_a.n > subtour_b.n) return  1;
  if (subtour_a.n > subtour_b.n) return -1;
  return 0;
}

// Pairs are stored in a flat array which represents a triangular matrix
// Get the real position of the (i, j) pair
int pair_pos(int i, int j, int n) {
  return ((i+1) * n) - ((i+1) * (i+2) / 2) - (n - j);
}

// Checks if two pairs of towns are adjacent
// This means that they are next to each other in a tour
int pair_adj(Pair a, Pair b) {
  return a.i.id == b.i.id || a.i.id == b.j.id ||
         a.j.id == b.i.id || a.j.id == b.j.id;
}

// Gets the town that's common to 2 pairs
Town pair_common(Pair a, Pair b) {
  if (a.i.id == b.i.id || a.i.id == b.j.id) return a.i;
  if (a.j.id == b.i.id || a.j.id == b.j.id) return a.j;
}

// Check if there's another subtour that hasn't been detected
// Once a solution var gets used it's to -1, check for this
int subtour_exists(TSP tsp, int *vars) {
  for (int i = 0; i < tsp.n; ++i) {
    if (vars[i] != -1) return 1;
  }
  return 0;
}

Subtour subtour_get(TSP tsp, int *vars) {
  // find the next subtour starting point
  int start = 0;
  for (; start < tsp.n; ++start) {
    if (vars[start] != -1) break;
  }
  
  Pair prev = tsp.pairs[vars[start]];
  Subtour subtour = { 1, NULL };
  subtour.tour = malloc(sizeof(Pair) * tsp.n);
  subtour.tour[0] = prev;
  vars[start] = -1;
  
  while(1) {
    for (int i = start + 1; i < tsp.n; ++i) {
      if (vars[i] == -1) continue; // Already used
      Pair cur = tsp.pairs[vars[i]];
      // Make sure this town follows on from the last town found
      if (!pair_adj(cur, prev)) continue;
      
      prev = subtour.tour[subtour.n++] = cur;
      vars[i] = -1;
      break;
    }
    
    // First and last points are adjacent: subtour complete
    if (subtour.n > 2 && pair_adj(subtour.tour[0], prev)) break;
  }
  
  return subtour;
}

void subtour_insert(Subtour subtour, Subtour *list, int *n) {
  int inserted = 0;
  for (int i = 0; i < *n; i++) {
    if (subtour.n < list[i].n) {
      for (int j = *n; j > i; j--) {
        list[j] = list[j-1];
      }
      
      list[i] = subtour;
      inserted = 1;
      break;
    }
  }
  
  if (!inserted) {
    list[*n] = subtour;
  }
  
  ++(*n);
}

void subtour_print(Subtour subtour) {
  for (int i = 0; i < subtour.n; i++) {
    Town t = pair_common(subtour.tour[i], subtour.tour[(i + 1) % subtour.n]);
    printf("%d", t.num);
    if (i != subtour.n - 1) printf(".");
  }
  printf("\n\n");
}
