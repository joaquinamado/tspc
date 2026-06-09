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

typedef struct {
  bool isPlaying;
  char *selectedAlg;
} State;

char *all_algorithms[] = {
    "HELD-KARP",
    "BRUTE-FORCE",
};

void DrawAlgorithmsButtons(State *state) {
  size_t len = ARRAY_LEN(all_algorithms);
  for (size_t i = 0; i < len; i++) {
    bool isSelected =
        state->selectedAlg && sv_eq(sv_from_cstr(state->selectedAlg),
                                    sv_from_cstr(all_algorithms[i]));
    Color buttonColor = BLUE;

    if (isSelected) {
      buttonColor = BLACK;
    }

    DrawRectangle(30, 60 + i * 40, 200, 20, buttonColor);
    DrawText(TextFormat("%s", all_algorithms[i]), 50, 60 + i * 40, POINT_RADIUS,
             WHITE);
    Rectangle touchArea = {30, 60 + i * 40, 200, 20};
    bool isValidPosition =
        CheckCollisionPointRec(GetMousePosition(), touchArea);

    if (isValidPosition && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (!isSelected) {
        state->selectedAlg = all_algorithms[i];
      } else {
        state->selectedAlg = NULL;
      }
    }
  }

  DrawRectangle(30, 60 + len * 40, 200, 20, ORANGE);
  DrawText(TextFormat("%s", "PLAY"), 50, 60 + len * 40, POINT_RADIUS, WHITE);
}

int main(void) {
  Graph graph = {0};
  State state = {0};
  int startIdx = -1;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "TSP Test");
  SetTargetFPS(60);
  bool node_mode = true;
  char *text;

  while (!WindowShouldClose()) {

    int width = GetScreenWidth();
    int height = GetScreenHeight();

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

    Rectangle touchArea = {width * 0.3, 0, width * 0.7, height};
    bool isValidPosition =
        CheckCollisionPointRec(GetMousePosition(), touchArea);

    if (isValidPosition && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        node_mode) {
      Point point = {.position = GetMousePosition(), .label = -1};

      graph_add_node(&graph, point);
    }

    if (isValidPosition && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        !node_mode) {
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
    DrawRectangle(0, 0, width * 0.3, height, GetColor(0x282828FF));
    DrawText(TextFormat("%s", text), 5, 5, POINT_RADIUS * 2, WHITE);
    DrawAlgorithmsButtons(&state);

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
