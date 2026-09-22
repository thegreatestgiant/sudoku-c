#include "../include/board.h"
#include "../include/command.h"
#include "../include/game.h"
#include "../include/history.h"
#include "../include/sudoku.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int KNOWN_SOLUTION[SUDOKU_CELL_COUNT] = {
    5, 3, 4, 6, 7, 8, 9, 1, 2,
    6, 7, 2, 1, 9, 5, 3, 4, 8,
    1, 9, 8, 3, 4, 2, 5, 6, 7,
    8, 5, 9, 7, 6, 1, 4, 2, 3,
    4, 2, 6, 8, 5, 3, 7, 9, 1,
    7, 1, 3, 9, 2, 4, 8, 5, 6,
    9, 6, 1, 5, 3, 7, 2, 8, 4,
    2, 8, 7, 4, 1, 9, 6, 3, 5,
    3, 4, 5, 2, 8, 6, 1, 7, 9
};

static SudokuBoard *make_known_solution(void) {
    SudokuBoard *board = board_create();

    assert(board != NULL);
    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        board->cells[index] = KNOWN_SOLUTION[index];
    }
    return board;
}

static int count_holes(const SudokuBoard *board) {
    int holes = 0;

    assert(board != NULL);
    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        if (board->cells[index] == SUDOKU_EMPTY) {
            holes++;
        }
    }
    return holes;
}

static unsigned long board_checksum(const SudokuBoard *board) {
    unsigned long checksum = 5381UL;

    assert(board != NULL);
    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        checksum = checksum * 33UL + (unsigned long)board->cells[index];
    }
    return checksum;
}

static void find_empty_cell(const SudokuGame *game, int *row, int *column) {
    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            const int *cell = board_cell_const(game->puzzle, r, c);
            if (cell != NULL && *cell == SUDOKU_EMPTY) {
                *row = r;
                *column = c;
                return;
            }
        }
    }

    assert(!"expected an empty cell");
}

static void test_board_lifecycle(void) {
    SudokuBoard *first = board_create();
    SudokuBoard *second = board_create();

    assert(first != NULL);
    assert(second != NULL);
    assert(first != second);
    assert(first->cells != NULL);
    assert(second->cells != NULL);
    assert(first->cells != second->cells);

    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        assert(first->cells[index] == 0);
        assert(second->cells[index] == 0);
    }

    first->cells[0] = 7;
    assert(second->cells[0] == 0);

    board_destroy(&first);
    assert(first == NULL);
    assert(second->cells[0] == 0);

    board_destroy(&second);
    assert(second == NULL);
    board_destroy(&second);
    board_destroy(NULL);
}

static void test_board_pointer_arithmetic_and_clear(void) {
    SudokuBoard *board = board_create();
    const SudokuBoard *const_board;

    assert(board != NULL);
    assert(board_cell(board, 4, 7) ==
           board->cells + 4 * SUDOKU_SIZE + 7);

    *board_cell(board, 4, 7) = 9;
    const_board = board;
    assert(*board_cell_const(const_board, 4, 7) == 9);

    assert(board_cell(NULL, 0, 0) == NULL);
    assert(board_cell(board, -1, 0) == NULL);
    assert(board_cell(board, 9, 0) == NULL);
    assert(board_cell(board, 0, 9) == NULL);
    assert(board_cell_const(NULL, 0, 0) == NULL);
    assert(board_cell_const(board, 0, -1) == NULL);

    board_clear(board);
    assert(*board_cell(board, 4, 7) == 0);

    board_destroy(&board);
}

static void test_deep_clone_and_copy(void) {
    SudokuBoard invalid_source = {NULL};
    SudokuBoard *source = make_known_solution();
    SudokuBoard *copy = board_clone(source);
    SudokuBoard *destination = board_create();

    assert(board_clone(NULL) == NULL);
    assert(board_clone(&invalid_source) == NULL);
    assert(copy != NULL);
    assert(destination != NULL);
    assert(copy != source);
    assert(copy->cells != source->cells);
    assert(board_equal(source, copy));

    copy->cells[0] = 9;
    assert(source->cells[0] == 5);
    assert(!board_equal(source, copy));

    assert(board_copy(destination, source));
    assert(board_equal(destination, source));
    assert(!board_copy(NULL, source));
    assert(!board_copy(destination, NULL));

    board_destroy(&destination);
    board_destroy(&copy);
    board_destroy(&source);
}

