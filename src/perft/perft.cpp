#include <iomanip>
#include <semaphore>
#include <thread>

#include "attack.hpp"
#include "board.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "utils.hpp"

// FEN debug positions
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "

enum {
    THREAD_MAX = 64
};

class Perft {
  public:
    using semaphore_t = std::counting_semaphore<THREAD_MAX>;
    Perft(semaphore_t &sem) : sem(sem) {}
    void operator()(const Board &board_start, Move move, int depth) {
        Board board = board_start;
        if (!move.make(board)) return;
        sem.acquire();
        // debug(board_start, move, board);

        if (depth > 1) {
            test(board, depth - 1);
        } else {
            score(board, move);
        }

        mutex.acquire();
        result += local;
        mutex.release();
        sem.release();
    }

    struct result_t {
        U64 node = 0;
#ifdef USE_FULL_COUNT
        U64 check = 0;
        U64 castle = 0;
        U64 promote = 0;
        U64 capture = 0;
        U64 enpassant = 0;
#endif
        result_t &operator+=(const result_t res) {
            node += res.node;
#ifdef USE_FULL_COUNT
            check += res.check;
            castle += res.castle;
            promote += res.promote;
            capture += res.capture;
            enpassant += res.enpassant;
#endif
            return *this;
        }
    };

    static result_t result;

  private:
    void test(const Board &board, int depth) {
        const MoveList list(board);
        for (int i = 0; i < list.size(); i++) {
            Board copy = board;
            if (!list[i].make(copy)) continue;
            // debug(board, list[i], copy);
            if (depth != 1) test(copy, depth - 1);
            else
                score(copy, list[i]);
        }
    }

    void debug(const Board &before, Move move, const Board &after) {
        std::cout << std::setw(16) << std::hex << before.get_hash() << " ";
        std::cout << move << " ";
        std::cout << std::setw(16) << std::hex << after.get_hash() << "\n";
    }

    void score(const Board &board, Move move) {
        local.node++;
#ifdef USE_FULL_COUNT
        if (board.is_check()) local.check++;
        if (move.is_capture()) local.capture++;
        if (move.is_enpassant()) local.enpassant++;
        if (move.is_castle()) local.castle++;
        if (move.is_promote()) local.promote++;
#endif
    }
    result_t local;
    semaphore_t &sem;
    static std::binary_semaphore mutex;
};

std::binary_semaphore Perft::mutex{1};
Perft::result_t Perft::result;

void perft_test(const char *fen, int depth, int thread_num) {
    const Board board = Board(fen);
    const MoveList list = MoveList(board);
    std::vector<std::thread> threads(list.size());

    Perft::semaphore_t sem(thread_num);

    int index = 0;
    for (int i = 0; i < list.size(); i++)
        threads[index++] = std::thread(Perft(sem), board, list[i], depth);

    for (auto &thread : threads)
        thread.join();

    std::cout << std::dec;
    std::cout << "     Nodes: " << Perft::result.node << "\n";
#ifdef USE_FULL_COUNT
    std::cout << "  Captures: " << Perft::result.capture << "\n";
    std::cout << "Enpassants: " << Perft::result.enpassant << "\n";
    std::cout << "   Castles: " << Perft::result.castle << "\n";
    std::cout << "Promotions: " << Perft::result.promote << "\n";
    std::cout << "    Checks: " << Perft::result.check << "\n";
#endif
}

void usage(const char *program) {
    std::cout << "Usage: " << program;
    std::cout << " [-h]";
    std::cout << " [-t thread number]";
    std::cout << " [-d depth]";
    std::cout << " [-f fen]" << std::endl;
}

int main(int argc, char *argv[]) {
    int c = 0, depth = 1, thread_num = 1;
    std::string s(start_position);
    const char *fen = s.data();
    while ((c = getopt(argc, argv, "ht:f:d:")) != -1) {
        switch (c) {
        case 't':
            thread_num = atoi(optarg);
            if (thread_num <= 0 && thread_num > THREAD_MAX) abort();
            break;
        case 'f': fen = optarg; break;
        case 'd':
            depth = atoi(optarg);
            if (depth <= 0) abort();
            break;
        case 'h': usage(argv[0]); return 1;
        default: usage(argv[0]); abort();
        }
    }

    attack::init();
    zobrist::init();
    perft_test(fen, depth, thread_num);
    return 0;
}
