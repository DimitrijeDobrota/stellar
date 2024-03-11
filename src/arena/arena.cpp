#include <format>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include <bits/getopt_core.h>
#include <stdexcept>

#include "attack.hpp"
#include "logger.hpp"
#include "match.hpp"

class Arena {
  public:
    Arena(const Arena &) = delete;
    Arena &operator==(const Arena &) = delete;
    Arena(const char *name1, const char *name2) : engine1(new Engine(name1)), engine2(new Engine(name2)) {
        logger::log(std::format("Arena {}: created", id), logger::Debug);
    }

    ~Arena() {
        delete (engine1), delete (engine2);
        logger::log(std::format("Arena {}: destroyed", id), logger::Debug);
    }

    void operator()(const std::vector<std::string> positions, Match::Settings swhite,
                    Match::Settings sblack) {
        Match match(*engine1, *engine2);
        for (const std::string &fen : positions) {
            logger::log(
                std::format("Arena {}: match from {}", id, fen == Game::startPosition ? "startpos" : fen),
                logger::Debug);
            Arena::print(match.play(swhite, sblack, fen));
        }
    }

  private:
    static void print(const Game game) {
        mutex.lock();
        std::cout << game << std::endl;
        mutex.unlock();
    }

    static uint16_t id_t;
    uint16_t id = id_t++;

    Engine *engine1;
    Engine *engine2;

    static std::mutex mutex;
};

uint16_t Arena::id_t = 0;

std::mutex Arena::mutex;
void usage(const char *program) {
    std::cerr << program << ": ";
    std::cerr << "[-f fen]";
    std::cerr << "\n[-G wmovestogo -g bmovestogo]";
    std::cerr << "\n[-D wdepth -d bdepth]";
    std::cerr << "\n[-T wtime -t btime]";
    std::cerr << "\n[-I winc -i binc]";
    std::cerr << "\n-E engine1 -e engine2";
    std::cerr << "\n[- startpos] ... fen ...";
}

int main(int argc, char *argv[]) {
    char *engine1 = nullptr, *engine2 = nullptr;
    Match::Settings settings1, settings2;

    char c = 0;
    while ((c = getopt(argc, argv, "hE:e:D:d:T:t:I:i:G:g:N:")) != -1) {
        switch (c) {
        case 'E': engine1 = optarg; break;
        case 'e': engine2 = optarg; break;
        case 'D': settings1.depth = atoi(optarg); break;
        case 'd': settings2.depth = atoi(optarg); break;
        case 'T': settings1.time = atoll(optarg); break;
        case 't': settings2.time = atoll(optarg); break;
        case 'I': settings1.inc = atoll(optarg); break;
        case 'i': settings2.inc = atoll(optarg); break;
        case 'G': settings1.togo = atoi(optarg); break;
        case 'g': settings2.togo = atoi(optarg); break;
        case 'h': usage(argv[0]); return 1;
        default: usage(argv[0]), abort();
        }
    }

    if (!engine1 || !engine2) {
        usage(argv[0]);
        abort();
    }

    std::vector<std::string> positions;
    for (int i = optind; i < argc; i++)
        positions.emplace_back(!strcmp(argv[i], "-") ? start_position : argv[i]);

    attack::init();
    Arena arena(engine1, engine2);
    arena(positions, settings1, settings2);

    return 0;
}
