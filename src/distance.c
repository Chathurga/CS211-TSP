/*
 * Contains methods for calculating the distnace between two points
 */

#include <math.h>

#define M_PI 3.14159265358979323846

static const double R = 6371; // radius of the Earth

double radians(double val) {
  return val * (M_PI / 180.0);
}

// Distance Functions
// Can be used as the function that calculates the distance between towns
// Use hsine if the towns are points on a globe
// Use point_dist if they are just co-ordinates on a simple x/y plane

// Calculates the Haversine distance between two points on a sphere
// http://en.wikipedia.org/wiki/Haversine_formula
// In this case the sphere is presumed to be the Earth
double hsine(double lat1, double lon1, double lat2, double lon2) {
  double dlon = radians(lon2 - lon1);
  double dlat = radians(lat2 - lat1);
  
  double a = pow(sin(dlat * 0.5), 2)
           + cos(radians(lat1)) * cos(radians(lat2))
           * pow(sin(dlon * 0.5),2);
  double c = 2.0 * atan2(sqrt(a), sqrt(1-a));
  
  return R * c;
}

// simple distance between two points
double point_dist(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}
