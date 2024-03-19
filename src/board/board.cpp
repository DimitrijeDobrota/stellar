#include <cctype>
#include <cstdio>
#include <cstring>
#include <exception>

#include "board.hpp"
#include "piece.hpp"
#include "utils.hpp"
#include "utils_ui.hpp"
#include "zobrist.hpp"

namespace zobrist {

/* Init arrays for Zobris hashing */
U32 keys_pawn[2][64] = {0};
U64 keys_piece[2][12][64] = {0};
U64 keys_enpassant[64] = {0};
U64 keys_castle[16] = {0};

} // namespace zobrist

/* Getters */

Board::Board(const std::string &fen) {
    int file = 0, rank = 7, i = 0;
    for (i = 0; fen[i] != ' '; i++) {
        if (isalpha(fen[i])) {
            const piece::Piece &piece = piece::get_from_code(fen[i]);
            set_piece(piece.type, piece.color, static_cast<Square>(rank * 8 + file));
            file++;
        } else if (isdigit(fen[i])) {
            file += fen[i] - '0';
        } else if (fen[i] == '/') {
            if (file != 8) throw std::runtime_error("File is not complete");
            file = 0;
            rank--;
        } else {
            throw std::runtime_error("Invalid piece position");
        }
    }

    side = fen[++i] == 'w' ? WHITE : fen[i] == 'b' ? BLACK : throw std::runtime_error("Invalid player char");

    for (i += 2; fen[i] != ' '; i++) {
        if (fen[i] == 'K') castle |= Castle::WK;
        else if (fen[i] == 'Q')
            castle |= Castle::WQ;
        else if (fen[i] == 'k')
            castle |= Castle::BK;
        else if (fen[i] == 'q')
            castle |= Castle::BQ;
        else if (fen[i] == '-') {
            i++;
            break;
        } else
            throw std::runtime_error("Invalid castle rights");
    }

    enpassant = fen[++i] != '-' ? from_coordinates(fen.substr(i, 2)) : Square::no_sq;

    hash_pawn = zobrist::hash_pawn(*this);
    hash = zobrist::hash(*this);
}

std::ostream &operator<<(std::ostream &os, const Board &board) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            if (!file) os << 8 - rank << " ";
            auto square = static_cast<Square>((7 - rank) * 8 + file);
            const Type t = board.get_square_piece_type(square);
            if (t == NO_TYPE) {
                os << ". ";
            } else {
                const Color c = board.get_square_piece_color(square);
                os << piece::get_code(t, c) << " ";
            }
        }
        printf("\n");
    }
    os << "  A B C D E F G H\n";
    os << "     Side: ";
    os << ((board.side == WHITE) ? "white" : "black") << "\n";
    os << "Enpassant: " << to_coordinates(board.enpassant) << "\n";
    os << "   Castle:";
    os << ((board.castle & Board::Castle::WK) ? 'K' : '-');
    os << ((board.castle & Board::Castle::WQ) ? 'Q' : '-');
    os << ((board.castle & Board::Castle::BK) ? 'k' : '-');
    os << ((board.castle & Board::Castle::BQ) ? 'q' : '-');
    os << "\n     Hash:" << board.hash << "\n\n";

    return os;
}
