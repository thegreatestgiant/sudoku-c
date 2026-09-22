#include "../include/game.h"

#include <stdlib.h>

static size_t cell_index(int row, int column) {
  return (size_t)row * SUDOKU_SIZE + (size_t)column;
}

static unsigned char *create_fixed_map(const SudokuBoard *puzzle) {
  unsigned char *fixed;

  if (puzzle == NULL || puzzle->cells == NULL) {
    return NULL;
  }

  fixed = calloc(SUDOKU_CELL_COUNT, sizeof(*fixed));
  if (fixed == NULL) {
    return NULL;
  }

  for (int row = 0; row < SUDOKU_SIZE; row++) {
    for (int column = 0; column < SUDOKU_SIZE; column++) {
      const int *cell = board_cell_const(puzzle, row, column);
      fixed[cell_index(row, column)] =
          (unsigned char)(cell != NULL && *cell != SUDOKU_EMPTY);
    }
  }

  return fixed;
}

/* STUDENT TODO 5: Construct an empty game object. */
SudokuGame *game_create(void) {
  SudokuGame *game = calloc(1, sizeof(SudokuGame));
  if (!game) {
    return NULL;
  }
  return game;
}

/* STUDENT TODO 5: Release every allocation owned by the game. */
void game_destroy(SudokuGame **game_ptr) {
  if (!game_ptr || !(*game_ptr)) {
    return;
  }
  board_destroy(&((*game_ptr)->puzzle));
  board_destroy(&((*game_ptr)->solution));
  (*game_ptr)->solution = NULL;
  free((*game_ptr)->fixed);
  (*game_ptr)->solution = NULL;
  free((*game_ptr)->history.items);
  (*game_ptr)->history.items = NULL;
  free(*game_ptr);
  *game_ptr = NULL;
}

int game_start_new(SudokuGame *game, Difficulty difficulty) {
  /*
   * STUDENT TODO 6: Replace the current game with a newly generated one.
   * A failed replacement must leave an existing game unchanged.
   */
  (void)game;
  (void)difficulty;
  (void)create_fixed_map;
  return 0;
}

int game_cell_is_fixed(const SudokuGame *game, int row, int column) {
  if (game == NULL || game->fixed == NULL ||
      !board_coordinates_in_range(row, column)) {
    return 0;
  }

  return game->fixed[cell_index(row, column)] != 0;
}

static int record_move(SudokuGame *game, int row, int column,
                       int previous_value, int new_value) {
  Move move;

  move.row = row;
  move.column = column;
  move.previous_value = previous_value;
  move.new_value = new_value;
  return history_push(&game->history, move);
}

MoveResult game_place_value(SudokuGame *game, int row, int column, int value) {
  int *cell;
  int old_value;

  if (game == NULL || !game->active) {
    return MOVE_NO_ACTIVE_GAME;
  }

  if (!board_coordinates_in_range(row, column) || value < 1 || value > 9) {
    return MOVE_OUT_OF_RANGE;
  }

  if (game_cell_is_fixed(game, row, column)) {
    return MOVE_FIXED_CELL;
  }

  cell = board_cell(game->puzzle, row, column);
  if (cell == NULL) {
    return MOVE_OUT_OF_RANGE;
  }

  old_value = *cell;
  *cell = SUDOKU_EMPTY;

  if (!sudoku_is_value_valid(game->puzzle, row, column, value)) {
    *cell = old_value;
    return MOVE_INVALID_PLACEMENT;
  }

  *cell = value;

  if (old_value != value && !record_move(game, row, column, old_value, value)) {
    *cell = old_value;
    return MOVE_MEMORY_ERROR;
  }

  return MOVE_OK;
}

