#include "transposition.hpp"
#include "board.hpp"
#include "score.hpp"

int TTable::read(const Board &board, int ply, Move *best, int alpha, int beta, uint8_t depth) const {

    U64 hash = board.get_hash();
    const Hashe &phashe = table[hash % table.size()];
    if (phashe.key == hash) {
        if (phashe.depth >= depth) {
            int score = phashe.score;

            if (score < -MATE_SCORE) score += ply;
            if (score > MATE_SCORE) score -= ply;

            if (phashe.flag == HasheFlag::Exact) return score;
            if ((phashe.flag == HasheFlag::Alpha) && (score <= alpha)) return alpha;
            if ((phashe.flag == HasheFlag::Beta) && (score >= beta)) return beta;
        }
        *best = phashe.best;
    }
    return TTABLE_UNKNOWN;
}

void TTable::write(const Board &board, int ply, Move best, int score, uint8_t depth, HasheFlag flag) {

    U64 hash = board.get_hash();
    Hashe &phashe = table[hash % table.size()];

    if (score < -MATE_SCORE) score += ply;
    if (score > MATE_SCORE) score -= ply;

    phashe = {hash, best, depth, score, flag};
}
