/* This file is legacy, will be refactored */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "distance.c"
#include "tour.h"
#include "solver.h"

struct Town {
  int num;
  float x, y;
};

char *strdup(const char *str) {
  int n = strlen(str) + 1;
  char *dup = malloc(n);
  strcpy(dup, str);
  return dup;
}

struct Town *opentsp(char *path, int *n, int *cols) {
  FILE* file = fopen(path, "r");
  struct Town* towns = NULL;
  int line_size = 100;
  char line[line_size];
  int offset = 0;
  
  while(fgets(line, line_size, file)) {
    char *num, *x, *y;
    // points are in the form "num x y", split the line by spaces
    num = strtok(line, " ");
    x = strtok(NULL, " ");
    y = strtok(NULL, " ");
    
    if (x == NULL || y == NULL) continue; // line doesn't contain a point
    
    struct Town town = { atoi(num), atof(x), atof(y) };
    
    (*n)++;
    if ((*n) - 1 % 100 == 0) { // allocate memory every 100 lines
      offset += 100;
      towns = realloc(towns, sizeof(town) * offset);
    }
    
    towns[(*n) - 1] = town;
  }
  
  *cols = POS((*n)-2, (*n)-1, (*n)) - 1;
  fclose(file);
  
  return towns;
}

double *problem(struct Town* t, int n, int cols, int *points, CPLEX *cplex) {
  // distance matrix
  double *dists = malloc(sizeof(double) * cols);
  
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
    
    if (i >= n - 1) continue; // all points added
    
    for (int j = i + 1; j < n; ++j) {
      if (i == j) continue;
      int pos = POS(i, j, n);
      
      int first  = (i * (n-1)) + (j-1);
      int second = (j * (n-1)) + i;
      visits[first] = visits[second] = pos;
      rmatval[first] = rmatval[second] = 1;
      
      dists[pos] = hsine(t[i].y, t[i].x, t[j].y, t[j].x);
      
      sprintf(str, "x(%d,%d)", i+1, j+1);
      headers[pos] = strdup(str);
      
      lb[pos] = 0;
      ub[pos] = 1;
      ctype[pos] = CPX_BINARY; // set the var to binary (value of 1 or 0)
      
      points[pos * 2] = i;
      points[pos * 2 + 1] = j;
    }
  }
  
  CPXnewcols(cplex->env, cplex->lp, cols, dists, lb, ub, ctype, headers);
  CPXaddrows(cplex->env, cplex->lp, 0, n, n*n-n, rhs, senses, rmatbeg,
             visits, rmatval, NULL, contraints);
  
  return dists;
}