#include "../nob.h"
#include "graphs.h"
#include <stdlib.h>

void graph_add_node(Graph *g, Point point) {
  da_append(&g->nodes, point);
  size_t n = g->nodes.count;

  float *new_adj = calloc(n * n, sizeof(float));

  for (size_t r = 0; r < n - 1; r++) {
    for (size_t c = 0; c < n - 1; c++) {
      new_adj[r * n + c] = g->adj[r * (n - 1) + c];
    }
  }

  free(g->adj);
  g->adj = new_adj;
}

void graph_free(Graph *g) {

  da_free(g->nodes);
  free(g->adj);

  g->nodes.count = 0;
  g->nodes.capacity = 0;
  g->nodes.items = NULL;
  g->adj = NULL;
}