static void test_validation_and_solution_counting(void) {
    SudokuBoard *board = make_known_solution();

    assert(sudoku_is_complete(board));
    assert(sudoku_is_board_valid(board));
    assert(sudoku_count_solutions(board, 2) == 1);

    board->cells[1] = 5;
    assert(!sudoku_is_board_valid(board));
    assert(sudoku_count_solutions(board, 2) == 0);
    board->cells[1] = 3;

    board->cells[0] = SUDOKU_EMPTY;
    assert(!sudoku_is_complete(board));
    assert(sudoku_is_value_valid(board, 0, 0, 5));
    assert(!sudoku_is_value_valid(board, 0, 0, 3));
    assert(!sudoku_is_value_valid(board, 0, 0, 6));
    assert(!sudoku_is_value_valid(board, 0, 0, 9));
    assert(!sudoku_is_value_valid(board, -1, 0, 5));
    assert(!sudoku_is_value_valid(board, 0, 0, 0));
    assert(!sudoku_is_value_valid(board, 0, 0, 10));
    assert(sudoku_count_solutions(board, 2) == 1);

    board_destroy(&board);
}

static void test_generation_for_all_difficulties(void) {
    const Difficulty difficulties[] = {
        DIFFICULTY_EASY,
        DIFFICULTY_MEDIUM,
        DIFFICULTY_HARD
    };

    for (size_t index = 0;
         index < sizeof(difficulties) / sizeof(difficulties[0]);
         index++) {
        Difficulty difficulty = difficulties[index];
        SudokuBoard *solution;
        SudokuBoard *puzzle;
        int holes = 0;

        srand((unsigned int)(1200U + index));
        solution = sudoku_generate_solution();
        assert(solution != NULL);
        assert(sudoku_is_complete(solution));
        assert(sudoku_is_board_valid(solution));

        puzzle = sudoku_generate_puzzle(solution, difficulty, &holes);
        assert(puzzle != NULL);
        assert(puzzle != solution);
        assert(puzzle->cells != solution->cells);
        assert(holes == count_holes(puzzle));
        assert(holes <= sudoku_holes_for_difficulty(difficulty));
        assert(holes >= sudoku_holes_for_difficulty(difficulty) - 3);
        assert(sudoku_is_board_valid(puzzle));
        assert(sudoku_count_solutions(puzzle, 2) == 1);

        for (size_t cell = 0; cell < SUDOKU_CELL_COUNT; cell++) {
            if (puzzle->cells[cell] != SUDOKU_EMPTY) {
                assert(puzzle->cells[cell] == solution->cells[cell]);
            }
        }

        board_destroy(&puzzle);
        board_destroy(&solution);
    }
}

static void test_history_vector(void) {
    MoveHistory history;
    Move ignored;

    history_init(NULL);
    history_clear(NULL);
    history_destroy(NULL);
    assert(!history_push(NULL, (Move){0, 0, 0, 0}));
    assert(!history_pop(NULL, &ignored));

    history_init(&history);
    assert(history.items == NULL);
    assert(history.count == 0);
    assert(history.capacity == 0);

    for (int index = 0; index < 100; index++) {
        Move move = {index % 9, (index / 9) % 9, index, index + 1};
        assert(history_push(&history, move));
        assert(history.count == (size_t)index + 1U);
        assert(history.capacity >= history.count);
    }

    for (int index = 99; index >= 0; index--) {
        Move move;
        assert(history_pop(&history, &move));
        assert(move.previous_value == index);
        assert(move.new_value == index + 1);
    }

    assert(!history_pop(&history, &ignored));
    assert(!history_pop(&history, NULL));

    assert(history_push(&history, (Move){1, 2, 3, 4}));
    history_clear(&history);
    assert(history.count == 0);
    assert(history.capacity > 0);
    assert(history.items != NULL);

    history_destroy(&history);
    assert(history.items == NULL);
    assert(history.count == 0);
    assert(history.capacity == 0);
    history_destroy(&history);
}

