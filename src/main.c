#include <ilcplex/cplex.h>

#include "timer.c"
#include "tour.c"
#include "solver.c"

int main() {
  struct timespec *start = timer_start();
  
  Problem problem = tsp_init("./data/ire100.tsp");
  CPLEX cplex = cplex_start();
  PassOutput output = cplex_init(problem, cplex);
  
  int presolve = timer_end(start);
  printf("Presolve time: %dms\n\n", presolve);
  printf("No.   Distance   Subtours   Time\n");
  
  while (output.n != 1) {
    cplex_constrain(output, cplex);
    output = cplex_pass(problem, output, cplex);
    
    printf("%3d   %-8.2f   %-8d   %dms\n",
           output.i, output.distance, output.n, output.cplex_time);
  }
  
  tsp_cplex_end(problem, cplex, output);
  
  int total = timer_end(start);
  free(start);
  
  printf("\nSolved! Best Route: %.2f\n", output.distance);
  printf("Solve Time: %dms\n\n", total - presolve);
  printf("Total Time: %dms\n\n", total);
  
  return 0;
}