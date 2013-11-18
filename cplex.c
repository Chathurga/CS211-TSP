/*
 * Contains functions that interact with IBM's CPLEX to solve the Travelling
 * Salesman Problem using interger programming techniques
 */

// Starts the CPLEX environment
CPLEX * cplex_start() {
  int status;
  CPXENVptr env = CPXopenCPLEX(&status);
  
  // disable screen output and data consistency checking for speed
  CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_OFF);
  CPXsetintparam(env, CPX_PARAM_DATACHECK, CPX_OFF);
  
  CPXLPptr lp = CPXcreateprob(env, &status, "tsp");
  CPXchgprobtype(env, lp, CPXPROB_MILP); // mixed integer problem
  CPXchgobjsen(env, lp, CPX_MIN); // objective is minimization
  
  CPLEX *cplex = malloc(sizeof(CPLEX));
  cplex->env = env;
  cplex->lp = lp;
  cplex->status = &status;
  
  return cplex;
}

PassOutput * cplex_pass(Problem *problem, PassOutput *prev, CPLEX *clpex) {
  int first_run = (prev == NULL);
  double distance;
  double *x = malloc(problem->cols * sizeof(double)); // solution vars
  
  PassOutput *output = malloc(sizeof(PassOutput));
  output->i = (first_run) ? 1 : prev->i + 1;
  output->n = 0;
  output->subtours = malloc(sizeof(Subtour *) * (problem->n / 3));
  
  struct timespec *cplex_start = timer_start();
  // solve the next cycle and get the solution variables
  CPXmipopt(clpex->env, clpex->lp);
  CPXsolution(clpex->env, clpex->lp, NULL, &(output->distance), x
             , NULL, NULL, NULL);
  output->cplex_time = timer_end(cplex_start);
  
  struct timespec *code_start = timer_start();
  // collect all active solution variables
  int count = 0;
  int *vars = malloc(sizeof(int) * problem->n);
  for (int i = 0; i < problem->cols; ++i) {
    if (x[i] == 0) continue;
    vars[count++] = i;
    if (count == problem->n) break;
  }
  
  Subtour *subtour;
  while (subtour = next_subtour(problem, vars)) {
    insert_subtour(subtour, output->subtours, &(output->n));
  }
  
  output->work_time = timer_end(code_start);
  return output;
}

void cplex_constrain(PassOutput *output, CPLEX *cplex) {
  for (int i = 0, half = output->n / 2; i < half; i++) {
    Subtour *subtour = output->subtours[i];
    
    double rhs[1] = {subtour->n - 1};
    char senses[1] = {'L'};
    int rmatbeg[1] = {0};
    
    double *rmatval = malloc(sizeof(double) * subtour->n);
    for (int i = 0; i < subtour->n; i++) {
      rmatval[i] = 1;
    }
    
    char str[16];
    sprintf(str, "pass(%d)", output->i);
    char *contraints[1] = {strdup(str)};
    
    CPXaddrows(cplex->env, cplex->lp, 0, 1, subtour->n, rhs, senses,
               rmatbeg, subtour->tour, rmatval, NULL, contraints);
  }
}