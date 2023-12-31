#include <ctype.h>
#include <exception>
#include <stdio.h>
#include <string.h>

#include "board.hpp"
#include "piece.hpp"
#include "square.hpp"
#include "utils.hpp"
#include "zobrist.hpp"

/* Getters */

Board::Board(const std::string &fen) {
    int file = 0, rank = 7, i;
    for (i = 0; fen[i] != ' '; i++) {
        if (isalpha(fen[i])) {
            const piece::Piece &piece = piece::get_from_code(fen[i]);
            set_piece(piece.type, piece.color, static_cast<square::Square>(rank * 8 + file));
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

    side = fen[++i] == 'w' ? color::WHITE
           : fen[i] == 'b' ? color::BLACK
                           : throw std::runtime_error("Invalid player char");

    for (i += 2; fen[i] != ' '; i++) {
        if (fen[i] == 'K')
            castle |= to_underlying(Castle::WK);
        else if (fen[i] == 'Q')
            castle |= to_underlying(Castle::WQ);
        else if (fen[i] == 'k')
            castle |= to_underlying(Castle::BK);
        else if (fen[i] == 'q')
            castle |= to_underlying(Castle::BQ);
        else if (fen[i] == '-') {
            i++;
            break;
        } else
            throw std::runtime_error("Invalid castle rights");
    }

    enpassant = fen[++i] != '-' ? square::from_coordinates(fen.substr(i, 2)) : square::no_sq;

    hash = Zobrist::hash(*this);
}

std::ostream &operator<<(std::ostream &os, const Board &board) {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            if (!file) os << 8 - rank << " ";
            square::Square square = static_cast<square::Square>((7 - rank) * 8 + file);
            const piece::Piece *piece = board.get_square_piece(square);
            os << (piece ? piece->code : '.') << " ";
        }
        printf("\n");
    }
    os << "  A B C D E F G H\n";
    os << "     Side: ";
    os << ((board.side == color::WHITE) ? "white" : "black") << "\n";
    os << "Enpassant: " << square::to_coordinates(board.enpassant) << "\n";
    os << "   Castle:";
    os << ((board.castle & to_underlying(Board::Castle::WK)) ? 'K' : '-');
    os << ((board.castle & to_underlying(Board::Castle::WQ)) ? 'Q' : '-');
    os << ((board.castle & to_underlying(Board::Castle::BK)) ? 'k' : '-');
    os << ((board.castle & to_underlying(Board::Castle::BQ)) ? 'q' : '-');
    os << "\n     Hash:" << board.hash << "\n\n";

    return os;
}
