#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include "moves.h"
#include "perft.h"

struct MoveList_T moveList[10];
long nodes;

void perft_driver(CBoard_T board, struct MoveList_T *moveList, int depth,
                  unsigned long long *nodes) {
    if (depth == 0) {
        (*nodes)++;
        return;
    }

    MoveList_T list = MoveList_generate(&moveList[depth], board);
    CBoard_T copy = CBoard_new();

    for (int i = 0; i < MoveList_size(list); i++) {
        CBoard_copy(board, copy);
        if (!Move_make(MoveList_move(list, i), copy, 0)) continue;
        perft_driver(copy, moveList, depth - 1, nodes);
    }

    MoveList_reset(list);
    CBoard_free(&copy);
}

void perft_test(CBoard_T board, int depth) {
    MoveList_T list = MoveList_generate(&moveList[depth], board);
    CBoard_T copy = CBoard_new();
    long start = get_time_ms();

    printf("\n     Performance test\n\n");

    nodes = 0;
    for (int i = 0; i < MoveList_size(list); i++) {
        CBoard_copy(board, copy);
        Move move = MoveList_move(list, i);
        if (!Move_make(MoveList_move(list, i), copy, 0)) continue;
        unsigned long long node = 0;
        perft_driver(copy, moveList, depth - 1, &node);
        printf("%s%s: %llu\n", square_to_coordinates[Move_source(move)],
               square_to_coordinates[Move_target(move)], node);
        nodes += node;
    }
    MoveList_reset(list);
    CBoard_free(&copy);

    printf("\nNodes searched: %ld\n\n", nodes);
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %ld\n", nodes);
    printf("     Time: %ld\n\n", get_time_ms() - start);
}

typedef struct perf_shared perf_shared;
struct perf_shared {
    CBoard_T board;
    MoveList_T list;
    int depth;
    sem_t *mutex;
    int *index;
    sem_t *finish;
    unsigned long long *total;
};

void *perft_thread(void *arg) {
    perf_shared *shared = (perf_shared *)arg;
    CBoard_T board = CBoard_new();
    unsigned long long node_count = 0;

    struct MoveList_T moveList[10];

    while (1) {
        sem_wait(shared->mutex);
        *shared->total += node_count;
        if (*shared->index >= MoveList_size(shared->list)) {
            sem_post(shared->mutex);
            break;
        }
        Move move = MoveList_move(shared->list, (*shared->index)++);
        sem_post(shared->mutex);

        CBoard_copy(shared->board, board);
        if (!Move_make(move, board, 0)) continue;

        node_count = 0;
        perft_driver(board, moveList, shared->depth, &node_count);
        printf("%s%s: %llu\n", square_to_coordinates[Move_source(move)],
               square_to_coordinates[Move_target(move)], node_count);
    }
    CBoard_free(&board);
    sem_post(shared->finish);
    return NULL;
}

void perft_test_threaded(CBoard_T board, int depth) {
    MoveList_T list = MoveList_generate(NULL, board);
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
    MoveList_free(&list);

    printf("Nodes processed: %llu\n", total);
}
