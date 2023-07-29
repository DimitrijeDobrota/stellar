#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include "attack.h"
#include "board.h"
#include "moves.h"
#include "perft.h"
#include "utils.h"

struct MoveList moveList[10];
long nodes;

void perft_driver(Board board, struct MoveList *moveList, int depth,
                  unsigned long long *nodes) {
    if (depth == 0) {
        (*nodes)++;
        return;
    }

    MoveList list = move_list_generate(&moveList[depth], board);
    Board copy = board_new();

    for (int i = 0; i < move_list_size(list); i++) {
        board_copy(board, copy);
        if (!move_make(move_list_move(list, i), copy, 0)) continue;
        perft_driver(copy, moveList, depth - 1, nodes);
    }

    move_list_reset(list);
    board_free(&copy);
}

void perft_test(Board board, int depth) {
    MoveList list = move_list_generate(&moveList[depth], board);
    Board copy = board_new();
    long start = get_time_ms();

    printf("\n     Performance test\n\n");

    nodes = 0;
    for (int i = 0; i < move_list_size(list); i++) {
        board_copy(board, copy);
        Move move = move_list_move(list, i);
        if (!move_make(move_list_move(list, i), copy, 0)) continue;
        unsigned long long node = 0;
        perft_driver(copy, moveList, depth - 1, &node);
        printf("%s%s: %llu\n", square_to_coordinates[move_source(move)],
               square_to_coordinates[move_target(move)], node);
        nodes += node;
    }
    move_list_reset(list);
    board_free(&copy);

    printf("\nNodes searched: %ld\n\n", nodes);
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %ld\n", nodes);
    printf("     Time: %ld\n\n", get_time_ms() - start);
}

typedef struct perf_shared perf_shared;
struct perf_shared {
    Board board;
    MoveList list;
    int depth;
    sem_t *mutex;
    int *index;
    sem_t *finish;
    unsigned long long *total;
};

void *perft_thread(void *arg) {
    perf_shared *shared = (perf_shared *)arg;
    Board board = board_new();
    unsigned long long node_count = 0;

    struct MoveList moveList[10];

    while (1) {
        sem_wait(shared->mutex);
        *shared->total += node_count;
        if (*shared->index >= move_list_size(shared->list)) {
            sem_post(shared->mutex);
            break;
        }
        Move move = move_list_move(shared->list, (*shared->index)++);
        sem_post(shared->mutex);

        board_copy(shared->board, board);
        if (!move_make(move, board, 0)) continue;

        node_count = 0;
        perft_driver(board, moveList, shared->depth, &node_count);
        printf("%s%s: %llu\n", square_to_coordinates[move_source(move)],
               square_to_coordinates[move_target(move)], node_count);
    }
    board_free(&board);
    sem_post(shared->finish);
    return NULL;
}

void perft_test_threaded(Board board, int depth) {
    MoveList list = move_list_generate(NULL, board);
    int size = 8;

    unsigned long long total = 0;
    perf_shared shared[size];
    pthread_t threads[size];
    sem_t mutex, finish;

    int index = 0;
    sem_init(&mutex, 0, 1);
    sem_init(&finish, 0, 0);
    for (int i = 0; i < size; i++) {
        shared[i].board = board;
        shared[i].list = list;
        shared[i].depth = depth - 1;
        shared[i].mutex = &mutex;
        shared[i].index = &index;
        shared[i].finish = &finish;
        shared[i].total = &total;
        pthread_create(threads + i, NULL, perft_thread, (void *)(shared + i));
    }

    for (int i = 0; i < size; i++)
        sem_wait(&finish);
    move_list_free(&list);

    printf("Nodes processed: %llu\n", total);
}

// FEN debug positions
#define tricky_position                                                        \
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position                                                        \
    "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position                                                           \
    "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

int main(void) {
    init_attacks();
    Board board = board_new();
    board_from_FEN(board, tricky_position);
    perft_test_threaded(board, 5);
}
