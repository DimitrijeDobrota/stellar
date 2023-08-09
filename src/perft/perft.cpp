#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "attacks.hpp"
#include "board.hpp"
#include "moves.hpp"
#include "perft.hpp"
#include "utils_cpp.hpp"
#include "zobrist.hpp"

// FEN debug positions
#define tricky_position                                                        \
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position                                                        \
    "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position                                                           \
    "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

void perft_result_print(PerftResult res) {
    printf("           - Perft Results -\n\n");
    printf("            Nodes: %llu\n", res.node);
#ifdef USE_FULL_COUNT
    printf("         Captures: %llu\n", res.capture);
    printf("       Enpassants: %llu\n", res.enpassant);
    printf("          Castles: %llu\n", res.castle);
    printf("       Promotions: %llu\n", res.promote);
    printf("           Checks: %llu\n", res.check);
    //     printf("Discovered Checks: %llu\n", res.checkDiscovered);
    //     printf("    Dobule Checks: %llu\n", res.checkDouble);
    //     printf("       Checkmates: %llu\n", res.checkmate);
#else
    printf("Other stats are disabled in this build...\n");
#endif
}

void perft_result_add(PerftResult *base, PerftResult *add) {
    base->node += add->node;
#ifdef USE_FULL_COUNT
    base->capture += add->capture;
    base->enpassant += add->enpassant;
    base->castle += add->castle;
    base->promote += add->promote;
    base->check += add->check;
    //     base->checkDiscovered += add->checkDiscovered;
    //     base->checkDouble += add->checkDouble;
    //     base->checkmate += add->checkmate;
#endif
}

typedef std::vector<MoveE> MoveList;

void perft_driver(Board &board, MoveList *moveList, int depth,
                  PerftResult *result) {
    moveList[depth] = move_list_generate(board);
    Board copy;

    for (const auto [move, _] : moveList[depth]) {
        copy = board;
        if (!move_make(move, copy, 0)) continue;

        if (depth != 1) {
            perft_driver(copy, moveList, depth - 1, result);
        } else {
            result->node++;
#ifdef USE_FULL_COUNT
            if (board_isCheck(copy)) result->check++;
            if (move_capture(move)) result->capture++;
            if (move_enpassant(move)) result->enpassant++;
            if (move_castle(move)) result->castle++;
            if (move_promote(move)) result->promote++;
#endif
        }
    }
}

typedef struct perf_shared perf_shared;
struct perf_shared {
    MoveList list;
    int depth;
    const char *fen;
    pthread_mutex_t mutex;
    unsigned int index;
    PerftResult result;
};

void *perft_thread(void *arg) {
    PerftResult result = {0};
    perf_shared *shared = (perf_shared *)arg;
    MoveList moveList[shared->depth + 1];

    Board board = Board(shared->fen), copy;

    while (1) {
        pthread_mutex_lock(&shared->mutex);
        perft_result_add(&shared->result, &result);
        if (shared->index >= shared->list.size()) {
            pthread_mutex_unlock(&shared->mutex);
            break;
        }
        Move move = shared->list[shared->index++].move;
        pthread_mutex_unlock(&shared->mutex);

        result = (PerftResult){0};

        copy = board;
        if (!move_make(move, copy, 0)) continue;

        if (shared->depth != 1) {
            perft_driver(copy, moveList, shared->depth - 1, &result);
        } else {
            result.node++;
#ifdef USE_FULL_COUNT
            if (board_isCheck(copy)) result.check++;
            if (move_capture(move)) result.capture++;
            if (move_enpassant(move)) result.enpassant++;
            if (move_castle(move)) result.castle++;
            if (move_promote(move)) result.promote++;
#endif
        }
    }
    return NULL;
}

PerftResult perft_test(const char *fen, int depth, int thread_num) {
    pthread_t threads[thread_num];
    perf_shared shared = (perf_shared){
        .list = move_list_generate(Board(fen)),
        .depth = depth,
        .fen = fen,
    };

    pthread_mutex_init(&shared.mutex, NULL);
    for (int i = 0; i < thread_num; i++)
        pthread_create(threads + i, NULL, perft_thread, (void *)(&shared));

    for (int i = 0; i < thread_num; i++)
        pthread_join(threads[i], NULL);

    return shared.result;
}

int main(int argc, char *argv[]) {

    int c, depth = 1, thread_num = 1;
    std::string s(start_position);
    const char *fen = s.data();
    while ((c = getopt(argc, argv, "t:f:d:")) != -1) {
        switch (c) {
        case 't':
            thread_num = atoi(optarg);
            if (thread_num <= 0) abort();
            break;
        case 'f':
            fen = optarg;
            break;
        case 'd':
            depth = atoi(optarg);
            if (depth <= 0) abort();
            break;
        default:
            abort();
        }
    }

    attacks_init();
    zobrist_init();

    PerftResult res = perft_test(fen, depth, thread_num);
    perft_result_print(res);
}
