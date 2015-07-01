#ifndef CHESSPIECEMOVE_H
#define CHESSPIECEMOVE_H

#include "chessboard.h"
#include <string>
#include <utility>
#include <memory>

template<class T1> inline void UNUSED(T1) {}
template<class T1, class T2> inline void UNUSED(T1,T2) {}


class ChessPieceMove
{
public:
    typedef ChessBoard::ChessPiece ChessPiece;
    typedef std::pair<vec2, ChessPiece> BoardCell;
    static const int ROWS = ChessBoard::ROWS;
    static const int COLS = ChessBoard::COLS;


    ChessPieceMove()                                                      {  m_is_applied = false;   }

    //@param board is needed to check whether king is under attack
    virtual bool apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board) {  UNUSED(field,board); return false;  }
    //@param board is needed to check whether king is under attack
    virtual bool undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board)  {  UNUSED(field,board); return false;  }

    //@ret returns changed cell positions with current pieces
    virtual BoardCell* get_changed_cells()                                {   return NULL;    }
    virtual int changed_cells_count()                                     {   return 0;   }

    virtual std::string to_string()                                       {   return std::string("");  }
    std::shared_ptr<ChessPieceMove> from_string(const std::string& str)   {  UNUSED(str);    }

    virtual ~ChessPieceMove(){}
protected:
    bool m_is_applied;
};


class SimpleMove:
        public ChessPieceMove
{
public:
    SimpleMove(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp, bool* first_move_indicator = NULL);
    virtual bool apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board);
    virtual bool undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board);

    virtual BoardCell* get_changed_cells() {   return m_changed_cells;    }
    virtual int changed_cells_count()      {   return 2;   }
protected:
    bool* m_check_when_applied;
    //contains (src, dst) chess piece pair
    const std::pair<ChessPiece,ChessPiece> m_chess_pieces;
    BoardCell m_changed_cells[2];
};

class PawnMoveWithPromotion:
        public SimpleMove
{
public:
    PawnMoveWithPromotion(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp, ChessPiece promote_to);
    virtual bool apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board);
private:
    const ChessPiece m_promote_to;
};

class Castling:
        public ChessPieceMove
{
public:
    Castling(const vec2& king_pos, ChessPiece king, const vec2& castle_pos, ChessPiece castle,
             bool& king_first_move, bool (&castles_first_move)[2]);

    virtual bool apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board);
    virtual bool undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board);

    virtual BoardCell* get_changed_cells() {   return m_changed_cells;    }
    virtual int changed_cells_count()      {   return m_is_applied ? 4 : 0;   }

public:
    bool& m_king_first_move;
    bool (&m_castles_first_move)[2];
    //contains (king, castle) chess piece pair
    const std::pair<ChessPiece,ChessPiece> m_chess_pieces;
    BoardCell m_changed_cells[4];
};


#endif // CHESSPIECEMOVE_H
