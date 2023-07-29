#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CBoard.h"
#include "attack.h"
#include "moves.h"
#include "perft.h"
#include "score.h"
#include "utils.h"

#include <cul/assert.h>
#include <cul/mem.h>

/* DEBUGGING */

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position                                                         \
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position                                                        \
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position                                                        \
    "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position                                                           \
    "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

#define MAX_PLY 64

typedef struct Stats_T *Stats_T;
struct Stats_T {
    long nodes;
    int ply;
    int pv_length[MAX_PLY];
    Move pv_table[MAX_PLY][MAX_PLY];
    Move killer_moves[2][MAX_PLY];
    U32 history_moves[16][64];
};

Stats_T Stats_new(void) {
    Stats_T p;
    NEW0(p);
    return p;
}

void Stats_free(Stats_T *p) { FREE(*p); }

int Move_score(Stats_T stats, Move move) {
    if (Move_capture(move)) {
        return Score_capture(Piece_piece(Move_piece(move)),
                             Piece_piece(Move_piece_capture(move)));
    } else {
        if (!Move_cmp(stats->killer_moves[0][stats->ply], move))
            return 9000;
        else if (!Move_cmp(stats->killer_moves[1][stats->ply], move))
            return 8000;
        else
            return stats->history_moves[Piece_index(Move_piece(move))]
                                       [Move_target(move)];
    }

    return 0;
}

