#include "game.hpp"

#include "board.hpp"
#include "logger.hpp"
#include "timer.hpp"
#include "utils_ui.hpp"

#include <format>

bool Game::san = true;
const std::string Game::startPosition = start_position;

uint16_t Game::id_t = 0;
Game::~Game() { logger::log(std::format("Game {}: destroyed", id), logger::Debug); }
Game::Game(const uint16_t match_id, const std::string &white, const std::string &black,
           const std::string &fen)
    : match_id(match_id), white(white), black(black), fen(fen) {
    logger::log(std::format("Game {}: started", id), logger::Debug);
}

const std::string Game::get_moves() const {
    std::string res;
    if (list.size()) res += (std::string)list[0];
    for (int i = 1; i < list.size(); i++)
        res += " " + (std::string)list[i];
    return res;
}

const std::string Game::to_san(const Board &board, const Move move) {
    Board copy = board;
    if (!move.make(copy)) {
        logger::log("illegal move", logger::Critical);
        throw std::runtime_error("illegal move");
    }

    if (move.is_castle_king()) return "O-O";
    if (move.is_castle_queen()) return "O-O-O";

    const Type piece = board.get_square_piece_type(move.source());
    const Type target = board.get_square_piece_type(move.target());

    std::string res;
    if (piece != PAWN) {
        U64 potential = board.get_bitboard_square_land(move.target(), piece, board.get_side());

        if (bit::count(potential) > 1) {
            int file[9] = {0}, rank[9] = {0};
            uint8_t square_i = 0;
            bitboard_for_each_bit(square_i, potential) {
                const std::string crd = to_coordinates(static_cast<Square>(square_i));
                file[crd[0] & 0x3]++;
                rank[crd[1] & 0x3]++;
            }

            const std::string crd = to_coordinates(move.source());
            if (file[crd[0] & 0x3] == 1) res += crd[0];
            else if (rank[crd[1] & 0x3] == 1)
                res += crd[1];
            else
                res += crd;
        }

        res += piece::get_code(piece, WHITE);
        if (target != Type::NO_TYPE) res += "x";
        res += to_coordinates(move.target());
    } else {
        if (target != Type::NO_TYPE) res += std::format("{}x", to_coordinates(move.source())[0]);
        res += to_coordinates(move.target());
        if (move.is_promote()) res += piece::get_code(move.promoted(), WHITE);
        if (move.is_enpassant()) res += " e.p.";
    }

    if (!MoveList(copy, false, true).size()) res += "#";
    else if (copy.is_check())
        res += "+";

    return res;
}

std::ostream &operator<<(std::ostream &os, const Game &game) {
    static const std::string name[] = {"death", "time forfeit", "rules infraction", "repetition"};
    os << std::format("[Event \"Match {}\"]", game.match_id);
    os << std::format("\n[Site \"{}\"]", "Stellar Arena");
    os << std::format("\n[Date \"{}\"]", timer::get_ms());
    os << std::format("\n[Round \"{}\"]", game.id);
    os << std::format("\n[White \"{}\"]", game.get_white());
    os << std::format("\n[Black \"{}\"]", game.get_black());
    os << std::format("\n[Result \"{}-{}\"]", (int)game.is_win_white(), (int)game.is_win_black());
    os << std::format("\n[Termination \"{}\"]", name[game.get_terminate()]);
    if (game.fen != Game::startPosition) {
        os << std::format("\n[SetUp \"1\"]");
        os << std::format("\n[FEN \"{}\"]", game.fen);
    }
    os << '\n';

    if (!game.list.size()) return os;
    Board board(game.fen);

    const Color side = board.get_side();
    if (side == BLACK) os << std::format("1. ... ");
    for (int i = 0; i < game.list.size(); i++) {
        if (i % 2 == side) os << std::format("{}. ", i / 2 + 1);
        os << std::format("{} ", Game::san ? Game::to_san(board, game.list[i]) : (std::string)game.list[i]);
        game.list[i].make(board);
    }

    return os;
}
