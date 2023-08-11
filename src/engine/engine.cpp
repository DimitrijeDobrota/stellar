#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack.hpp"
#include "board.hpp"
#include "moves.hpp"
#include "score.hpp"
#include "stats.hpp"
#include "transposition.hpp"
#include "utils_cpp.hpp"
#include "zobrist.hpp"

#define FULL_DEPTH 4
#define REDUCTION_LIMIT 3
#define REDUCTION_MOVE 2

#define WINDOW 50

void move_list_sort_pv(std::vector<MoveE> &list, Stats &stats, Move best) {
    for (MoveE &move : list) {
        if (move_cmp(best, move.move)) {
            move.score = 30000;
            return;
        }
    }

    if (stats.ply && stats.follow_pv) {
        stats.follow_pv = 0;
        for (MoveE &move : list) {
            if (move_cmp(stats.pv_table[0][stats.ply], move.move)) {
                move.score = 20000;
                stats.follow_pv = 1;
                break;
            }
        }
    }
}

/* SEARCHING */

int evaluate(const Board &board) {
    Color side = board.get_side();
    Color sideOther = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;

    U64 occupancy = board.get_bitboard_color(side);
    uint8_t square_i;

    int score = 0;
    for (piece::Type type : piece::TypeIter()) {
        U64 bitboard = board.get_bitboard_piece(type);
        bitboard_for_each_bit(square_i, bitboard) {
            Square square = static_cast<Square>(square_i);
            if (bit_get(occupancy, square_i)) {
                score += Score_value(type);
                score += Score_position(type, side, square);
            } else {
                score -= Score_value(type);
                score -= Score_position(type, sideOther, square);
            }
        }
    }

    return score;
}

int is_repetition() {
    return 0;
}

int stats_move_make(Stats &stats, Board &copy, Move move, int flag) {
    copy = stats.board;
    if (!move_make(move, stats.board, flag)) {
        stats.board = copy;
        return 0;
    }
    stats.ply++;
    return 1;
}

void stats_move_make_pruning(Stats &stats, Board &copy) {
    copy = stats.board;
    stats.board.switch_side();
    stats.board.set_enpassant(Square::no_sq);
    stats.ply++;
}

void stats_move_unmake(Stats &stats, Board &copy) {
    stats.board = copy;
    stats.ply--;
}

void stats_pv_store(Stats &stats, Move move) {
    const int ply = stats.ply;
    stats.pv_table[ply][ply] = move;
    for (int i = ply + 1; i < stats.pv_length[ply + 1]; i++) {
        stats.pv_table[ply][i] = stats.pv_table[ply + 1][i];
    }
    stats.pv_length[ply] = stats.pv_length[ply + 1];
}

int quiescence(Stats &stats, int alpha, int beta) {
    stats.pv_length[stats.ply] = stats.ply;
    stats.nodes++;

    int score = evaluate(stats.board);
    if (stats.ply > MAX_PLY - 1) return score;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;

    Board copy;
    std::vector<MoveE> list = move_list_generate(stats.board);
    Score_move_list(stats, list);
    move_list_sort_pv(list, stats, {0});
    move_list_sort(list);

    for (const auto [move, _] : list) {
        if (!stats_move_make(stats, copy, move, 1)) continue;
        score = -quiescence(stats, -beta, -alpha);
        stats_move_unmake(stats, copy);

        if (score > alpha) {
            alpha = score;
            stats_pv_store(stats, move);
            if (score >= beta) return beta;
        }
    }

    return alpha;
}

