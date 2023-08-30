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
#include "uci.hpp"
#include "utils.hpp"

#define MAX_PLY 64
#define FULL_DEPTH 4
#define REDUCTION_LIMIT 3
#define REDUCTION_MOVE 2

#define WINDOW 50

namespace engine {

struct Hashe {
    enum class Flag : uint8_t {
        Exact,
        Alpha,
        Beta
    };
    U64 key;
    Move best;
    uint8_t depth;
    int16_t score;
    Flag flag;
};

class TTable {
  public:
    static inline constexpr const uint16_t unknown = 32500;

    TTable() {}
    TTable(U64 size) : table(size, {0}) {}

    void clear() { table.clear(); };
    int16_t read(const Board &board, int ply, Move *best, int16_t alpha, int16_t beta, uint8_t depth) const {
        U64 hash = board.get_hash();
        const Hashe &phashe = table[hash % table.size()];
        if (phashe.key == hash) {
            if (phashe.depth >= depth) {
                int16_t score = phashe.score;

                if (score < -MATE_SCORE) score += ply;
                if (score > MATE_SCORE) score -= ply;

                if (phashe.flag == Hashe::Flag::Exact) return score;
                if ((phashe.flag == Hashe::Flag::Alpha) && (score <= alpha)) return alpha;
                if ((phashe.flag == Hashe::Flag::Beta) && (score >= beta)) return beta;
            }
            *best = phashe.best;
        }
        return unknown;
    }

    void write(const Board &board, int ply, Move best, int16_t score, uint8_t depth, Hashe::Flag flag) {
        U64 hash = board.get_hash();
        Hashe &phashe = table[hash % table.size()];

        if (score < -MATE_SCORE) score += ply;
        if (score > MATE_SCORE) score -= ply;

        phashe = {hash, best, depth, score, flag};
    }

  private:
    std::vector<Hashe> table;
};

class RTable {
  public:
    RTable() = default;

    bool is_repetition(const U64 hash) const {
        for (int i = repetitions.size() - 1; i >= 0; i--) {
            if (repetitions[i] == hash) return true;
            if (repetitions[i] == hashNull) return false;
        }
        return false;
    }

    void pop(void) { repetitions.pop_back(); }
    void clear(void) { repetitions.clear(); }
    void push_null(void) { repetitions.push_back(hashNull); }
    void push_hash(U64 hash) { repetitions.push_back(hash); }

    friend std::ostream &operator<<(std::ostream &os, const RTable &rtable) {
        for (const U64 hash : rtable.repetitions)
            os << hash << " ";
        return os;
    }

  private:
    std::vector<U64> repetitions;

    const static int hashNull = 0;
};

static const uci::Settings *settings = nullptr;
static Board board;
static TTable ttable;
static RTable rtable;
static Move pv_table[MAX_PLY][MAX_PLY];
static Move killer[2][MAX_PLY];
static U32 history[12][64];
static int pv_length[MAX_PLY];
static bool follow_pv;
static U64 nodes;
static U32 ply;

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

std::vector<int> move_list_sort(MoveList &list, const Move best) {
    static std::vector<int> score(256);

    bool best_found = false;
    for (int i = 0; i < list.size(); i++) {
        score[i] = move_list_score(list[i]);
        if (list[i] == best) {
            score[i] = 30000;
            best_found = true;
        }
    }

    if (!best_found && ply && follow_pv) {
        follow_pv = false;
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == pv_table[0][ply]) {
                score[i] = 20000;
                follow_pv = true;
                break;
            }
        }
    }

    std::vector<int> index(list.size());
    std::iota(begin(index), end(index), 0);
    sort(begin(index), end(index), [&](int a, int b) { return score[a] > score[b]; });
    return index;
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

int stats_move_make(Board &copy, const Move move, int flag) {
    copy = board;
    if (!move.make(board, flag)) {
        board = copy;
        return 0;
    }
    ply++;
    rtable.push_hash(copy.get_hash());
    if (!move.is_repetable()) rtable.push_null();
    return 1;
}

void stats_move_make_pruning(Board &copy) {
    copy = board;
    board.switch_side();
    board.set_enpassant(Square::no_sq);
    ply++;
}

