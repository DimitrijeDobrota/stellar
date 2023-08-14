#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack.hpp"
#include "board.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "score.hpp"
#include "transposition.hpp"
#include "utils_cpp.hpp"
#include "zobrist.hpp"

#define MAX_PLY 64
#define FULL_DEPTH 4
#define REDUCTION_LIMIT 3
#define REDUCTION_MOVE 2

#define WINDOW 50

Board board;
TTable ttable(C64(0x400000));
Move pv_table[MAX_PLY][MAX_PLY];
Move killer[2][MAX_PLY];
U32 history[16][64];
int pv_length[MAX_PLY];
bool follow_pv;
U64 nodes;
U32 ply;

Move move_list_best_move;
U32 inline move_list_score(Move move) {
    const piece::Type type = move.piece().type;
    if (move.is_capture()) return piece::score(type, move.piece_capture().type) + 10000;
    if (killer[0][ply] == move) return 9000;
    if (killer[1][ply] == move) return 8000;
    return history[to_underlying(type)][to_underlying(move.target())];
}

void move_list_sort(MoveList &list, std::vector<int> &index, bool bestCheck = true) {
    static std::vector<int> score(256);

    bool best = false;
    for (int i = 0; i < list.size(); i++) {
        score[i] = move_list_score(list[i]);
        index[i] = i;
        if (bestCheck && list[i] == move_list_best_move) {
            score[i] = 30000;
            bestCheck = false;
            best = true;
        }
    }

    if (!best && ply && follow_pv) {
        follow_pv = false;
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == pv_table[0][ply]) {
                score[i] = 20000;
                follow_pv = true;
                break;
            }
        }
    }

    sort(index.begin(), index.begin() + list.size(), [&](int a, int b) { return score[a] > score[b]; });
}

int evaluate(const Board &board) {
    Color side = board.get_side();
    Color sideOther = (side == Color::BLACK) ? Color::WHITE : Color::BLACK;

    U64 occupancy = board.get_bitboard_color(side);
    uint8_t square_i;

    int score = 0;
    for (const piece::Type type : piece::TypeIter()) {
        U64 bitboard = board.get_bitboard_piece(type);
        bitboard_for_each_bit(square_i, bitboard) {
            Square square = static_cast<Square>(square_i);
            if (bit_get(occupancy, square_i)) {
                score += piece::score(type);
                score += piece::score(type, side, square);
            } else {
                score -= piece::score(type);
                score -= piece::score(type, sideOther, square);
            }
        }
    }

    return score;
}

int is_repetition() { return 0; }

int stats_move_make(Board &copy, Move move, int flag) {
    copy = board;
    if (!move.make(board, flag)) {
        board = copy;
        return 0;
    }
    ply++;
    return 1;
}

void stats_move_make_pruning(Board &copy) {
    copy = board;
    board.switch_side();
    board.set_enpassant(Square::no_sq);
    ply++;
}

void stats_move_unmake(Board &copy) {
    board = copy;
    ply--;
}

void stats_pv_store(Move move) {
    pv_table[ply][ply] = move;
    for (int i = ply + 1; i < pv_length[ply + 1]; i++) {
        pv_table[ply][i] = pv_table[ply + 1][i];
    }
    pv_length[ply] = pv_length[ply + 1];
}

int quiescence(int alpha, int beta) {
    pv_length[ply] = ply;
    nodes++;

    int score = evaluate(board);
    if (ply > MAX_PLY - 1) return score;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;

    Board copy;

    MoveList list(board);
    std::vector<int> index(list.size());
    move_list_sort(list, index, false);
    for (int idx : index) {
        if (!stats_move_make(copy, list[idx], 1)) continue;
        score = -quiescence(-beta, -alpha);
        stats_move_unmake(copy);

        if (score > alpha) {
            alpha = score;
            stats_pv_store(list[idx]);
            if (score >= beta) return beta;
        }
    }

    return alpha;
}