static void test_game_lifecycle(void) {
    SudokuGame *game = game_create();

    assert(!game_start_new(NULL, DIFFICULTY_EASY));
    assert(game != NULL);
    assert(game->puzzle == NULL);
    assert(game->solution == NULL);
    assert(game->fixed == NULL);
    assert(game->history.items == NULL);
    assert(game->history.count == 0);
    assert(game->history.capacity == 0);
    assert(!game->active);

    game_destroy(&game);
    assert(game == NULL);
    game_destroy(&game);
    game_destroy(NULL);
}

static void test_game_moves_undo_and_win(void) {
    SudokuGame *game = game_create();
    int row = -1;
    int column = -1;
    int conflicting_value = 0;
    int correct_value;

    assert(game != NULL);
    assert(!game->active);
    assert(game_place_value(game, 0, 0, 1) == MOVE_NO_ACTIVE_GAME);
    assert(game_undo(game) == MOVE_NO_ACTIVE_GAME);

    srand(2001U);
    assert(game_start_new(game, DIFFICULTY_EASY) > 0);
    assert(game->active);
    assert(game->puzzle != NULL);
    assert(game->solution != NULL);
    assert(game->fixed != NULL);
    assert(!game_has_won(game));

    for (int r = 0; r < SUDOKU_SIZE && row == -1; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            if (!game_cell_is_fixed(game, r, c)) {
                for (int other = 0; other < SUDOKU_SIZE; other++) {
                    const int *candidate = board_cell_const(game->puzzle, r, other);
                    if (candidate != NULL && *candidate != SUDOKU_EMPTY) {
                        row = r;
                        column = c;
                        conflicting_value = *candidate;
                        break;
                    }
                }
            }
            if (row != -1) {
                break;
            }
        }
    }

    assert(row != -1);
    assert(game_place_value(game, -1, column, 1) == MOVE_OUT_OF_RANGE);
    assert(game_place_value(game, row, column, 10) == MOVE_OUT_OF_RANGE);
    assert(game_place_value(game, row, column, conflicting_value) ==
           MOVE_INVALID_PLACEMENT);

    correct_value = *board_cell_const(game->solution, row, column);
    assert(game_place_value(game, row, column, correct_value) == MOVE_OK);
    assert(*board_cell_const(game->puzzle, row, column) == correct_value);
    assert(game->history.count == 1);

    assert(game_clear_value(game, row, column) == MOVE_OK);
    assert(*board_cell_const(game->puzzle, row, column) == SUDOKU_EMPTY);
    assert(game->history.count == 2);

    assert(game_undo(game) == MOVE_OK);
    assert(*board_cell_const(game->puzzle, row, column) == correct_value);
    assert(game_undo(game) == MOVE_OK);
    assert(*board_cell_const(game->puzzle, row, column) == SUDOKU_EMPTY);
    assert(game_undo(game) == MOVE_NOTHING_TO_UNDO);

    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            if (!game_cell_is_fixed(game, r, c)) {
                int value = *board_cell_const(game->solution, r, c);
                assert(game_place_value(game, r, c, value) == MOVE_OK);
            }
        }
    }
    assert(game_has_won(game));

    game_destroy(&game);
    assert(game == NULL);
}

static void test_fixed_cells_and_new_game_commit(void) {
    SudokuGame *game = game_create();
    SudokuBoard *old_puzzle;
    SudokuBoard *old_solution;
    unsigned char *old_fixed;

    assert(game != NULL);
    srand(3001U);
    assert(game_start_new(game, DIFFICULTY_EASY) > 0);

    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int column = 0; column < SUDOKU_SIZE; column++) {
            const int *cell = board_cell_const(game->puzzle, row, column);
            assert(game_cell_is_fixed(game, row, column) ==
                   (cell != NULL && *cell != SUDOKU_EMPTY));
            if (game_cell_is_fixed(game, row, column)) {
                assert(game_clear_value(game, row, column) == MOVE_FIXED_CELL);
            }
        }
    }

    {
        int row;
        int column;
        int value;
        find_empty_cell(game, &row, &column);
        value = *board_cell_const(game->solution, row, column);
        assert(game_place_value(game, row, column, value) == MOVE_OK);
        assert(game->history.count == 1);
    }

    old_puzzle = game->puzzle;
    old_solution = game->solution;
    old_fixed = game->fixed;

    srand(3002U);
    assert(game_start_new(game, DIFFICULTY_HARD) > 0);
    assert(game->puzzle != old_puzzle);
    assert(game->solution != old_solution);
    assert(game->fixed != old_fixed);
    assert(game->history.count == 0);
    assert(game->difficulty == DIFFICULTY_HARD);

    game_destroy(&game);
}

