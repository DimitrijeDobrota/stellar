#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "piece.hpp"
#include "repetition.hpp"
#include "score.hpp"
#include "timer.hpp"
#include "uci.hpp"
#include "utils.hpp"

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
    static inline constexpr const int16_t unknown = 32500;

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

class PVTable {
  public:
    Move best(uint8_t ply = 0) { return table[0][ply]; }

    void start(uint8_t ply) { length[ply] = ply; }
    void store(Move move, uint8_t ply) {
        table[ply][ply] = move;
        for (uint8_t i = ply + 1; i < length[ply + 1]; i++)
            table[ply][i] = table[ply + 1][i];
        length[ply] = length[ply + 1];
    }

    friend std::ostream &operator<<(std::ostream &os, const PVTable &pvtable);

  private:
    Move table[MAX_PLY][MAX_PLY] = {{}};
    uint8_t length[MAX_PLY] = {0};
};

std::ostream &operator<<(std::ostream &os, const PVTable &pvtable) {
    for (uint8_t i = 0; i < pvtable.length[0]; i++)
        os << pvtable.table[0][i] << " ";
    return os;
}

static const uci::Settings *settings = nullptr;
static Board board;
static TTable ttable;
static repetition::Table rtable;

static PVTable pvtable;

static Move killer[2][MAX_PLY];
static U32 history[12][64];
static bool follow_pv;
static U64 nodes;
static uint8_t ply;

U32 inline move_score(const Move move) {
    static constexpr const uint16_t capture[6][6] = {
        // clang-format off
        {105, 205, 305, 405, 505, 605},
        {104, 204, 304, 404, 504, 604},
        {103, 203, 303, 403, 503, 603},
        {102, 202, 302, 402, 502, 602},
        {101, 201, 301, 401, 501, 601},
        {100, 200, 300, 400, 500, 600},
        // clang-format on
    };

    const piece::Type type = board.get_square_piece_type(move.source());
    if (move.is_capture()) {
        const piece::Type captured = board.get_square_piece_type(move.target());
        return capture[to_underlying(type)][to_underlying(captured)] + 10000;
    }
    if (killer[0][ply] == move) return 9000;
    if (killer[1][ply] == move) return 8000;
    return history[piece::get_index(type, board.get_side())][to_underlying(move.target())];
}

void move_list_sort(MoveList &list, std::vector<int> &score, int crnt) {
    for (int i = crnt + 1; i < list.size(); i++) {
        if (score[crnt] < score[i]) {
            std::swap(list[crnt], list[i]);
            std::swap(score[crnt], score[i]);
        }
    }
}

std::vector<int> move_list_score(MoveList &list, const Move best) {
    std::vector<int> score(list.size(), 0);

    bool best_found = false;
    for (int i = 0; i < list.size(); i++) {
        score[i] = move_score(list[i]);
        if (list[i] == best) {
            score[i] = 30000;
            best_found = true;
        }
    }

    if (best_found) return score;

    if (ply && follow_pv) {
        follow_pv = false;
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == pvtable.best(ply)) {
                score[i] = 20000;
                follow_pv = true;
                break;
            }
        }
    }

    return score;
}

int stats_move_make(Board &copy, const Move move) {
    copy = board;
    if (!move.make(board)) {
        board = copy;
        return 0;
    }
    ply++;
    rtable.push_hash(copy.get_hash());
    if (!move.is_repeatable()) rtable.push_null();
    return 1;
}

void stats_move_make_pruning(Board &copy) {
    copy = board;
    board.switch_side();
    board.set_enpassant(square::no_sq);
    ply++;
}

void stats_move_unmake_pruning(Board &copy) {
    board = copy;
    ply--;
}

void stats_move_unmake(Board &copy, const Move move) {
    board = copy;
    if (!move.is_repeatable()) rtable.pop();
    rtable.pop();
    ply--;
}

int16_t quiescence(int16_t alpha, int16_t beta) {
    pvtable.start(ply);
    if ((nodes & 2047) == 0) {
        uci::communicate(settings);
        if (settings->stopped) return 0;
    }
    nodes++;

    int score = evaluate::score_position(board);
    if (ply > MAX_PLY - 1) return score;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;

    Board copy;
    MoveList list(board, true);
    std::vector<int> listScore = move_list_score(list, Move());
    for (int i = 0; i < list.size(); i++) {
        move_list_sort(list, listScore, i);
        const Move move = list[i];
        if (!stats_move_make(copy, move)) continue;
        score = -quiescence(-beta, -alpha);
        stats_move_unmake(copy, move);

        if (settings->stopped) return 0;
        if (score > alpha) {
            alpha = score;
            pvtable.store(move, ply);
            if (score >= beta) return beta;
        }
    }

    return alpha;
}

