#ifndef TOUR_HEADER
#define TOUR_HEADER

// simple Subtour Struct
// contains a pointer to a list and the length of that list 
typedef struct {
  int n;
  int *tour;
} Subtour;

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
