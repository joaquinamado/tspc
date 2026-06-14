#include "../nob.h"
#include "distances.h"
#include "graphs.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

void fillArrayf(float *array, size_t len, float value) {
  for (size_t i = 0; i < len; i++) {
    array[i] = value;
  }
}

void fillArray(int *array, size_t len, int value) {
  for (size_t i = 0; i < len; i++) {
    array[i] = value;
  }
}

Points heldKarp(Graph graph, size_t startIdx) {
  Points route = {0};
  size_t len = graph.nodes.count;
  if (len == 1) {
    if (startIdx == 0) {
      da_append(&route, graph.nodes.items[startIdx]);
    }
    return route;
  }

  size_t NUM_MASKS = 1 << len;

  float dp[NUM_MASKS * len];
  int parent[NUM_MASKS * len];
  fillArrayf(dp, NUM_MASKS * len, INFINITY);
  fillArray(parent, NUM_MASKS * len, -1);

  dp[(1 << startIdx) * len + startIdx] = 0;

  for (size_t mask = 1; mask < NUM_MASKS; mask++) {
    size_t offset = mask * len;
    for (size_t u = 0; u < len; u++) {
      float dpVal = dp[offset + u];
      if (!(mask & (1 << u)) || dpVal == INFINITY)
        continue;
      for (size_t v = 0; v < len; v++) {
        if (mask & (1 << v))
          continue;
        size_t nextOffset = (mask | (1 << v)) * len;
        float newdist;
        if (graph.adj[u * len + v] == 0) {
          // @TODO(joaquin): we assume that distance 0 means
          // that there is no edge. I should change calloc to
          // something else
          newdist = dpVal + INFINITY;
        } else {
          newdist = dpVal + graph.adj[u * len + v];
        }
        if (newdist < dp[nextOffset + v]) {
          dp[nextOffset + v] = newdist;
          parent[nextOffset + v] = u;
        }
      }
    }
  }

  size_t fullMask = NUM_MASKS - 1;
  size_t fullMaskOffset = fullMask * len;
  float bestDist = INFINITY;
  int lastNode = -1;

  for (size_t u = 0; u < len; u++) {
    if (u == startIdx)
      continue;
    float dpVal = dp[fullMaskOffset + u];
    if (dpVal == INFINITY)
      continue;
    if (dpVal < bestDist) {
      bestDist = dpVal;
      lastNode = u;
    }
  }

  size_t mask = fullMask;
  int current = lastNode;
  while (current != -1) {
    da_append(&route, graph.nodes.items[current]);
    int prev = parent[mask * len + current];
    mask ^= 1 << current;
    current = prev;
  }

  Points orderedRoute = {0};
  while (route.count != 0) {
    Point last = da_last(&route);
    da_append(&orderedRoute, last);
    da_remove_unordered(&route, route.count - 1);
  }
  da_free(route);

  return orderedRoute;
}
