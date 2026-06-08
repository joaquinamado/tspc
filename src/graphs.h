#ifndef GRAPHS_H_
#define GRAPHS_H_

#include "distances.h"

typedef struct {
  size_t from;
  size_t to;
  float weight;
} Edge;

typedef struct {
  Edge *items;
  size_t count;
  size_t capacity;
} Edges;

typedef struct {
  Points nodes;
//  Edges edges;
  float *adj;
} Graph;

void graph_add_node(Graph *g, Point point);
void graph_free(Graph *g);

#endif // GRAPHS_H_
