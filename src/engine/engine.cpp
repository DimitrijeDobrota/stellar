#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack.hpp"
#include "board.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "piece.hpp"
#include "score.hpp"
#include "transposition.hpp"
#include "uci.hpp"
#include "utils.hpp"

#define MAX_PLY 64
#define FULL_DEPTH 4
#define REDUCTION_LIMIT 3
#define REDUCTION_MOVE 2

#define WINDOW 50

namespace engine {

Board board;
TTable ttable(C64(0x400000));
Move pv_table[MAX_PLY][MAX_PLY];
Move killer[2][MAX_PLY];
U32 history[12][64];
int pv_length[MAX_PLY];
bool follow_pv;
U64 nodes;
U32 ply;

U32 inline move_list_score(Move move) {
    const piece::Type type = board.get_square_piece_type(move.source());
    if (move.is_capture()) {
        const piece::Type captured = board.get_square_piece_type(move.target());
        return piece::score(type, captured) + 10000;
    }
    if (killer[0][ply] == move) return 9000;
    if (killer[1][ply] == move) return 8000;
    return history[piece::get_index(type, board.get_side())][to_underlying(move.target())];
}

Move move_list_best_move;
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

int negamax(int alpha, int beta, uint8_t depth, bool null) {
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
                const piece::Type piece = board.get_square_piece_type(move.source());
                history[piece::get_index(piece, board.get_side())][to_underlying(move.target())] += depth;
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

void search_position(const uci::Settings &setting) {
    int alpha = -SCORE_INFINITY, beta = SCORE_INFINITY;

    nodes = 0;
    board = setting.board;
    for (uint8_t depth_crnt = 1; depth_crnt <= setting.depth;) {
        follow_pv = 1;

        int score = negamax(alpha, beta, depth_crnt, true);
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
                std::cout << "info score cp " << score;
            }

            std::cout << " depth " << (unsigned)depth_crnt;
            std::cout << " nodes " << nodes;
            std::cout << " pv ";
            for (int i = 0; i < pv_length[0]; i++) {
                uci::move_print(board, pv_table[0][i]);
                std::cout << " ";
            }
            std::cout << "\n";
        }
        depth_crnt++;
    }

    std::cout << "bestmove ";
    uci::move_print(board, pv_table[0][0]);
    std::cout << "\n";
}
} // namespace engine

int main(void) {
    uci::loop();
    return 0;
}
