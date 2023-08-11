#include "transposition.hpp"
#include "board.hpp"
#include "score.hpp"

int TTable::read(const Stats &stats, Move *best, int alpha, int beta, int depth) const {

    U64 hash = stats.board.get_hash();
    const Hashe &phashe = table[hash % table.size()];
    if (phashe.key == hash) {
        if (phashe.depth >= depth) {
            int score = phashe.score;

            if (score < -MATE_SCORE) score += stats.ply;
            if (score > MATE_SCORE) score -= stats.ply;

            if (phashe.flag == HasheFlag::Exact) return score;
            if ((phashe.flag == HasheFlag::Alpha) && (score <= alpha)) return alpha;
            if ((phashe.flag == HasheFlag::Beta) && (score >= beta)) return beta;
        }
        *best = phashe.best;
    }
    return TTABLE_UNKNOWN;
}

void TTable::write(const Stats &stats, Move best, int score, int depth, HasheFlag flag) {

    U64 hash = stats.board.get_hash();
    Hashe &phashe = table[hash % table.size()];

    if (score < -MATE_SCORE) score += stats.ply;
    if (score > MATE_SCORE) score -= stats.ply;

    phashe = {hash, best, depth, score, flag};
}