int negamax(Stats &stats, int alpha, int beta, int depth, bool null) {
    int pv_node = (beta - alpha) > 1;
    HasheFlag flag = HasheFlag::Alpha;
    Move bestMove = {0};
    int futility = 0;
    Board copy;

    stats.pv_length[stats.ply] = stats.ply;

    int score = stats.ttable.read(stats, &bestMove, alpha, beta, depth);
    if (stats.ply && score != TTABLE_UNKNOWN && !pv_node) return score;

    // && fifty >= 100
    if (stats.ply && is_repetition()) return 0;
    if (depth == 0) {
        stats.nodes++;
        int score = quiescence(stats, alpha, beta);
        // ttable_write(stats, bestMove, score, depth, HasheFlag::Exact);
        return score;
    }

    if (alpha < -MATE_VALUE) alpha = -MATE_VALUE;
    if (beta > MATE_VALUE - 1) beta = MATE_VALUE - 1;
    if (alpha >= beta) return alpha;
    // if (stats.ply > MAX_PLY - 1) return evaluate(stats.board);

    int isCheck = stats.board.is_check();
    if (isCheck) depth++;

    if (!pv_node && !isCheck) {
        int staticEval = evaluate(stats.board);

        // evaluation pruning
        if (depth < 3 && abs(beta - 1) > -MATE_VALUE + 100) {
            int marginEval = Score_value(piece::Type::PAWN) * depth;
            if (staticEval - marginEval >= beta) return staticEval - marginEval;
        }

        if (null) {
            // null move pruning
            if (stats.ply && depth > 2 && staticEval >= beta) {
                stats_move_make_pruning(stats, copy);
                score = -negamax(stats, -beta, -beta + 1, depth - 1 - REDUCTION_MOVE, false);
                stats_move_unmake(stats, copy);
                if (score >= beta) return beta;
            }

            // razoring
            score = staticEval + Score_value(piece::Type::PAWN);
            int scoreNew = quiescence(stats, alpha, beta);

            if (score < beta && depth == 1) {
                return (scoreNew > score) ? scoreNew : score;
            }

            score += Score_value(piece::Type::PAWN);
            if (score < beta && depth < 4) {
                if (scoreNew < beta) return (scoreNew > score) ? scoreNew : score;
            }
        }

        // futility pruning condition
        static const int margin[] = {0, 100, 300, 500};
        if (depth < 4 && abs(alpha) < MATE_SCORE && staticEval + margin[depth] <= alpha) futility = 1;
    }

    std::vector<MoveE> list = move_list_generate(stats.board);
    Score_move_list(stats, list);
    move_list_sort_pv(list, stats, bestMove);
    move_list_sort(list);

    int legal_moves = 0;
    int searched = 0;
    for (const auto [move, _] : list) {
        if (!stats_move_make(stats, copy, move, 0)) continue;
        legal_moves++;

        // futility pruning
        if (futility && searched && !move_capture(move) && !move_promote(move) && !stats.board.is_check()) {
            stats_move_unmake(stats, copy);
            continue;
        }

        if (!searched) {
            score = -negamax(stats, -beta, -alpha, depth - 1, true);
        } else {
            // Late Move Reduction
            if (!pv_node && searched >= FULL_DEPTH && depth >= REDUCTION_LIMIT && !isCheck &&
                !move_capture(move) && !move_promote(move) &&
                (move_source(move) != move_source(stats.killer[0][stats.ply]) ||
                 move_target(move) != move_target(stats.killer[0][stats.ply])) &&
                (move_source(move) != move_source(stats.killer[1][stats.ply]) ||
                 move_target(move) != move_target(stats.killer[1][stats.ply]))) {
                score = -negamax(stats, -alpha - 1, -alpha, depth - 2, true);
            } else
                score = alpha + 1;

            // found better move
            // Principal Variation Search
            if (score > alpha) {
                score = -negamax(stats, -alpha - 1, -alpha, depth - 1, true);

                // if fail research
                if ((score > alpha) && (score < beta))
                    score = -negamax(stats, -beta, -alpha, depth - 1, true);
            }
        }

        stats_move_unmake(stats, copy);
        searched++;

        if (score > alpha) {
            if (!move_capture(move)) {
                stats.history[move_piece(move).index][move_target(move)] += depth;
            }

            alpha = score;
            flag = HasheFlag::Exact;
            bestMove = move;
            stats_pv_store(stats, move);

            if (score >= beta) {
                stats.ttable.write(stats, bestMove, beta, depth, HasheFlag::Beta);

                if (!move_capture(move)) {
                    stats.killer[1][stats.ply] = stats.killer[0][stats.ply];
                    stats.killer[0][stats.ply] = move;
                }

                return beta;
            }
        }
    }

    if (legal_moves == 0) {
        if (isCheck)
            return -MATE_VALUE + stats.ply;
        else
            return 0;
    }

    stats.ttable.write(stats, bestMove, alpha, depth, flag);
    return alpha;
}

void move_print_UCI(Move move) {
    printf("%s%s", square_to_coordinates(static_cast<Square>(move_source(move))),
           square_to_coordinates(static_cast<Square>(move_target(move))));
    if (move_promote(move)) printf("%c", (move_piece_promote(move).code));
}