int16_t negamax(int16_t alpha, int16_t beta, uint8_t depth, bool null) {
    int pv_node = (beta - alpha) > 1;
    Hashe::Flag flag = Hashe::Flag::Alpha;
    int futility = 0;
    Move bestMove;
    Board copy;

    pvtable.start(ply);
    if ((nodes & 2047) == 0) {
        uci::communicate(settings);
        if (settings->stopped) return 0;
    }

    // && fifty >= 100
    if (ply && rtable.is_repetition(board.get_hash())) return 0;

    int16_t score = ttable.read(board, ply, &bestMove, alpha, beta, depth);
    if (ply && score != TTable::unknown && !pv_node) return score;

    bool isCheck = board.is_check();
    if (isCheck) depth++;

    if (depth == 0) {
        nodes++;
        int16_t score = quiescence(alpha, beta);
        // ttable_write(board, ply, bestMove, score, depth, HasheFlag::Exact);
        return score;
    }

    if (alpha < -MATE_VALUE) alpha = -MATE_VALUE;
    if (beta > MATE_VALUE - 1) beta = MATE_VALUE - 1;
    if (alpha >= beta) return alpha;
    // if (ply > MAX_PLY - 1) return evaluate::score_position(board);

    if (!pv_node && !isCheck) {
        static constexpr const U32 score_pawn = score::get(piece::Type::PAWN);
        int16_t staticEval = evaluate::score_position(board);

        // evaluation pruning
        if (depth < 3 && abs(beta - 1) > -MATE_VALUE + 100) {
            int16_t marginEval = score_pawn * depth;
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
            int16_t scoreNew = quiescence(alpha, beta);

            if (score < beta && depth == 1) {
                return (scoreNew > score) ? scoreNew : score;
            }

            score += score_pawn;
            if (score < beta && depth < 4) {
                if (scoreNew < beta) return (scoreNew > score) ? scoreNew : score;
            }
        }

        // futility pruning condition
        static constexpr const int16_t margin[] = {
            0,
            score::get(piece::Type::PAWN),
            score::get(piece::Type::KNIGHT),
            score::get(piece::Type::ROOK),
        };
        if (depth < 4 && abs(alpha) < MATE_SCORE && staticEval + margin[depth] <= alpha) futility = 1;
    }

    uint8_t legal_moves = 0;
    uint8_t searched = 0;

    MoveList list(board);
    std::vector<int> listScore = move_list_score(list, bestMove);
    for (int i = 0; i < list.size(); i++) {
        move_list_sort(list, listScore, i);
        const Move move = list[i];
        if (!stats_move_make(copy, move)) continue;
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
            pvtable.store(move, ply);

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
        if (isCheck) return -MATE_VALUE + ply;
        else
            return 0;
    }

    ttable.write(board, ply, bestMove, alpha, depth, flag);
    return alpha;
}

Move search_position(const uci::Settings &settingsr) {
    int16_t alpha = -SCORE_INFINITY, beta = SCORE_INFINITY;
    settings = &settingsr;

    if (settings->newgame) {
        ttable = TTable(C64(0x2FB4377));
    }

    rtable.clear();
    board = settings->board;
    for (int i = 0; i < settings->madeMoves.size(); i++) {
        rtable.push_hash(board.get_hash());
        settings->madeMoves[i].make(board);
        if (!settings->madeMoves[i].is_repeatable()) rtable.clear();
    }

    ply = 0;
    nodes = 0;
    settings->stopped = false;
    memset(killer, 0x00, sizeof(killer));
    memset(history, 0x00, sizeof(history));
    rtable = repetition::Table();

    Move lastBest;

    uint64_t time_last = timer::get_ms();
    uint8_t max_depth = settings->depth ? settings->depth : MAX_PLY;
    for (uint8_t depth = 1; depth <= max_depth; depth++) {
        lastBest = pvtable.best();
        follow_pv = 1;
        int16_t score = negamax(alpha, beta, depth, true);

        uci::communicate(settings);
        if (settings->stopped) break;

        if ((score <= alpha) || (score >= beta)) {
            alpha = -SCORE_INFINITY;
            beta = SCORE_INFINITY;
            depth--;
            continue;
        }

        alpha = score - WINDOW;
        beta = score + WINDOW;

        uint8_t mate_ply = 0xFF;
        if (score > -MATE_VALUE && score < -MATE_SCORE) {
            mate_ply = (score + MATE_VALUE) / 2 + 1;
            std::cout << "info score mate -" << (int)mate_ply;
        } else if (score > MATE_SCORE && score < MATE_VALUE) {
            mate_ply = (MATE_VALUE - score) / 2 + 1;
            std::cout << "info score mate " << (int)mate_ply;
        } else {
            std::cout << "info score cp " << score;
        }

        std::cout << " depth " << (unsigned)depth;
        std::cout << " nodes " << nodes;
        std::cout << " time " << timer::get_ms() - settings->starttime;
        std::cout << " pv " << pvtable << std::endl;

        if (depth >= mate_ply) break;

        uint64_t time_crnt = timer::get_ms();
        if (!settings->depth && 2 * time_crnt - time_last > settings->stoptime) break;
        time_last = time_crnt;
    }

    settings->board = board;
    return !settings->stopped ? pvtable.best() : lastBest;
}
} // namespace engine

int main(void) {
    uci::loop();
    return 0;
}
