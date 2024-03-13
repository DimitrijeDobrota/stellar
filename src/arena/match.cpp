#include "match.hpp"
#include "logger.hpp"
#include "repetition.hpp"
#include "timer.hpp"
#include "utils_ui.hpp"

uint16_t Match::id_t = 0;
Match::~Match() { logger::log(std::format("Match {}: destroyed", id), logger::Debug); }
Match::Match(Engine &white, Engine &black) : engines({&white, &black}) {
    logger::log(std::format("Match {}: created", id), logger::Debug);
}

Game Match::play(Settings swhite, Settings sblack, const std::string fen = Game::startPosition) {
    const std::string position = "position " + (fen == Game::startPosition ? "startpos" : "fen " + fen);

    repetition::Table rtable;
    Board board(fen);
    Move move;

    logger::log(std::format("Match {}: Play a game between {}(white) and {}(black)", id,
                            engines[0]->get_name(), engines[1]->get_name()));
    Game game(id, engines[0]->get_name(), engines[1]->get_name(), fen);

    engines[0]->send("ucinewgame");
    engines[1]->send("ucinewgame");

    Color turn = board.get_side();
    while (true) {
        const MoveList list = MoveList(board, false, true);
        if (!list.size()) {
            game.set_winner(other(turn));
            break;
        }

        Engine *engine = engines[turn];
        engine->send(std::format("{} moves {}", position, game.get_moves()));
        engine->send(get_go(swhite, sblack, turn));

        uint64_t time_start = timer::get_ms();

        std::string response;
        while (true) {
            response = engine->receive();
            if (response.starts_with("bestmove")) break;
        }

        std::string move_str = response.substr(9);
        if ((move = parse_move(list, move_str)) == Move() || !move.make(board)) {
            logger::log(std::format("Match {}: {} illegal {}", id, to_string(turn), (std::string)move));
            game.set_terminate(Game::Illegal);
            game.set_winner(other(turn));
            break;
        }

        if (rtable.is_repetition(board.get_hash())) {
            logger::log(std::format("Match {}: {} repetition", id, to_string(turn)));
            game.set_terminate(Game::Repetition);
            game.set_draw(true);
            break;
        }

        rtable.push_hash(board.get_hash());
        if (!move.is_repeatable()) rtable.push_null();
        game.play(move);

        uint64_t time_passed = timer::get_ms() - time_start;
        if (turn == WHITE ? swhite.time <= time_passed : sblack.time <= time_passed) {
            logger::log(std::format("Match {}: {} timeout", id, to_string(turn)));
            game.set_terminate(Game::Timeout);
            game.set_winner(other(turn));
            break;
        }

        if (turn == WHITE && !swhite.depth) swhite.time -= time_passed;
        if (turn == BLACK && !sblack.depth) sblack.time -= time_passed;

        turn = other(turn);
    }

    if (!game.is_draw()) {
        logger::log(std::format("Match {}: winner is {}", id, to_string(turn)));
    } else {
        logger::log(std::format("Match {}: ended in a draw", id));
    }

    std::swap(engines[0], engines[1]);
    return game;
}

std::string Match::get_go(Settings &swhite, Settings &sblack, Color side) {
    std::string go = "go";
    if (side == WHITE && swhite.depth) go += " depth " + std::to_string(swhite.depth);
    else {
        if (side == WHITE && swhite.togo) go += " movestogo " + std::to_string(swhite.togo);
        if (!sblack.depth && swhite.time) go += " wtime " + std::to_string(swhite.time);
        if (swhite.inc) go += " winc " + std::to_string(swhite.inc);
        if (swhite.movetime) go += " movetime " + std::to_string(swhite.movetime);
    }

    if (side == BLACK && sblack.depth) go += " depth " + std::to_string(sblack.depth);
    else {
        if (side == BLACK && sblack.togo) go += " movestogo " + std::to_string(sblack.togo);
        if (!swhite.depth && sblack.time) go += " btime " + std::to_string(sblack.time);
        if (sblack.inc) go += " binc " + std::to_string(sblack.inc);
        if (swhite.movetime) go += " movetime " + std::to_string(sblack.movetime);
    }
    return go;
}

Move Match::parse_move(const MoveList list, const std::string &move_string) {
    const Square source = from_coordinates(move_string.substr(0, 2));
    const Square target = from_coordinates(move_string.substr(2, 2));

    for (int i = 0; i < list.size(); i++) {
        const Move crnt = list[i];
        if (crnt.source() != source || crnt.target() != target) continue;
        if (move_string[4] && tolower(piece::get_code(crnt.promoted())) != move_string[4]) continue;
        return crnt;
    }
    return {};
}
