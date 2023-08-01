#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attacks.h"
#include "board.h"
#include "moves.h"
#include "perft.h"
#include "score.h"
#include "utils.h"

#include <cul/assert.h>
#include <cul/mem.h>

#define MAX_PLY 64

typedef struct Stats Stats;
struct Stats {
    long nodes;
    int ply;
    int pv_length[MAX_PLY];
    Move pv_table[MAX_PLY][MAX_PLY];
    Move killer_moves[2][MAX_PLY];
    U32 history_moves[16][64];
};

Stats *Stats_new(void) {
    Stats *p;
    NEW0(p);
    return p;
}

void Stats_free(Stats **p) { FREE(*p); }

int move_score(Stats *stats, Move move) {
    if (move_capture(move)) {
        return Score_capture(piece_piece(move_piece(move)),
                             piece_piece(move_piece_capture(move)));
    } else {
        if (move_cmp(stats->killer_moves[0][stats->ply], move))
            return 9000;
        else if (move_cmp(stats->killer_moves[1][stats->ply], move))
            return 8000;
        else
            return stats->history_moves[piece_index(move_piece(move))]
                                       [move_target(move)];
    }

    return 0;
}

void move_list_sort(Stats *stats, MoveList *list) {
    int score[list->count];
    for (int i = 0; i < list->count; i++)
        score[i] = move_score(stats, list->moves[i]);

    for (int i = 0; i < list->count; i++)
        for (int j = i + 1; j < list->count; j++)
            if (score[i] < score[j]) {
                Move t = list->moves[i];
                list->moves[i] = list->moves[j];
                list->moves[j] = t;

                int s = score[i];
                score[i] = score[j];
                score[j] = s;
            }
}

/* SEARCHING */

int evaluate(Board *board) {
    Square square;
    eColor side = board_side(board);
    U64 occupancy = board_color(board, side);

    int score = 0;
    for (int i = 0; i < 6; i++) {
        U64 bitboard = board_piece(board, i);
        bitboard_for_each_bit(square, bitboard) {
            if (bit_get(occupancy, square)) {
                score += Score_value(i);
                score += Score_position(i, side, square);
            } else {
                score -= Score_value(i);
                score -= Score_position(i, !side, square);
            }
        }
    }

    return score;
}

