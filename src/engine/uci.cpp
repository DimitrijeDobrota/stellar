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

void move_print(const Board &board, Move move) {
    std::cout << square_to_coordinates(move.source()) << square_to_coordinates(move.target());
    if (move.is_promote()) std::cout << piece::get_code(move.promoted(), board.get_side());
}

void communicate(const uci::Settings *settings) {
    if (!settings->infinite && uci::get_time_ms() > settings->stoptime) settings->stopped = true;

    /*
    if (std::cin.peek() == EOF || std::cin.peek() == '\n') {
        std::cin.clear();
        return;
    }
    std::string command;
    std::cin >> command;
    if (command == "stop" || command == "quit") settings->stopped = true;
    */
}

inline bool parse_move(const Board &board, Move &move, const std::string &move_string) {
    Square source = square_from_coordinates(move_string.substr(0, 2));
    Square target = square_from_coordinates(move_string.substr(2, 2));

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
            iss >> command;
            if (command == "startpos") {
                settings.board = Board(start_position);
            } else if (command == "fen") {
                iss >> command;
                settings.board = Board(command);
            }
            iss >> command;
            while (iss >> command) {
                if (!parse_move(settings.board, move, command)) break;
                move.make(settings.board);
            }
        } else if (command == "go") {
            uint32_t wtime = 0, btime = 0, movetime = 0;
            uint16_t winc = 0, binc = 0, movestogo = 30;

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
                        settings.searchmoves.push(move);
                    }
                }
            }

            settings.starttime = get_time_ms();
            uint32_t time = (settings.board.get_side() == Color::WHITE) ? wtime : btime;

            if (movetime != 0) {
                time = movetime;
                movestogo = 1;
            }

            if (time != 0) {
                uint16_t inc = (settings.board.get_side() == Color::WHITE) ? winc : binc;
                time /= movestogo;
                time -= 50;
                settings.stoptime = settings.starttime += time + inc;
                settings.infinite = false;
            }

            if (!time) settings.infinite = true;

            // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            engine::search_position(settings);
            settings.newgame = false;
        }
    }
}
} // namespace uci
