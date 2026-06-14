#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nob.h"
#include "src/distances.h"
#include "src/graphs.h"
#include "src/held_karp.h"

#define POINT_RADIUS 20
#define EDGE_THICKNESS 2.5
#define POINT_COLOR RED
#define NODE_START_COLOR BLUE
#define EDGE_COLOR DARKGREEN
#define NOTIFICATION_EXPIRATION_SECONDS 5

typedef struct {
  bool isPlaying;
  char *selectedAlg;
  unsigned char mode;
  const char *notification;
  double notificationExpiresAt;
  int startIdx;
} State;

char *all_algorithms[] = {
    "HELD-KARP",
    "BRUTE-FORCE",
};

void BuildNotification(State *s, const char *notification) {
  s->notification = notification;
  s->notificationExpiresAt = GetTime() + NOTIFICATION_EXPIRATION_SECONDS;
}

void DrawAlgorithmsButtons(State *state, Graph *g) {
  size_t len = ARRAY_LEN(all_algorithms);
  for (size_t i = 0; i < len; i++) {
    bool isSelected =
        state->selectedAlg && sv_eq(sv_from_cstr(state->selectedAlg),
                                    sv_from_cstr(all_algorithms[i]));
    Color buttonColor = BLUE;

    if (isSelected) {
      buttonColor = BLACK;
    }

    DrawRectangle(30, 100 + i * 40, 200, 20, buttonColor);
    DrawText(TextFormat("%s", all_algorithms[i]), 50, 100 + i * 40,
             POINT_RADIUS, WHITE);
    Rectangle touchArea = {30, 100 + i * 40, 200, 20};
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

  Rectangle touchArea = {30, 100 + len * 40, 200, 20};
  bool isValidPosition = CheckCollisionPointRec(GetMousePosition(), touchArea);
  DrawRectangle(30, 100 + len * 40, 200, 20, ORANGE);
  DrawText(TextFormat("%s", state->mode != 2 ? "PLAY" : "STOP"), 50,
           100 + len * 40, POINT_RADIUS, WHITE);
  if (isValidPosition && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (!state->selectedAlg) {
      BuildNotification(state, "Select an algorithm prior to playing");
      return;
    }
    if (state->mode != 2) {
      state->mode = 2;
      state->startIdx = -1;
    } else {
      state->mode = 0;
      g->nodes.items[state->startIdx].color = POINT_COLOR;
      state->startIdx = -1;
    }
  }
}

int main(void) {
  Graph graph = {0};
  State state = {
      .mode = 0,
      .isPlaying = false,
      .selectedAlg = NULL,
      .startIdx = -1,
  };

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "TSP Test");
  SetTargetFPS(60);
  char *text;
  char textBuffer[256];

  while (!WindowShouldClose()) {

    int width = GetScreenWidth();
    int height = GetScreenHeight();

    // Change from/to insert node/edge modes
    if (IsKeyPressed(KEY_R) && state.mode < 2) {
      state.mode = (1 + state.mode) % 2;
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
      state.startIdx = -1;
      state.mode = 0;
      state.isPlaying = false;
      state.selectedAlg = NULL;
      state.notification = NULL;
    }

    switch (state.mode) {
    case 0:
      text = "NODE CREATION MODE";
      break;
    case 1:
      text = "EDGE CREATION MODE";
      break;
    case 2:
      snprintf(textBuffer, sizeof(textBuffer), "PLAYING ALGORITHM \n%s",
               state.selectedAlg);
      text = textBuffer;
      break;
    default:
      printf("Mode (%d) unimplemented\n", state.mode);
      graph_free(&graph);
      return 1;
    }

    if (state.notification && GetTime() <= state.notificationExpiresAt) {
      size_t len = strlen(state.notification);
      size_t textsize = 10;
      DrawRectangle(width - 250, height * 0.95, len * textsize, 20, RED);
      DrawText(TextFormat("%s", state.notification), width - 250, height * 0.95,
               textsize, WHITE);
    }

    Rectangle touchArea = {width * 0.3, 0, width * 0.7, height};
    bool isValidPosition =
        CheckCollisionPointRec(GetMousePosition(), touchArea);

    if (isValidPosition && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        state.mode == 0) {
      Point point = {.position = GetMousePosition(), .color = POINT_COLOR};

      graph_add_node(&graph, point);
    }

    if (isValidPosition && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        state.mode == 1) {
      Point point = {.position = GetMousePosition(), .color = POINT_COLOR};

      for (size_t i = 0; i < graph.nodes.count; i++) {
        Point pnt = graph.nodes.items[i];

        if (is_inside_circle(pnt, point, POINT_RADIUS)) {
          if (state.startIdx != -1) {
            float dist =
                Vector2Distance(graph.nodes.items[state.startIdx].position,
                                graph.nodes.items[i].position);
            graph.adj[state.startIdx * graph.nodes.count + i] = dist;
            graph.adj[i * graph.nodes.count + state.startIdx] = dist;
            state.startIdx = -1;
          } else {
            state.startIdx = i;
          }
          break;
        }
      }
    }

    if (state.mode == 2 && state.startIdx == -1 && isValidPosition &&
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Point point = {.position = GetMousePosition(), .color = POINT_COLOR};

      for (size_t i = 0; i < graph.nodes.count; i++) {
        Point pnt = graph.nodes.items[i];

        if (is_inside_circle(pnt, point, POINT_RADIUS)) {
          graph.nodes.items[i].color = NODE_START_COLOR;
          state.startIdx = i;
          Points route = heldKarp(graph, state.startIdx);
          if (route.count == 0) {
            BuildNotification(&state,
                              "Unable to find a TSP for the given graph");
          }
          for (size_t i = 0; i < route.count; i++) {
            Point pnt = route.items[i];
            printf("ROUTE N = %lu \n", i);
            printf("POS X = %f \n", pnt.position.x);
            printf("POS Y = %f \n", pnt.position.y);
            printf("====================\n");
          }
          break;
        }
      }
    }

    BeginDrawing();
    ClearBackground(GetColor(0x181818FF));
    DrawRectangle(0, 0, width * 0.3, height, GetColor(0x282828FF));
    DrawText(TextFormat("%s", text), 5, 5, POINT_RADIUS * 2, WHITE);
    DrawAlgorithmsButtons(&state, &graph);

    size_t n = graph.nodes.count;
    for (size_t i = 0; i < n; i++) {
      DrawCircleV(graph.nodes.items[i].position, POINT_RADIUS,
                  graph.nodes.items[i].color);
      DrawText(TextFormat("(%.2f, %.2f)", graph.nodes.items[i].position.x,
                          graph.nodes.items[i].position.y),
               graph.nodes.items[i].position.x, graph.nodes.items[i].position.y,
               POINT_RADIUS / 2, WHITE);
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

  return 0;
}
