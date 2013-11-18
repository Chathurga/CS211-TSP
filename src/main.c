#include <ilcplex/cplex.h>

#include "timer.c"
#include "tour.c"
#include "solver.c"

int main() {
  struct timespec *start = timer_start();
  
  CPLEX *cplex = cplex_start();
  Problem *problem = init_problem("./data/ire100.tsp", cplex);
  
  int presolve = timer_end(start);
  printf("Presolve time: %dms\n\n", presolve);
  printf("No.   Distance   Subtours   Time\n");
  
  PassOutput *output = NULL;
  while (1) {
    output = cplex_pass(problem, output, cplex);
    
    printf("%3d   %-8.2f   %-8d   %dms\n",
           output->i, output->distance, output->n, output->cplex_time);
    
    if (output->n == 1) break;
    
    cplex_constrain(output, cplex);
  }
  
  int total = timer_end(start);
  
  printf("\nSolved! Best Route: %.2f\n", output->distance);
  printf("Solve Time: %dms\n\n", total - presolve);
  printf("Total Time: %dms\n\n", total);
  
  return 0;
}