int quiescence(Stats *stats, Board *board, int alpha, int beta) {
    MoveList *moves;
    Board *copy;

    int eval = evaluate(board);
    stats->nodes++;

    if (eval >= beta) {
        return beta;
    }

    if (eval > alpha) {
        alpha = eval;
    }

    copy = board_new();
    moves = move_list_generate(NULL, board);
    move_list_sort(stats, moves);

    for (int i = 0; i < move_list_size(moves); i++) {
        board_copy(board, copy);

        if (move_make(move_list_move(moves, i), copy, 1) == 0) continue;

        stats->ply++;
        int score = -quiescence(stats, copy, -beta, -alpha);
        stats->ply--;

        if (score >= beta) {
            move_list_free(&moves);
            board_free(&copy);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    move_list_free(&moves);
    board_free(&copy);
    return alpha;
}

int negamax(Stats *stats, Board *board, int alpha, int beta, int depth) {
    MoveList *list;
    Board *copy;
    int isCheck = 0;
    int ply = stats->ply;

    stats->pv_length[ply] = ply;

    if (depth == 0) return quiescence(stats, board, alpha, beta);

    if (ply > MAX_PLY - 1) return evaluate(board);

    stats->nodes++;

    copy = board_new();
    list = move_list_generate(NULL, board);
    isCheck = board_isCheck(board);

    if (isCheck) depth++;

    int legal_moves = 0;
    move_list_sort(stats, list);
    for (int i = 0; i < move_list_size(list); i++) {
        Move move = move_list_move(list, i);

        board_copy(board, copy);
        if (move_make(move, copy, 0) == 0) {
            continue;
        }

        stats->ply++;
        int score = -negamax(stats, copy, -beta, -alpha, depth - 1);
        stats->ply--;
        legal_moves++;

        if (score >= beta) {
            if (!move_capture(move)) {
                stats->killer_moves[1][ply] = stats->killer_moves[0][ply];
                stats->killer_moves[0][ply] = move;
            }
            move_list_free(&list);
            board_free(&copy);
            return beta;
        }

        if (score > alpha) {
            if (!move_capture(move))
                stats->history_moves[piece_index(move_piece(move))]
                                    [move_target(move)] += depth;
            alpha = score;

            stats->pv_table[ply][ply] = move;
            for (int i = stats->ply + 1; i < stats->pv_length[ply + 1]; i++)
                stats->pv_table[ply][i] = stats->pv_table[ply + 1][i];
            stats->pv_length[ply] = stats->pv_length[ply + 1];
        }
    }

    if (legal_moves == 0) {
        if (isCheck)
            return -49000 + stats->ply;
        else
            return 0;
    }

    move_list_free(&list);
    board_free(&copy);
    return alpha;
}

void move_print_UCI(Move move) {
    printf("%s%s", square_to_coordinates[move_source(move)],
           square_to_coordinates[move_target(move)]);
    if (move_promote(move)) printf(" %c", piece_asci(move_piece_promote(move)));
}

void search_position(Board *board, int depth) {
    Stats *stats = Stats_new();

    for (int crnt = 1; crnt <= depth; crnt++) {
        int score = negamax(stats, board, -50000, 50000, crnt);

        printf("info score cp %d depth %d nodes %ld pv ", score, crnt,
               stats->nodes);

        for (int i = 0; i < stats->pv_length[0]; i++) {
            move_print_UCI(stats->pv_table[0][i]);
            printf(" ");
        }
        printf("\n");
    }

    printf("bestmove ");
    move_print_UCI(stats->pv_table[0][0]);
    printf("\n");

    Stats_free(&stats);
}

void print_info(void) {
    printf("id name Stellar\n");
    printf("id author Dimitrije Dobrota\n");
    printf("uciok\n");
}

typedef struct Instruction Instruction;
struct Instruction {
    char *command;
    char *token;
    char *crnt;
};

char *Instruction_token_next(Instruction *self);

Instruction *Instruction_new(char *command) {
    Instruction *p;
    NEW0(p);
    p->command = ALLOC(strlen(command) + 1);
    p->token = ALLOC(strlen(command) + 1);
    strcpy(p->command, command);
    p->crnt = command;
    Instruction_token_next(p);
    return p;
}

void Instruction_free(Instruction **p) {
    FREE((*p)->command);
    FREE((*p)->token);
    FREE(*p);
}

char *Instruction_token(Instruction *self) { return self->token; }
char *Instruction_token_n(Instruction *self, int n) {
    while (isspace(*self->crnt) && *self->crnt != '\0')
        self->crnt++;

    if (*self->crnt == '\0') {
        *self->token = '\0';
        return NULL;
    }

    char *p = self->token;
    while (n--) {
        while (!isspace(*self->crnt) && *self->crnt != '\0' &&
               *self->crnt != ';')
            *p++ = *self->crnt++;
        if (*self->crnt == '\0') {
            p++;
            break;
        }
        self->crnt++;
        *p++ = ' ';
    }
    *--p = '\0';

    return self->token;
}

char *Instruction_token_next(Instruction *self) {
    return Instruction_token_n(self, 1);
}

Move parse_move(Board *board, char *move_string) {
    Move result = {0};
    MoveList *moves;
    Square source, target;

    source = coordinates_to_square(move_string);
    target = coordinates_to_square(move_string + 2);

    moves = move_list_generate(NULL, board);
    for (int i = 0; i < moves->count; i++) {
        Move move = moves->moves[i];
        if (move_source(move) == source && move_target(move) == target) {
            if (move_string[4]) {
                if (tolower(piece_code(move_piece_promote(move))) !=
                    move_string[4])
                    continue;
            }
            result = move;
            break;
        }
    }

    move_list_free(&moves);
    return result;
}

Board *Instruction_parse(Instruction *self, Board *board) {
    char *token = Instruction_token(self);

    if (!board) board = board_new();

    do {
        if (strcmp(token, "ucinewgame") == 0) {
            board = board_from_FEN(board, start_position);
            continue;
        }

        if (strcmp(token, "quit") == 0) return NULL;

        if (strcmp(token, "position") == 0) {
            token = Instruction_token_next(self);
            if (token && strcmp(token, "startpos") == 0) {
                board = board_from_FEN(board, start_position);
            } else if (token && strcmp(token, "fen") == 0) {
                token = Instruction_token_n(self, 6);
                board = board_from_FEN(board, token);
            } else {
                printf("Unknown argument after position\n");
            }
            // board_print(board);
            continue;
        }

        if (strcmp(token, "moves") == 0) {
            while ((token = Instruction_token_next(self))) {
                Move move = parse_move(board, token);
                if (!move_cmp(move, (Move){0})) {
                    move_make(move, board, 0);
                } else {
                    printf("Invalid move %s!\n", token);
                }
            }
            // board_print(board);
            return board;
        }

        if (strcmp(token, "go") == 0) {
            token = Instruction_token_next(self);
            int depth = 6;
            if (token && strcmp(token, "depth") == 0) {
                token = Instruction_token_next(self);
                depth = atoi(token);
            } else {
                printf("Unknown argument after go\n");
            }
            search_position(board, depth);
            continue;
        }

        if (strcmp(token, "isready") == 0) {
            printf("readyok\n");
            continue;
        }

        if (strcmp(token, "uci") == 0) {
            print_info();
            continue;
        }

    } while ((token = Instruction_token_next(self)));

    return board;
}

void uci_loop(void) {
    Board *board = NULL;
    Instruction *instruction;
    char input[200000];

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    print_info();
    while (1) {
        memset(input, 0, sizeof(input));
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) continue;

        instruction = Instruction_new(input);
        if (!(board = Instruction_parse(instruction, board))) break;
        Instruction_free(&instruction);
    }

    Instruction_free(&instruction);
    board_free(&board);
}

/* MAIN */

int main(void) {
    attacks_init();
    uci_loop();
    return 0;
}