TTable ttable(C64(0x400000));
void search_position(Board &board, int depth) {
    Stats stats = {ttable, board, 0};

    int alpha = -INFINITY, beta = INFINITY;
    for (int crnt = 1; crnt <= depth;) {
        stats.follow_pv = 1;

        int score = negamax(stats, alpha, beta, crnt, true);
        if ((score <= alpha) || (score >= beta)) {
            alpha = -INFINITY;
            beta = INFINITY;
            continue;
        }
        alpha = score - WINDOW;
        beta = score + WINDOW;

        if (stats.pv_length[0]) {
            if (score > -MATE_VALUE && score < -MATE_SCORE) {
                printf("info score mate %d depth %d nodes %ld pv ", -(score + MATE_VALUE) / 2 - 1, crnt,
                       stats.nodes);
            } else if (score > MATE_SCORE && score < MATE_VALUE) {
                printf("info score mate %d depth %d nodes %ld pv ", (MATE_VALUE - score) / 2 + 1, crnt,
                       stats.nodes);
            } else {
                printf("info score cp %d depth %d nodes %ld pv ", score, crnt, stats.nodes);
            }

            for (int i = 0; i < stats.pv_length[0]; i++) {
                move_print_UCI(stats.pv_table[0][i]);
                printf(" ");
            }
            printf("\n");
        }
        crnt++;
    }

    printf("bestmove ");
    move_print_UCI(stats.pv_table[0][0]);
    printf("\n");
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
    Instruction *p = new Instruction();
    p->command = new char(strlen(command) + 1);
    p->token = new char(strlen(command) + 1);
    strcpy(p->command, command);
    p->crnt = command;
    Instruction_token_next(p);
    return p;
}

void Instruction_free(Instruction **p) {
    delete ((*p)->command);
    delete ((*p)->token);
    delete (*p);
}

char *Instruction_token(Instruction *self) {
    return self->token;
}
char *Instruction_token_n(Instruction *self, int n) {
    while (isspace(*self->crnt) && *self->crnt != '\0')
        self->crnt++;

    if (*self->crnt == '\0') {
        *self->token = '\0';
        return NULL;
    }

    char *p = self->token;
    while (n--) {
        while (!isspace(*self->crnt) && *self->crnt != '\0' && *self->crnt != ';')
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

Move parse_move(Board &board, char *move_string) {
    Move result = {0};

    uint8_t source = to_underlying(square_from_coordinates(move_string));
    uint8_t target = to_underlying(square_from_coordinates(move_string + 2));

    std::vector<MoveE> list = move_list_generate(board);
    for (const auto [move, _] : list) {
        if (move_source(move) == source && move_target(move) == target) {
            if (move_string[4]) {
                if (tolower(move_piece_promote(move).code) != move_string[4]) continue;
            }
            result = move;
            break;
        }
    }

    return result;
}

Board *Instruction_parse(Instruction *self, Board &board) {
    char *token = Instruction_token(self);

    do {
        if (strcmp(token, "ucinewgame") == 0) {
            board = Board(start_position);
            continue;
        }

        if (strcmp(token, "quit") == 0) return nullptr;

        if (strcmp(token, "position") == 0) {
            token = Instruction_token_next(self);
            if (token && strcmp(token, "startpos") == 0) {
                board = Board(start_position);
            } else if (token && strcmp(token, "fen") == 0) {
                token = Instruction_token_n(self, 6);
                board = Board(token);
            } else {
                printf("Unknown argument after position\n");
            }
            // board_print(board);
            continue;
        }

        if (strcmp(token, "moves") == 0) {
            while ((token = Instruction_token_next(self))) {
                Move move = parse_move(board, token);
                if (!move_cmp(move, {0})) {
                    move_make(move, board, 0);
                } else {
                    printf("Invalid move %s!\n", token);
                }
            }
            // board_print(board);
            return &board;
        }

        if (strcmp(token, "go") == 0) {
            int depth = 6;
            for (token = Instruction_token_next(self); token; token = Instruction_token_next(self)) {

                if (token && strcmp(token, "depth") == 0) {
                    token = Instruction_token_next(self);
                    depth = atoi(token);
                } else {
                    // printf("Unknown argument %s after go\n", token);
                }
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

    return &board;
}

void uci_loop(void) {
    Board board;
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
        if (!Instruction_parse(instruction, board)) break;
        Instruction_free(&instruction);
    }

    Instruction_free(&instruction);
}

/* MAIN */

int main(void) {
    uci_loop();
    return 0;
}