int negamax(int alpha, int beta, int depth, bool null) {
    int pv_node = (beta - alpha) > 1;
    HasheFlag flag = HasheFlag::Alpha;
    int futility = 0;
    Move bestMove;
    Board copy;

    pv_length[ply] = ply;

    int score = ttable.read(board, ply, &bestMove, alpha, beta, depth);
    if (ply && score != TTABLE_UNKNOWN && !pv_node) return score;

    // && fifty >= 100
    if (ply && is_repetition()) return 0;
    if (depth == 0) {
        nodes++;
        int score = quiescence(alpha, beta);
        // ttable_write(board, ply, bestMove, score, depth, HasheFlag::Exact);
        return score;
    }

    if (alpha < -MATE_VALUE) alpha = -MATE_VALUE;
    if (beta > MATE_VALUE - 1) beta = MATE_VALUE - 1;
    if (alpha >= beta) return alpha;
    // if (ply > MAX_PLY - 1) return evaluate(board);

    int isCheck = board.is_check();
    if (isCheck) depth++;

    if (!pv_node && !isCheck) {
        static constexpr const U32 score_pawn = piece::score(piece::Type::PAWN);
        int staticEval = evaluate(board);

        // evaluation pruning
        if (depth < 3 && abs(beta - 1) > -MATE_VALUE + 100) {
            int marginEval = score_pawn * depth;
            if (staticEval - marginEval >= beta) return staticEval - marginEval;
        }

        if (null) {
            // null move pruning
            if (ply && depth > 2 && staticEval >= beta) {
                stats_move_make_pruning(copy);
                score = -negamax(-beta, -beta + 1, depth - 1 - REDUCTION_MOVE, false);
                stats_move_unmake(copy);
                if (score >= beta) return beta;
            }

            // razoring
            score = staticEval + score_pawn;
            int scoreNew = quiescence(alpha, beta);

            if (score < beta && depth == 1) {
                return (scoreNew > score) ? scoreNew : score;
            }

            score += score_pawn;
            if (score < beta && depth < 4) {
                if (scoreNew < beta) return (scoreNew > score) ? scoreNew : score;
            }
        }

        // futility pruning condition
        static constexpr const int margin[] = {
            0,
            piece::score(piece::Type::PAWN),
            piece::score(piece::Type::KNIGHT),
            piece::score(piece::Type::ROOK),
        };
        if (depth < 4 && abs(alpha) < MATE_SCORE && staticEval + margin[depth] <= alpha) futility = 1;
    }

    int legal_moves = 0;
    int searched = 0;

    move_list_best_move = bestMove;
    MoveList list(board);
    std::vector<int> index(list.size());
    move_list_sort(list, index);
    for (int idx : index) {
        const Move move = list[idx];
        if (!stats_move_make(copy, move, 0)) continue;
        legal_moves++;

        // futility pruning
        if (futility && searched && !move.is_capture() && !move.is_promote() && !board.is_check()) {
            stats_move_unmake(copy);
            continue;
        }

        if (!searched) {
            score = -negamax(-beta, -alpha, depth - 1, true);
        } else {
            // Late Move Reduction
            if (!pv_node && searched >= FULL_DEPTH && depth >= REDUCTION_LIMIT && !isCheck &&
                !move.is_capture() && !move.is_promote() &&
                (move.source() != killer[0][ply].source() || move.target() != killer[0][ply].target()) &&
                (move.source() != killer[1][ply].source() || move.target() != killer[1][ply].target())) {
                score = -negamax(-alpha - 1, -alpha, depth - 2, true);
            } else
                score = alpha + 1;

            // Principal Variation Search
            if (score > alpha) {
                score = -negamax(-alpha - 1, -alpha, depth - 1, true);

                // if fail research
                if ((score > alpha) && (score < beta)) score = -negamax(-beta, -alpha, depth - 1, true);
            }
        }

        stats_move_unmake(copy);
        searched++;

        if (score > alpha) {
            if (!move.is_capture()) {
                history[move.piece().index][to_underlying(move.target())] += depth;
            }

            alpha = score;
            flag = HasheFlag::Exact;
            bestMove = move;
            stats_pv_store(move);

            if (score >= beta) {
                ttable.write(board, ply, bestMove, beta, depth, HasheFlag::Beta);

                if (!move.is_capture()) {
                    killer[1][ply] = killer[0][ply];
                    killer[0][ply] = move;
                }

                return beta;
            }
        }
    }

    if (legal_moves == 0) {
        if (isCheck)
            return -MATE_VALUE + ply;
        else
            return 0;
    }

    ttable.write(board, ply, bestMove, alpha, depth, flag);
    return alpha;
}

void move_print_UCI(Move move) {
    std::cout << square_to_coordinates(move.source()) << square_to_coordinates(move.target());
    if (move.is_promote()) std::cout << move.piece_promote().code;
}

void search_position(int depth) {
    int alpha = -SCORE_INFINITY, beta = SCORE_INFINITY;
    for (int crnt = 1; crnt <= depth;) {
        follow_pv = 1;

        int score = negamax(alpha, beta, crnt, true);
        if ((score <= alpha) || (score >= beta)) {
            alpha = -SCORE_INFINITY;
            beta = SCORE_INFINITY;
            continue;
        }
        alpha = score - WINDOW;
        beta = score + WINDOW;

        if (pv_length[0]) {
            if (score > -MATE_VALUE && score < -MATE_SCORE) {
                std::cout << "info score mate " << -(score + MATE_VALUE) / 2 - 1;
            } else if (score > MATE_SCORE && score < MATE_VALUE) {
                std::cout << "info score mate " << (MATE_VALUE - score) / 2 + 1;
            } else {
                std::cout << "info score " << score;
            }

            std::cout << " depth " << crnt;
            std::cout << " nodes " << nodes;
            std::cout << " pv ";
            for (int i = 0; i < pv_length[0]; i++) {
                move_print_UCI(pv_table[0][i]);
                std::cout << " ";
            }
            std::cout << "\n";
        }
        crnt++;
    }

    std::cout << "bestmove ";
    move_print_UCI(pv_table[0][0]);
    std::cout << "\n";
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

char *Instruction_token_next(Instruction *self) { return Instruction_token_n(self, 1); }

Move parse_move(char *move_string) {
    Square source = square_from_coordinates(move_string);
    Square target = square_from_coordinates(move_string + 2);

    const MoveList list(board);
    for (int i = 0; i < list.size(); i++) {
        const Move move = list[i];
        if (move.source() == source && move.target() == target) {
            if (move_string[4]) {
                if (tolower(move.piece_promote().code) != move_string[4]) continue;
            }
            return move;
        }
    }
    return {};
}

Board *Instruction_parse(Instruction *self) {
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
                Move move = parse_move(token);
                if (move == Move()) {
                    move.make(board, 0);
                } else {
                    printf("Invalid move %s!\n", token);
                }
            }
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
            search_position(depth);
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
        if (!Instruction_parse(instruction)) break;
        Instruction_free(&instruction);
    }

    Instruction_free(&instruction);
}

/* MAIN */

int main(void) {
    uci_loop();
    return 0;
}
