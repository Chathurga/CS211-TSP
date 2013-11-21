#include "tour.h"
#include "solver.h"

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