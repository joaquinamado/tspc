#include "distances.h"
#include "graphs.h"
#include "raymath.h"
#include <stddef.h>
Points *heldKarp(Graph graph) {
  float g[graph.nodes.count];
  for (size_t i = 1; i < graph.nodes.count; i++) {
    g[i] = Vector2Distance(graph.nodes.items[0].position,
                           graph.nodes.items[i].position);
  }
}
