
#include <board.h>
#include <stdlib.h>

int board_coordinates_in_range(int row, int column) {
  return row >= 0 && row < SUDOKU_SIZE && column >= 0 && column < SUDOKU_SIZE;
}

SudokuBoard *board_create(void) {
  /* STUDENT TODO 1: Implement the complete board constructor. */
  SudokuBoard *board;
  board = calloc(1, sizeof(SudokuBoard));
  if (board == NULL) {
    return NULL;
  }
  board->cells = NULL;
  int *cells = calloc(81, sizeof(*cells));
  if (cells == NULL) {
    free(board);
    return NULL;
  }
  board->cells = cells;
  return board;
}

SudokuBoard *board_clone(const SudokuBoard *source) {
  /* STUDENT TODO 3: Return a separate board with independent cell storage. */
  (void)source;
  return NULL;
}

void board_destroy(SudokuBoard **board_ptr) {
  /* STUDENT TODO 1: Release a board and clear the caller's pointer. */
  if (board_ptr != NULL && *board_ptr != NULL) {
    if ((*board_ptr)->cells != NULL) {
      free((*board_ptr)->cells);
      (*board_ptr)->cells = NULL;
    }
    free(*board_ptr);
    *board_ptr = NULL;
  }
  (void)board_ptr;
}

int *board_cell(SudokuBoard *board, int row, int column) {
  /* STUDENT TODO 2: Return the mutable cell pointer for this coordinate. */
  (void)board;
  (void)row;
  (void)column;
  return NULL;
}

const int *board_cell_const(const SudokuBoard *board, int row, int column) {
  /* STUDENT TODO 2: Return the read-only cell pointer for this coordinate. */
  (void)board;
  (void)row;
  (void)column;
  return NULL;
}

void board_clear(SudokuBoard *board) {
  int *cursor;
  int *end;

  if (board == NULL || board->cells == NULL) {
    return;
  }

  cursor = board->cells;
  end = board->cells + SUDOKU_CELL_COUNT;

  while (cursor < end) {
    *cursor = SUDOKU_EMPTY;
    cursor++;
  }
}

int board_copy(SudokuBoard *destination, const SudokuBoard *source) {
  if (destination == NULL || destination->cells == NULL || source == NULL ||
      source->cells == NULL) {
    return 0;
  }

  for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
    destination->cells[index] = source->cells[index];
  }

  return 1;
}

int board_equal(const SudokuBoard *first, const SudokuBoard *second) {
  if (first == NULL || first->cells == NULL || second == NULL ||
      second->cells == NULL) {
    return 0;
  }

  for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
    if (first->cells[index] != second->cells[index]) {
      return 0;
    }
  }

  return 1;
}
