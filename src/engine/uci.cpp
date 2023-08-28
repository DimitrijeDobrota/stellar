#include <iostream>
#include <sstream>
#include <string>

#include "engine.hpp"
#include "stellar_version.hpp"
#include "uci.hpp"

namespace uci {

void move_print(const Board &board, Move move) {
    std::cout << square_to_coordinates(move.source()) << square_to_coordinates(move.target());
    if (move.is_promote()) std::cout << piece::get_code(move.promoted(), board.get_side());
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
                if (parse_move(settings.board, move, command))
                    move.make(settings.board);
                else
                    break;
            }
        } else if (command == "go") {
            while (iss >> command) {
                if (command == "wtime")
                    iss >> settings.wtime;
                else if (command == "btime")
                    iss >> settings.btime;
                else if (command == "winc")
                    iss >> settings.winc;
                else if (command == "binc")
                    iss >> settings.binc;
                else if (command == "depth")
                    iss >> settings.depth;
                else if (command == "nodes")
                    iss >> settings.nodes;
                else if (command == "movetime")
                    iss >> settings.movetime;
                else if (command == "ponder")
                    settings.ponder = true;
                else if (command == "mate")
                    settings.mate = true;
                else if (command == "infinite")
                    settings.infinite = true;
                else if (command == "searchmoves") {
                    while (iss >> command) {
                        if (parse_move(settings.board, move, command))
                            settings.searchmoves.push(move);
                        else
                            break;
                    }
                }
            }
            engine::search_position(settings);
        }
    }
}
} // namespace uci