void stats_move_unmake_pruning(Board &copy) {
    board = copy;
    ply--;
}

void stats_move_unmake(Board &copy, const Move move) {
    board = copy;
    if (!move.is_repetable()) rtable.pop();
    rtable.pop();
    ply--;
}

void stats_pv_store(Move move) {
    pv_table[ply][ply] = move;
    for (int i = ply + 1; i < pv_length[ply + 1]; i++) {
        pv_table[ply][i] = pv_table[ply + 1][i];
    }
    pv_length[ply] = pv_length[ply + 1];
}

int quiescence(int16_t alpha, int16_t beta) {
    if ((nodes & 2047) == 0) {
        uci::communicate(settings);
        if (settings->stopped) return 0;
    }
    pv_length[ply] = ply;
    nodes++;

    int score = evaluate(board);
    if (ply > MAX_PLY - 1) return score;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;

    Board copy;

    MoveList list(board);
    for (int idx : move_list_sort(list, Move())) {
        const Move move = list[idx];
        if (!stats_move_make(copy, list[idx], 1)) continue;
        score = -quiescence(-beta, -alpha);
        stats_move_unmake(copy, move);

        if (score > alpha) {
            alpha = score;
            stats_pv_store(move);
            if (score >= beta) return beta;
        }

        if (settings->stopped) return 0;
    }

    return alpha;
}

int negamax(int16_t alpha, int16_t beta, uint8_t depth, bool null) {
    int pv_node = (beta - alpha) > 1;
    Hashe::Flag flag = Hashe::Flag::Alpha;
    int futility = 0;
    Move bestMove;
    Board copy;

    if ((nodes & 2047) == 0) {
        uci::communicate(settings);
        if (settings->stopped) return 0;
    }
    pv_length[ply] = ply;

    int score = ttable.read(board, ply, &bestMove, alpha, beta, depth);
    if (ply && score != TTable::unknown && !pv_node) return score;

    // && fifty >= 100
    if (ply && rtable.is_repetition(board.get_hash())) return 0;
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

        if (settings->stopped) return 0;

        if (null) {
            // null move pruning
            if (ply && depth > 2 && staticEval >= beta) {
                stats_move_make_pruning(copy);
                score = -negamax(-beta, -beta + 1, depth - 1 - REDUCTION_MOVE, false);
                stats_move_unmake_pruning(copy);
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

    MoveList list(board);
    for (int idx : move_list_sort(list, bestMove)) {
        const Move move = list[idx];
        if (!stats_move_make(copy, move, 0)) continue;
        legal_moves++;

        // futility pruning
        if (futility && searched && !move.is_capture() && !move.is_promote() && !board.is_check()) {
            stats_move_unmake(copy, move);
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

        stats_move_unmake(copy, move);
        searched++;

        if (settings->stopped) return 0;
        if (score > alpha) {
            if (!move.is_capture()) {
                const piece::Type piece = board.get_square_piece_type(move.source());
                history[piece::get_index(piece, board.get_side())][to_underlying(move.target())] += depth;
            }

            alpha = score;
            flag = Hashe::Flag::Exact;
            bestMove = move;
            stats_pv_store(move);

            if (score >= beta) {
                ttable.write(board, ply, bestMove, beta, depth, Hashe::Flag::Beta);

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

void search_position(const uci::Settings &settingsr) {
    int16_t alpha = -SCORE_INFINITY, beta = SCORE_INFINITY;
    settings = &settingsr;

    if (settings->newgame) {
        ttable = TTable(C64(0x1000000));
    }

    rtable.clear();
    board = settings->board;
    for (int i = 0; i < settings->madeMoves.size(); i++) {
        rtable.push_hash(board.get_hash());
        settings->madeMoves[i].make(board);
        if (!settings->madeMoves[i].is_repetable()) rtable.push_null();
    }

    ply = 0;
    nodes = 0;
    settings->stopped = false;
    memset(killer, 0x00, sizeof(killer));
    memset(history, 0x00, sizeof(history));
    memset(pv_table, 0x00, sizeof(pv_table));
    memset(pv_length, 0x00, sizeof(pv_length));

    for (uint8_t depth_crnt = 1; depth_crnt <= settings->depth;) {
        uci::communicate(settings);
        if (settings->stopped) break;

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
