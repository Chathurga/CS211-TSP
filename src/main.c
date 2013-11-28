#include <ilcplex/cplex.h>

#include "timer.c"
#include "tour.c"
#include "solver.c"

char *choose_file(char *dir_path) {
  
}

int main() {
  // Ask the user for the TSP problem file name
  char line[255];
  char path[255] = "./data/";
  printf("Enter name of TSP file:\n");
  scanf("%s", line);
  strcat(path, line);
  printf("\n");
  
  struct timespec *start = timer_start();
  TSP tsp = tsp_init(path);
  CPLEX cplex = cplex_start();
  Solution solution = cplex_init(tsp, cplex);
  
  int presolve = timer_end(start);
  printf("Presolve time: %dms\n\n", presolve);
  printf("No.   Distance   Subtours   Time\n");
  
  while (solution.n != 1) {
    cplex_constrain(solution, cplex);
    solution = cplex_solve(tsp, solution, cplex);
    
    printf("%3d   %-8.2f   %-8d   %dms\n",
           solution.i, solution.distance, solution.n, solution.cplex_time);
  }
  
  tsp_cplex_end(tsp, cplex, solution);
  
  int total = timer_end(start);
  free(start);
  
  printf("\nSolved! Best Route: %.2f\n", solution.distance);
  printf("Solve Time: %dms\n\n", total - presolve);
  printf("Total Time: %dms\n\n", total);
  
  return 0;
}