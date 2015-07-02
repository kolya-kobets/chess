#ifndef CHESSPIECEMOVE_H
#define CHESSPIECEMOVE_H

#include "chessboard.h"
#include <string>
#include <utility>
#include <memory>
#include <istream>

template<class T1> inline void UNUSED(T1) {}
template<class T1, class T2> inline void UNUSED(T1,T2) {}

/*
 *   ChessPieceMove - base class for
 */

class ChessPieceMove
{
public:
    typedef ChessBoard::ChessPiece ChessPiece;
    typedef std::pair<vec2, ChessPiece> BoardCell;
    static const int ROWS = ChessBoard::ROWS;
    static const int COLS = ChessBoard::COLS;

    ChessPieceMove()                                                      {  m_is_applied = false;   }

    //@param board is needed to check whether king is under attack
    virtual bool apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS],
                       const ChessBoard& board)                           {  UNUSED(field,board); UNUSED(fst_move); return false;  }
    //@param board is needed to check whether king is under attack
    virtual bool undo(ChessPiece (&field)[ROWS][COLS],
                      bool (&fst_move)[ROWS][COLS])                       {  UNUSED(field,fst_move); return false;  }

    //@ret returns changed cell positions with current pieces
    virtual BoardCell* get_changed_cells()                                {   return NULL;    }
    virtual int changed_cells_count()                                     {   return 0;   }

    virtual std::string to_string()                                       {   return std::string("");  }
    static std::shared_ptr<ChessPieceMove> from_string(const std::string& str);

    virtual ~ChessPieceMove(){}
protected:
    bool m_is_applied;
    bool m_is_1st_move[2];
};

/*
 *   SimpleMove
 */

class SimpleMove:
        public ChessPieceMove
{
public:
    SimpleMove(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp);
    virtual bool apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS], const ChessBoard& board);
    virtual bool undo(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS]);

    virtual BoardCell* get_changed_cells() {   return m_changed_cells;    }
    virtual int changed_cells_count()      {   return 2;   }

    virtual std::string to_string();

    static std::shared_ptr<ChessPieceMove> from_string(const std::string& str);
protected:

    //contains (src, dst) chess piece pair
    const std::pair<ChessPiece,ChessPiece> m_chess_pieces;
    BoardCell m_changed_cells[2];
};

/*
 *   PawnMoveWithPromotion
 */

class PawnMoveWithPromotion:
        public SimpleMove
{
public:
    PawnMoveWithPromotion(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp, ChessPiece promote_to);
    virtual bool apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS], const ChessBoard& board);
    virtual std::string to_string();

    static std::shared_ptr<ChessPieceMove> from_string(const std::string& str);
private:
    const ChessPiece m_promote_to;
};

/*
 *   Castling
 */

class Castling:
        public ChessPieceMove
{
public:
    Castling(const vec2& king_pos, ChessPiece king, const vec2& castle_pos, ChessPiece castle);

    virtual bool apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS], const ChessBoard& board);
    virtual bool undo(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS]);

    virtual BoardCell* get_changed_cells() {   return m_changed_cells;    }
    virtual int changed_cells_count()      {   return 4;   }

    virtual std::string to_string();
    static std::shared_ptr<ChessPieceMove> from_string(const std::string& str);
public:
    //contains (king, castle) chess piece pair
    const std::pair<ChessPiece,ChessPiece> m_chess_pieces;
    BoardCell m_changed_cells[4];
};


#endif // CHESSPIECEMOVE_H