MoveResult game_clear_value(SudokuGame *game, int row, int column) {
  int *cell;
  int old_value;

  if (game == NULL || !game->active) {
    return MOVE_NO_ACTIVE_GAME;
  }

  if (!board_coordinates_in_range(row, column)) {
    return MOVE_OUT_OF_RANGE;
  }

  if (game_cell_is_fixed(game, row, column)) {
    return MOVE_FIXED_CELL;
  }

  cell = board_cell(game->puzzle, row, column);
  if (cell == NULL) {
    return MOVE_OUT_OF_RANGE;
  }

  old_value = *cell;
  if (old_value == SUDOKU_EMPTY) {
    return MOVE_OK;
  }

  *cell = SUDOKU_EMPTY;
  if (!record_move(game, row, column, old_value, SUDOKU_EMPTY)) {
    *cell = old_value;
    return MOVE_MEMORY_ERROR;
  }

  return MOVE_OK;
}

MoveResult game_undo(SudokuGame *game) {
  Move last_move;
  int *cell;

  if (game == NULL || !game->active) {
    return MOVE_NO_ACTIVE_GAME;
  }

  if (!history_pop(&game->history, &last_move)) {
    return MOVE_NOTHING_TO_UNDO;
  }

  cell = board_cell(game->puzzle, last_move.row, last_move.column);
  if (cell == NULL) {
    return MOVE_OUT_OF_RANGE;
  }

  *cell = last_move.previous_value;
  return MOVE_OK;
}

int game_has_won(const SudokuGame *game) {
  if (game == NULL || !game->active) {
    return 0;
  }

  return sudoku_is_complete(game->puzzle) &&
         sudoku_is_board_valid(game->puzzle) &&
         board_equal(game->puzzle, game->solution);
}

static void print_board(const SudokuBoard *board, FILE *output) {
  fprintf(output, "      1 2 3   4 5 6   7 8 9\n");
  fprintf(output, "    +-------+-------+-------+\n");

  for (int row = 0; row < SUDOKU_SIZE; row++) {
    fprintf(output, " %d  |", row + 1);

    for (int column = 0; column < SUDOKU_SIZE; column++) {
      const int *cell = board_cell_const(board, row, column);
      int value = cell == NULL ? SUDOKU_EMPTY : *cell;

      if (value == SUDOKU_EMPTY) {
        fprintf(output, " .");
      } else {
        fprintf(output, " %d", value);
      }

      if ((column + 1) % SUDOKU_BOX_SIZE == 0) {
        fprintf(output, " |");
      }
    }

    fprintf(output, "\n");

    if ((row + 1) % SUDOKU_BOX_SIZE == 0) {
      fprintf(output, "    +-------+-------+-------+\n");
    }
  }

  fprintf(output, "\n");
}

void game_print_to(const SudokuGame *game, FILE *output) {
  if (output == NULL) {
    return;
  }

  if (game == NULL || !game->active) {
    fprintf(output, "No active game. Enter 'new game' to begin.\n");
    return;
  }

  fprintf(output, "\nDifficulty: %s\n",
          sudoku_difficulty_name(game->difficulty));
  print_board(game->puzzle, output);
}

void game_print_solution_to(const SudokuGame *game, FILE *output) {
  if (output == NULL) {
    return;
  }

  if (game == NULL || !game->active) {
    fprintf(output, "No active game. Enter 'new game' to begin.\n");
    return;
  }

  fprintf(output, "\nSolution:\n");
  print_board(game->solution, output);
}

void game_print(const SudokuGame *game) { game_print_to(game, stdout); }

void game_print_solution(const SudokuGame *game) {
  game_print_solution_to(game, stdout);
}

const char *game_move_result_message(MoveResult result) {
  switch (result) {
  case MOVE_OK:
    return "Move accepted.";
  case MOVE_NO_ACTIVE_GAME:
    return "No active game. Enter 'new game' first.";
  case MOVE_OUT_OF_RANGE:
    return "Invalid input: rows, columns, and values must be integers from 1 "
           "to 9.";
  case MOVE_FIXED_CELL:
    return "That cell is part of the original puzzle and cannot be changed.";
  case MOVE_INVALID_PLACEMENT:
    return "Invalid int placement: that value conflicts with its row, column, "
           "or 3x3 box.";
  case MOVE_NOTHING_TO_UNDO:
    return "Nothing to undo.";
  case MOVE_MEMORY_ERROR:
    return "Move failed: the program could not grow the move-history array.";
  default:
    return "Unknown move result.";
  }
}