void MoveList_sort(Stats_T stats, MoveList_T list) {
    int score[list->count];
    for (int i = 0; i < list->count; i++)
        score[i] = Move_score(stats, list->moves[i]);

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

int evaluate(CBoard_T board) {
    Square square;
    eColor side = CBoard_side(board);
    U64 occupancy = CBoard_colorBB(board, side);

    int score = 0;
    for (int i = 0; i < 6; i++) {
        U64 bitboard = CBoard_pieceBB(board, i);
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

int quiescence(Stats_T stats, CBoard_T board, int alpha, int beta) {
    MoveList_T moves;
    CBoard_T copy;

    int eval = evaluate(board);
    stats->nodes++;

    if (eval >= beta) {
        return beta;
    }

    if (eval > alpha) {
        alpha = eval;
    }

    copy = CBoard_new();
    moves = MoveList_generate(NULL, board);
    MoveList_sort(stats, moves);

    for (int i = 0; i < MoveList_size(moves); i++) {
        CBoard_copy(board, copy);

        if (Move_make(MoveList_move(moves, i), copy, 1) == 0) continue;

        stats->ply++;
        int score = -quiescence(stats, copy, -beta, -alpha);
        stats->ply--;

        if (score >= beta) {
            MoveList_free(&moves);
            CBoard_free(&copy);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    MoveList_free(&moves);
    CBoard_free(&copy);
    return alpha;
}

int negamax(Stats_T stats, CBoard_T board, int alpha, int beta, int depth) {
    MoveList_T list;
    CBoard_T copy;
    int isCheck = 0;
    int ply = stats->ply;

    stats->pv_length[ply] = ply;

    if (depth == 0) return quiescence(stats, board, alpha, beta);

    if (ply > MAX_PLY - 1) return evaluate(board);

    stats->nodes++;

    copy = CBoard_new();
    list = MoveList_generate(NULL, board);
    isCheck = CBoard_isCheck(board);

    if (isCheck) depth++;

    int legal_moves = 0;
    MoveList_sort(stats, list);
    for (int i = 0; i < MoveList_size(list); i++) {
        Move move = MoveList_move(list, i);

        CBoard_copy(board, copy);
        if (Move_make(move, copy, 0) == 0) {
            continue;
        }

        stats->ply++;
        int score = -negamax(stats, copy, -beta, -alpha, depth - 1);
        stats->ply--;
        legal_moves++;

        if (score >= beta) {
            if (!Move_capture(move)) {
                stats->killer_moves[1][ply] = stats->killer_moves[0][ply];
                stats->killer_moves[0][ply] = move;
            }
            MoveList_free(&list);
            CBoard_free(&copy);
            return beta;
        }

        if (score > alpha) {
            if (!Move_capture(move))
                stats->history_moves[Piece_index(Move_piece(move))]
                                    [Move_target(move)] += depth;
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

    MoveList_free(&list);
    CBoard_free(&copy);
    return alpha;
}

void Move_print_UCI(Move move) {
    printf("%s%s", square_to_coordinates[Move_source(move)],
           square_to_coordinates[Move_target(move)]);
    if (Move_promote(move)) printf(" %c", Piece_asci(Move_piece_promote(move)));
}

void search_position(CBoard_T board, int depth) {
    Stats_T stats = Stats_new();

    for (int crnt = 1; crnt <= depth; crnt++) {
        int score = negamax(stats, board, -50000, 50000, crnt);

        printf("info score cp %d depth %d nodes %ld pv ", score, crnt,
               stats->nodes);

        for (int i = 0; i < stats->pv_length[0]; i++) {
            Move_print_UCI(stats->pv_table[0][i]);
            printf(" ");
        }
        printf("\n");
    }

    printf("bestmove ");
    Move_print_UCI(stats->pv_table[0][0]);
    printf("\n");

    Stats_free(&stats);
}

void print_info(void) {
    printf("id name Stellar\n");
    printf("id author Dimitrije Dobrota\n");
    printf("uciok\n");
}

typedef struct Instruction_T *Instruction_T;
struct Instruction_T {
    char *command;
    char *token;
    char *crnt;
};

char *Instruction_token_next(Instruction_T self);

Instruction_T Instruction_new(char *command) {
    Instruction_T p;
    NEW0(p);
    p->command = ALLOC(strlen(command) + 1);
    p->token = ALLOC(strlen(command) + 1);
    strcpy(p->command, command);
    p->crnt = command;
    Instruction_token_next(p);
    return p;
}

void Instruction_free(Instruction_T *p) {
    FREE((*p)->command);
    FREE((*p)->token);
    FREE(*p);
}

char *Instruction_token(Instruction_T self) { return self->token; }
char *Instruction_token_n(Instruction_T self, int n) {
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

char *Instruction_token_next(Instruction_T self) {
    return Instruction_token_n(self, 1);
}

Move parse_move(CBoard_T board, char *move_string) {
    Move result = {0};
    MoveList_T moves;
    Square source, target;

    source = coordinates_to_square(move_string);
    target = coordinates_to_square(move_string + 2);

    moves = MoveList_generate(NULL, board);
    for (int i = 0; i < moves->count; i++) {
        Move move = moves->moves[i];
        if (Move_source(move) == source && Move_target(move) == target) {
            if (move_string[4]) {
                if (tolower(Piece_code(Move_piece_promote(move))) !=
                    move_string[4])
                    continue;
            }
            result = move;
            break;
        }
    }

    MoveList_free(&moves);
    return result;
}

CBoard_T Instruction_parse(Instruction_T self, CBoard_T board) {
    char *token = Instruction_token(self);

    if (!board) board = CBoard_new();

    do {
        if (strcmp(token, "ucinewgame") == 0) {
            board = CBoard_fromFEN(board, start_position);
            continue;
        }

        if (strcmp(token, "quit") == 0) return NULL;

        if (strcmp(token, "position") == 0) {
            token = Instruction_token_next(self);
            if (token && strcmp(token, "startpos") == 0) {
                board = CBoard_fromFEN(board, start_position);
            } else if (token && strcmp(token, "fen") == 0) {
                token = Instruction_token_n(self, 6);
                board = CBoard_fromFEN(board, token);
            } else {
                printf("Unknown argument after position\n");
            }
            // CBoard_print(board);
            continue;
        }

        if (strcmp(token, "moves") == 0) {
            while ((token = Instruction_token_next(self))) {
                Move move = parse_move(board, token);
                if (!Move_cmp(move, (Move){0})) {
                    Move_make(move, board, 0);
                } else {
                    printf("Invalid move %s!\n", token);
                }
            }
            // CBoard_print(board);
            return board;
        }

        if (strcmp(token, "go") == 0) {
            token = Instruction_token_next(self);
            int depth = 5;
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
    CBoard_T board = NULL;
    Instruction_T instruction;
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
    CBoard_free(&board);
}

/* MAIN */

void init_all() {
    init_leapers_attacks();
    init_sliders_attacks();
}

int main(void) {
    init_all();
    uci_loop();
    return 0;
}
