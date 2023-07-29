#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include "moves.h"
#include "perft.h"

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
