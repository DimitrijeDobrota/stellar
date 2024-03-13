#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "engine.hpp"
#include "stellar_version.hpp"
#include "timer.hpp"
#include "uci.hpp"

namespace uci {

void communicate(const uci::Settings *settings) {
    if (!settings->infinite && timer::get_ms() > settings->stoptime) {
        settings->stopped = true;
        return;
    }
}

inline bool parse_move(const Board &board, Move &move, const std::string &move_string) {
    const Square source = from_coordinates(move_string.substr(0, 2));
    const Square target = from_coordinates(move_string.substr(2, 2));

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

void loop() {
    static Settings settings;
    static std::string line, command;
    static Move move;

    Board board;

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

            board = settings.board;
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
                if (command == "wtime") iss >> wtime;
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
                        if (!parse_move(board, move, command)) break;
                        settings.searchMoves.push(move);
                    }
                }
            }

            settings.starttime = timer::get_ms();
            uint64_t time = (board.get_side() == WHITE) ? wtime : btime;

            if (movetime != 0) {
                time = movetime;
                movestogo = 1;
            } else if (time != 0) {
                uint16_t inc = (board.get_side() == WHITE) ? winc : binc;
                time /= movestogo;
                time -= 50;
                settings.stoptime = settings.starttime + time + inc;
                settings.infinite = false;
            } else
                settings.infinite = true;

            const Move best = engine::search_position(settings);
            std::cout << "bestmove " << best << '\n';

            settings.newgame = false;
        }
    }
}
} // namespace uci
