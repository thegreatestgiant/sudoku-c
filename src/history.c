#include "../include/history.h"

#include <stdint.h>
#include <stdlib.h>

#define INITIAL_HISTORY_CAPACITY 8U

void history_init(MoveHistory *history) {
  if (history == NULL) {
    return;
  }

  history->items = NULL;
  history->count = 0;
  history->capacity = 0;
}

/* STUDENT TODO 4: Append one move to the resizable history array. */
int history_push(MoveHistory *history, Move move) {
  if (!history) {
    return 0;
  }
  if (history->count < history->capacity) {
    Move *item = history->items + history->count++;
    *item = move;
    return 1;
  }
  size_t cap = history->capacity;
  int new_cap = cap == 0 ? INITIAL_HISTORY_CAPACITY : cap * 2;
  Move *temp = realloc(history->items, new_cap * sizeof(MoveHistory));
  if (temp == NULL) {
    free(temp);
    return 0;
  }
  history->items = temp;
  history->capacity = new_cap;
  Move *item = history->items + history->count++;
  *item = move;
  return 1;
}

/* STUDENT TODO 4: Remove and return the most recent move. */
int history_pop(MoveHistory *history, Move *result) {
  if (!history || !result || history->count < 1) {
    return 0;
  }
  Move *move = history->items + --history->count;
  *result = *move;
  return 1;
}

void history_clear(MoveHistory *history) {
  if (history == NULL) {
    return;
  }

  history->count = 0;
}

/* STUDENT TODO 4: Release all storage owned by the history. */
void history_destroy(MoveHistory *history) {
  if (!history) {
    return;
  }
  free(history->items);
  history_init(history);
}
