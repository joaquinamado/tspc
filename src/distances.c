#include "distances.h"
#include "raymath.h"

bool is_inside_circle(Point center, Point coords, int radius) {
  int dist = Vector2Distance(center.position, coords.position);
  return dist <= radius;
}
