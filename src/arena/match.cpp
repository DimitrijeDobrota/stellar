#include "match.hpp"
#include "logger.hpp"
#include "repetition.hpp"
#include "timer.hpp"

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

    color::Color turn = board.get_side();
    while (true) {
        const MoveList list = MoveList(board, false, true);
        if (!list.size()) {
            game.set_winner(color::other(turn));
            break;
        }

        Engine *engine = engines[to_underlying(turn)];
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
            logger::log(
                std::format("Match {}: {} illegal {}", id, color::to_string(turn), (std::string)move));
            game.set_terminate(Game::Illegal);
            game.set_winner(color::other(turn));
            break;
        }

        if (rtable.is_repetition(board.get_hash())) {
            logger::log(std::format("Match {}: {} repetition", id, color::to_string(turn)));
            game.set_terminate(Game::Repetition);
            game.set_draw(true);
            break;
        }

        rtable.push_hash(board.get_hash());
        if (!move.is_repeatable()) rtable.push_null();
        game.play(move);

        uint64_t time_passed = timer::get_ms() - time_start;
        if (turn == color::WHITE ? swhite.time <= time_passed : sblack.time <= time_passed) {
            logger::log(std::format("Match {}: {} timeout", id, color::to_string(turn)));
            game.set_terminate(Game::Timeout);
            game.set_winner(color::other(turn));
            break;
        }

        if (turn == color::WHITE && !swhite.depth) swhite.time -= time_passed;
        if (turn == color::BLACK && !sblack.depth) sblack.time -= time_passed;

        turn = color::other(turn);
    }

    if (!game.is_draw()) {
        logger::log(std::format("Match {}: winner is {}", id, color::to_string(turn)));
    } else {
        logger::log(std::format("Match {}: ended in a draw", id));
    }

    std::swap(engines[0], engines[1]);
    return game;
}

std::string Match::get_go(Settings &swhite, Settings &sblack, color::Color side) {
    std::string go = "go";
    if (side == color::WHITE && swhite.depth) go += " depth " + std::to_string(swhite.depth);
    else {
        if (side == color::WHITE && swhite.togo) go += " movestogo " + std::to_string(swhite.togo);
        if (!sblack.depth && swhite.time) go += " wtime " + std::to_string(swhite.time);
        if (swhite.inc) go += " winc " + std::to_string(swhite.inc);
        if (swhite.movetime) go += " movetime " + std::to_string(swhite.movetime);
    }

    if (side == color::BLACK && sblack.depth) go += " depth " + std::to_string(sblack.depth);
    else {
        if (side == color::BLACK && sblack.togo) go += " movestogo " + std::to_string(sblack.togo);
        if (!swhite.depth && sblack.time) go += " btime " + std::to_string(sblack.time);
        if (sblack.inc) go += " binc " + std::to_string(sblack.inc);
        if (swhite.movetime) go += " movetime " + std::to_string(sblack.movetime);
    }
    return go;
}

Move Match::parse_move(const MoveList list, const std::string &move_string) {
    const square::Square source = square::from_coordinates(move_string.substr(0, 2));
    const square::Square target = square::from_coordinates(move_string.substr(2, 2));

    for (int i = 0; i < list.size(); i++) {
        const Move crnt = list[i];
        if (crnt.source() != source || crnt.target() != target) continue;
        if (move_string[4] && tolower(piece::get_code(crnt.promoted())) != move_string[4]) continue;
        return crnt;
    }
    return {};
}
