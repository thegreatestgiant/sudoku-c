#include "history.h"

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
  if (history == NULL) {
    return 0;
  }
  if (history->count < history->capacity) {
    Move *item = history->items + history->count++;
    *item = move;
    return 1;
  }
  int cap = history->capacity;
  int new_cap = cap == 0 ? sizeof(Move) : sizeof(Move) * cap * 2;
  Move *new_items = realloc(history->items, new_cap * sizeof(MoveHistory));
  if (new_items == NULL) {
    free(new_items);
    return 0;
  }
  history->items = new_items;
  history->capacity = new_cap;
  Move *item = history->items + history->count++;
  *item = move;
  return 1;
}

int history_pop(MoveHistory *history, Move *result) {
  /* STUDENT TODO 4: Remove and return the most recent move. */
  (void)history;
  (void)result;
  return 0;
}

void history_clear(MoveHistory *history) {
  if (history == NULL) {
    return;
  }

  history->count = 0;
}

void history_destroy(MoveHistory *history) {
  /* STUDENT TODO 4: Release all storage owned by the history. */
  (void)history;
}