static void read_stream(FILE *stream, char *buffer, size_t size) {
    size_t bytes;

    assert(stream != NULL);
    assert(buffer != NULL);
    assert(size > 0);

    fflush(stream);
    rewind(stream);
    bytes = fread(buffer, 1, size - 1, stream);
    buffer[bytes] = '\0';
}

static void test_command_table_and_dispatch(void) {
    SudokuGame *game = game_create();
    FILE *output = tmpfile();
    const CommandEntry *commands;
    size_t command_count = 0;
    char buffer[8192];
    char new_command[] = "  NEW EASY\n";
    char solution_command[] = "solution\n";
    char unknown_command[] = "xyzzy\n";
    char quit_command[] = "quit\n";
    unsigned long before_solution;

    assert(game != NULL);
    assert(output != NULL);

    assert(command_table(NULL) != NULL);
    assert(command_dispatch(NULL, NULL, output) == 0);
    assert(command_dispatch(game, new_command, NULL) == 0);

    commands = command_table(&command_count);
    assert(commands != NULL);
    assert(command_count >= 10);
    for (size_t index = 0; index < command_count; index++) {
        assert(commands[index].name != NULL);
        assert(commands[index].handler != NULL);
    }

    srand(4001U);
    assert(command_dispatch(game, new_command, output) == 1);
    assert(game->active);
    assert(game->difficulty == DIFFICULTY_EASY);

    before_solution = board_checksum(game->puzzle);
    assert(command_dispatch(game, solution_command, output) == 1);
    assert(board_checksum(game->puzzle) == before_solution);

    assert(command_dispatch(game, unknown_command, output) == 1);
    assert(command_dispatch(game, quit_command, output) == 0);

    read_stream(output, buffer, sizeof(buffer));
    assert(strstr(buffer, "Started a new easy game") != NULL);
    assert(strstr(buffer, "Solution:") != NULL);
    assert(strstr(buffer, "Unknown command") != NULL);

    fclose(output);
    game_destroy(&game);
}

static void test_printing_without_active_game(void) {
    SudokuGame *game = game_create();
    FILE *output = tmpfile();
    char buffer[512];

    assert(game != NULL);
    assert(output != NULL);
    game_print_to(game, output);
    game_print_solution_to(game, output);
    read_stream(output, buffer, sizeof(buffer));
    assert(strstr(buffer, "No active game") != NULL);

    fclose(output);
    game_destroy(&game);
}

static int requested(const char *selected, const char *name) {
    return selected == NULL || strcmp(selected, "all") == 0 ||
           strcmp(selected, name) == 0;
}

int main(int argc, char **argv) {
    const char *selected = argc >= 2 ? argv[1] : "all";
    int recognized = 0;

    if (requested(selected, "board-lifecycle")) {
        recognized = 1;
        test_board_lifecycle();
    }
    if (requested(selected, "board-access")) {
        recognized = 1;
        test_board_pointer_arithmetic_and_clear();
    }
    if (requested(selected, "board-clone")) {
        recognized = 1;
        test_deep_clone_and_copy();
    }
    if (requested(selected, "sudoku")) {
        recognized = 1;
        test_validation_and_solution_counting();
        test_generation_for_all_difficulties();
    }
    if (requested(selected, "history")) {
        recognized = 1;
        test_history_vector();
    }
    if (requested(selected, "game-lifecycle")) {
        recognized = 1;
        test_game_lifecycle();
    }
    if (requested(selected, "new-game")) {
        recognized = 1;
        test_validation_and_solution_counting();
        test_generation_for_all_difficulties();
        test_game_moves_undo_and_win();
        test_fixed_cells_and_new_game_commit();
        test_printing_without_active_game();
    }
    if (requested(selected, "commands")) {
        recognized = 1;
        test_command_table_and_dispatch();
    }

    if (!recognized) {
        fprintf(stderr, "Unknown test group: %s\n", selected);
        return EXIT_FAILURE;
    }

    printf("Functional test group '%s' passed.\n", selected);
    return EXIT_SUCCESS;
}
