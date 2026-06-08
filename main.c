#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdlib.h>

#include "nob.h"
#include "src/distances.h"
#include "src/graphs.h"

#define POINT_RADIUS 20
#define EDGE_THICKNESS 2.5
#define POINT_COLOR RED
#define EDGE_COLOR DARKGREEN
#define VISITED_COLOR BLUE

Color all_colors[] = {
    WHITE,  DARKGREEN,  YELLOW, ORANGE,  PINK,  DARKBROWN, RED,
    GREEN,  GOLD,       LIME,   SKYBLUE, BLUE,  DARKBLUE,  PURPLE,
    VIOLET, DARKPURPLE, MAROON, BEIGE,   BROWN, MAGENTA,   RAYWHITE,
};

int main(void) {
  Graph graph = {0};
  int startIdx = -1;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "TSP Test");
  SetTargetFPS(60);
  bool node_mode = true;
  char *text;

  while (!WindowShouldClose()) {

    // Change from/to insert node/edge modes
    if (IsKeyPressed(KEY_R)) {
      node_mode = !node_mode;
    }
    // Free the graph and close the app
    if (IsKeyPressed(KEY_ESCAPE)) {
      da_free(graph.nodes);
      CloseWindow();
      return 0;
    }
    // Free the graph but and keep the app running
    if (IsKeyPressed(KEY_C)) {
      graph_free(&graph);
      startIdx = -1;
    }

    if (node_mode) {
      text = "NODE CREATION MODE";
    } else {
      text = "EDGE CREATION MODE";
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && node_mode) {
      Point point = {.position = GetMousePosition(), .label = -1};

      graph_add_node(&graph, point);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !node_mode) {
      Point point = {.position = GetMousePosition(), .label = -1};

      for (size_t i = 0; i < graph.nodes.count; i++) {
        Point pnt = graph.nodes.items[i];

        if (is_inside_circle(pnt, point, POINT_RADIUS)) {
          if (startIdx != -1) {
            float dist = Vector2Distance(graph.nodes.items[startIdx].position,
                                         graph.nodes.items[i].position);
            graph.adj[startIdx * graph.nodes.count + i] = dist;
            graph.adj[i * graph.nodes.count + startIdx] = dist;
            startIdx = -1;
          } else {
            startIdx = i;
          }
          break;
        }
      }
    }

    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));
    DrawText(TextFormat("%s", text), 5, 5, POINT_RADIUS * 2, WHITE);

    size_t n = graph.nodes.count;
    for (size_t i = 0; i < n; i++) {
      DrawCircleV(graph.nodes.items[i].position, POINT_RADIUS, POINT_COLOR);
      for (size_t j = i + 1; j < n; j++) {
        if (graph.adj[i * n + j] != 0.0f) {
          Point from = graph.nodes.items[i];
          Point to = graph.nodes.items[j];
          DrawLineBezier(from.position, to.position, EDGE_THICKNESS,
                         EDGE_COLOR);
          DrawText(TextFormat("%.2f", graph.adj[i * n + j]),
                   (from.position.x + to.position.x) / 2,
                   (from.position.y + to.position.y) / 2, POINT_RADIUS / 2,
                   WHITE);
        }
      }
    }
    EndDrawing();
  }
  CloseWindow();

  graph_free(&graph);
  startIdx = -1;
  return 0;
}
