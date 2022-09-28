#ifndef CBOARD_H
#define CBOARD_H

#include "utils.h"

enum enumCastle { WK = 1, WQ = 2, BK = 4, BQ = 8 };
typedef enum enumCastle eCastle;

typedef struct Piece_T *Piece_T;

char   Piece_asci(Piece_T pt);
char   Piece_code(Piece_T pt);
char  *Piece_unicode(Piece_T pt);
eColor Piece_color(Piece_T pt);
ePiece Piece_piece(Piece_T pt);
int    Piece_index(Piece_T self);

Piece_T Piece_fromCode(char code);
Piece_T Piece_fromIndex(int index);
Piece_T Piece_get(ePiece piece, eColor color);

typedef struct CBoard_T *CBoard_T;

CBoard_T CBoard_new(void);
void     CBoard_copy(CBoard_T self, CBoard_T dest);

U64     CBoard_colorBB(CBoard_T self, eColor color);
U64     CBoard_occupancy(CBoard_T self);
U64     CBoard_pieceBB(CBoard_T self, ePiece piece);
eCastle CBoard_castle(CBoard_T self);
eColor  CBoard_side(CBoard_T self);

Square CBoard_enpassant(CBoard_T self);
void   CBoard_enpassant_set(CBoard_T self, Square target);

U64  CBoard_pieceSet(CBoard_T self, Piece_T piece);
U64  CBoard_piece_attacks(CBoard_T self, Piece_T Piece, Square src);
void CBoard_piece_capture(CBoard_T self, Piece_T Piece, Square source, Square target);
void CBoard_piece_move(CBoard_T self, Piece_T Piece, Square square, Square target);
void CBoard_piece_pop(CBoard_T self, Piece_T Piece, Square square);
void CBoard_piece_set(CBoard_T self, Piece_T Piece, Square square);

U64  CBoard_colorBB_get(CBoard_T self, eColor color, Square target);
void CBoard_colorBB_pop(CBoard_T self, eColor color, Square target);
void CBoard_colorBB_set(CBoard_T self, eColor color, Square target);

void CBoard_castle_and(CBoard_T self, int exp);
void CBoard_castle_pop(CBoard_T self, eCastle castle);

Piece_T CBoard_square_piece(CBoard_T self, Square square, eColor side);
int     CBoard_square_isAttack(CBoard_T self, Square square, eColor side);
int     CBoard_square_isOccupied(CBoard_T self, Square square);

CBoard_T CBoard_fromFEN(CBoard_T board, char *fen);
int      CBoard_isCheck(CBoard_T self);
void     CBoard_print(CBoard_T self);
void     CBoard_side_switch(CBoard_T self);

#endif
