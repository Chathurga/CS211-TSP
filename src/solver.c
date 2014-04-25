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

#define POS(i, j, n) (((i+1) * n) - ((i+1) * (i+2) >> 1) - (n - j))

char *strdup(const char *str) {
  int n = strlen(str) + 1;
  char *dup = malloc(n);
  strcpy(dup, str);
  return dup;
}

Town *tsp_open(char *path, int *n) {
  FILE* file = fopen(path, "r");
  
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
    
    Town town = { atoi(num), atof(x), atof(y) };
    
    (*n)++;
    if ((*n) - 1 % 100 == 0) { // allocate memory every 100 lines
      offset += 100;
      towns = realloc(towns, sizeof(town) * offset);
    }
    
    towns[(*n) - 1] = town;
  }
  
  fclose(file);
  
  return towns;
}

TSP tsp_init(char *name) {
  // Load the TSP file
  int n = 0; // no. of towns
  Town *towns = tsp_open(name, &n);
  
  // The number of unique pairs of towns
  int cols = POS(n-2, n-1, n) - 1;
  
  int *points = malloc(sizeof(int) * cols * 2);
  double *dists = malloc(sizeof(double) * cols);
  
  for (int i = 0; i < n; ++i) {
    if (i >= n - 1) continue;
    
    for (int j = i + 1; j < n; ++j) {
      if (i == j) continue;
      int pos = POS(i, j, n);
      
      dists[pos] = hsine(towns[i].y, towns[i].x, towns[j].y, towns[j].x);
      points[pos * 2] = i;
      points[pos * 2 + 1] = j;
    }
  }
  
  TSP tsp = {n, cols, dists, points};
  
  return tsp;
}

void tsp_cplex_end(TSP tsp, CPLEX cplex, Solution solution) {
  // Close CPLEX and free all memory associated with it
  CPXcloseCPLEX(&cplex.env);
  free(tsp.distances);
  free(tsp.points);
  cycle_free(solution);
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
    int i = tsp.points[pos*2];
    int j = tsp.points[pos*2 + 1];
    
    int first  = (i * (n-1)) + (j-1);
    int second = (j * (n-1)) + i;
    visits[first] = visits[second] = pos;
    rmatval[first] = rmatval[second] = 1;
    
    sprintf(str, "x(%d,%d)", i+1, j+1);
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
  
  // collect all active solution variables
  int count = 0;
  int *vars = malloc(sizeof(int) * tsp.n);
  for (int i = 0; i < tsp.cols; ++i) {
    if (x[i] == 0) continue;
    vars[count++] = i;
    if (count == tsp.n) break; // all active solution vars found
  }
  
  Subtour *subtour;
  while (subtour = next_subtour(tsp, vars)) {
    insert_subtour(subtour, solution.subtours, &(solution.n));
  }
  
  solution.work_time = timer_end(code_start);
  //cycle_free(prev);
  
  return solution;
}

void cplex_constrain(Solution solution, CPLEX cplex) {
  if (solution.n == 0) return;
  
  for (int i = 0, half = solution.n / 2; i < half; i++) {
    Subtour *subtour = solution.subtours[i];
    
    double rhs[1] = {subtour->n - 1};
    char senses[1] = {'L'};
    int rmatbeg[1] = {0};
    
    double *rmatval = malloc(sizeof(double) * subtour->n);
    for (int i = 0; i < subtour->n; i++) {
      rmatval[i] = 1;
    }
    
    char str[16];
    sprintf(str, "pass(%d)", solution.i);
    char *contraints[1] = {strdup(str)};
    
    CPXaddrows(cplex.env, cplex.lp, 0, 1, subtour->n, rhs, senses,
               rmatbeg, subtour->tour, rmatval, NULL, contraints);
  }
}

// Free memory used by a solve cycle
void cycle_free(Solution solution) {
  for (int i = 0; i < solution.n; i++) {
    free(solution.subtours[i]->tour);
  }
  
  free(solution.subtours);
}

// compares two subtours by checking their length
int shortest(const void *a, const void *b) {
  Subtour subtour_a = *((Subtour*)a);
  Subtour subtour_b = *((Subtour*)b);
  
  if (subtour_a.n > subtour_b.n) return  1;
  if (subtour_a.n > subtour_b.n) return -1;
  return 0;
}

Subtour * next_subtour(TSP tsp, int *vars) {
  // find the next subtour 'starting' point
  int start = -1;
  for (int i = 0; i < tsp.n; ++i) {
    if (vars[i] != -1) {
      start = i;
      break;
    }
  }
  
  if (start == -1) return NULL; // all subtours found
  
  Subtour *subtour = malloc(sizeof(Subtour));
  subtour->n = 1;
  subtour->tour = malloc(sizeof(int) * tsp.n);
  subtour->tour[0] = vars[start];
  vars[start] = -1;
  
  while(1) {
    int fst1, fst2, snd1, snd2;
    get_points(fst, tsp.points, subtour->tour[0]);
    get_points(snd, tsp.points, subtour->tour[subtour->n - 1]);
    
    for (int i = start + 1; i < tsp.n; ++i) {
      if (vars[i] == -1) continue;
      
      int cur1, cur2;
      get_points(cur, tsp.points, vars[i]);
      
      int m1 = match_points(cur, fst);
      int m2 = match_points(cur, snd);
      if (!(m1 || m2)) continue;
      
      // this follows on from the last point
      if (m2) {
        subtour->tour[subtour->n] = vars[i];
        snd1 = cur1; snd2 = cur2;
      }
      // before the first point
      else {
        for (int j = subtour->n + 1; j > 0; --j) {
          subtour->tour[j] = subtour->tour[j-1];
        }
        subtour->tour[0] = vars[i];
        fst1 = cur1; fst2 = cur2;
      }
      
      subtour->n++;
      vars[i] = -1;
    }
    
    get_points(fst, tsp.points, subtour->tour[0]);
    get_points(snd, tsp.points, subtour->tour[subtour->n - 1]);
    
    // subtour loop found
    if (match_points(fst, snd)) break;
  }
  
  return subtour;
}

void insert_subtour(Subtour *subtour, Subtour **list, int *n) {
  int inserted = 0;
  for (int i = 0; i < *n; i++) {
    if (subtour->n < list[i]->n) {
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
