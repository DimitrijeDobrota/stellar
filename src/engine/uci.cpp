#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <sys/time.h>

#include "engine.hpp"
#include "stellar_version.hpp"
#include "uci.hpp"

namespace uci {

uint32_t get_time_ms(void) {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

void pv_print(int16_t score, uint8_t depth, uint64_t nodes, const engine::PVTable &pvtable) {
    if (score > -MATE_VALUE && score < -MATE_SCORE) {
        std::cout << "info score mate " << -(score + MATE_VALUE) / 2 - 1;
    } else if (score > MATE_SCORE && score < MATE_VALUE) {
        std::cout << "info score mate " << (MATE_VALUE - score) / 2 + 1;
    } else {
        std::cout << "info score cp " << score;
    }

    std::cout << " depth " << (unsigned)depth;
    std::cout << " nodes " << nodes;
    std::cout << " pv " << pvtable << "\n";
}

void communicate(const uci::Settings *settings) {
    if (!settings->infinite && uci::get_time_ms() > settings->stoptime) {
        settings->stopped = true;
        return;
    }
}

inline bool parse_move(const Board &board, Move &move, const std::string &move_string) {
    const square::Square source = square::from_coordinates(move_string.substr(0, 2));
    const square::Square target = square::from_coordinates(move_string.substr(2, 2));

    const MoveList list(board);
    for (int i = 0; i < list.size(); i++) {
        const Move crnt = list[i];
        if (crnt.source() == source && crnt.target() == target) {
            if (move_string[4]) {
                if (tolower(piece::get_code(crnt.promoted())) != move_string[4]) continue;
            }
            move = crnt;
            return true;
        }
    }
    return false;
}

void loop(void) {
    static Settings settings;
    static std::string line, command;
    static Move move;

    while (true) {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        iss >> command;
        if (command == "quit") {
            break;
        } else if (command == "uci") {
            std::cout << "id name Stellar " << getStellarVersion() << "\n";
            std::cout << "id author Dimitrije Dobrota\n";
            std::cout << "uciok\n";
        } else if (command == "debug") {
            iss >> command;
            settings.debug = (command == "on");
        } else if (command == "isready") {
            std::cout << "readyok\n";
        } else if (command == "ucinewgame") {
            settings = Settings();
            settings.board = Board(start_position);
        } else if (command == "position") {
            settings.madeMoves.clear();
            iss >> command;
            if (command == "startpos") {
                settings.board = Board(start_position);
            } else if (command == "fen") {
                iss >> command;
                settings.board = Board(line.substr(13));
            }

            while (iss >> command)
                if (command == "moves") break;

            Board board = settings.board;
            while (iss >> command) {
                if (!parse_move(board, move, command)) break;
                settings.madeMoves.push(move);
                move.make(board);
            }
        } else if (command == "go") {
            settings.searchMoves.clear();
            uint64_t wtime = 0, btime = 0, movetime = 0;
            uint16_t winc = 0, binc = 0, movestogo = 60;

            while (iss >> command) {
                if (command == "wtime")
                    iss >> wtime;
                else if (command == "btime")
                    iss >> btime;
                else if (command == "winc")
                    iss >> winc;
                else if (command == "binc")
                    iss >> binc;
                else if (command == "depth")
                    iss >> settings.depth;
                else if (command == "nodes")
                    iss >> settings.nodes;
                else if (command == "movetime")
                    iss >> movetime;
                else if (command == "movestogo")
                    iss >> movestogo;
                else if (command == "ponder")
                    settings.ponder = true;
                else if (command == "mate")
                    settings.mate = true;
                else if (command == "infinite")
                    settings.infinite = true;
                else if (command == "searchmoves") {
                    while (iss >> command) {
                        if (!parse_move(settings.board, move, command)) break;
                        settings.searchMoves.push(move);
                    }
                }
            }

            settings.starttime = get_time_ms();
            uint32_t time = (settings.board.get_side() == color::WHITE) ? wtime : btime;

            if (movetime != 0) {
                time = movetime;
                movestogo = 1;
            }

            if (time != 0) {
                uint16_t inc = (settings.board.get_side() == color::WHITE) ? winc : binc;
                time /= movestogo;
                time -= 50;
                settings.stoptime = settings.starttime + time + inc;
                settings.infinite = false;
            }

            if (!time) settings.infinite = true;

            const Move best = engine::search_position(settings);
            std::cout << "bestmove " << best << '\n';

            settings.newgame = false;
        }
    }
}
} // namespace uci
