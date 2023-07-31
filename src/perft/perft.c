#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cul/assert.h>

#include "attacks.h"
#include "board.h"
#include "moves.h"
#include "perft.h"
#include "utils.h"

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

void perft_driver(Board board, struct MoveList *moveList, int depth,
                  PerftResult *result) {
    MoveList list = move_list_generate(&moveList[depth], board);
    Board copy = board_new();

    for (int i = 0; i < move_list_size(list); i++) {
        Move move = move_list_move(list, i);
        board_copy(board, copy);
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

    move_list_reset(list);
    board_free(&copy);
}

typedef struct perf_shared perf_shared;
struct perf_shared {
    const char *fen;
    MoveList list;
    int depth;
    pthread_mutex_t mutex;
    unsigned int index;
    PerftResult result;
};

void *perft_thread(void *arg) {
    PerftResult result = {0};
    perf_shared *shared = (perf_shared *)arg;
    struct MoveList moveList[shared->depth + 1];

    Board board = board_from_FEN(NULL, shared->fen);
    Board copy = board_new();

    while (1) {
        pthread_mutex_lock(&shared->mutex);
        perft_result_add(&shared->result, &result);
        if (shared->index >= move_list_size(shared->list)) {
            pthread_mutex_unlock(&shared->mutex);
            break;
        }
        Move move = move_list_move(shared->list, shared->index++);
        pthread_mutex_unlock(&shared->mutex);

        result = (PerftResult){0};

        board_copy(board, copy);
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
    board_free(&board);
    return NULL;
}

PerftResult perft_test(const char *fen, int depth, int thread_num) {
    assert(fen);
    assert(depth > 0);

    pthread_t threads[thread_num];
    perf_shared shared = (perf_shared){
        .list = move_list_generate(NULL, board_from_FEN(NULL, fen)),
        .depth = depth,
        .fen = fen,
    };

    pthread_mutex_init(&shared.mutex, NULL);
    for (int i = 0; i < thread_num; i++)
        pthread_create(threads + i, NULL, perft_thread, (void *)(&shared));

    for (int i = 0; i < thread_num; i++)
        pthread_join(threads[i], NULL);

    move_list_free(&shared.list);
    return shared.result;
}

int main(int argc, char *argv[]) {

    int c, depth = 1, thread_num = 1;
    char *fen = start_position;
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
    PerftResult res = perft_test(fen, depth, thread_num);
    perft_result_print(res);
}
