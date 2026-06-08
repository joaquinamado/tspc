#ifndef DISTANCES_H_
#define DISTANCES_H_

#include "raylib.h"
#include <stddef.h>

typedef struct {
  Vector2 position;
  int label;
} Point;

typedef struct {
  Point *items;
  size_t count;
  size_t capacity;
} Points;

bool is_inside_circle(Point center, Point coords, int radius);

#endif // DISTANCES_H_
