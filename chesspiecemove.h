#ifndef CHESSPIECEMOVE_H
#define CHESSPIECEMOVE_H


#include <string>
#include <utility>
#include <memory>
#include <istream>

template<class T1> inline void UNUSED(T1) {}
template<class T1, class T2> inline void UNUSED(T1,T2) {}

struct vec2
{
    vec2(int v1=0, int v2=0)      {    v[0]=v1; v[1]=v2;   }
    int& operator[](int ind)      {   return v[ind];   }
    int operator[](int ind) const {   return v[ind];   }
private:
    int v[2];
};

inline vec2 make_vec2(int v1, int v2)       {   return vec2(v1,v2);  }
inline vec2 make_vec2(std::pair<int,int> v) {   return vec2(v.first,v.second);  }


/*
 *  ChessPiece
 */

enum class ChessPiece
{
    NONE=0,
    WT_KING,
    WT_QUEEN,
    WT_BISHOP,
    WT_KNIGHT,
    WT_CASTLE,
    WT_PAWN,

    BK_KING,
    BK_QUEEN,
    BK_BISHOP,
    BK_KNIGHT,
    BK_CASTLE,
    BK_PAWN,
    PIECES_COUNT
};

inline int to_int(ChessPiece cp)                {   return static_cast<int>(cp);    }

/*
 *   BoardMgr
 */

class BoardMgr
{
public:
    static const int ROWS = 8;
    static const int COLS = 8;

    BoardMgr(ChessPiece (&chess_board)[ROWS][COLS], bool (&fst_move_flags)[ROWS][COLS]);

    bool is_1st_move(const vec2& pos) const;

    void change_piece_pos(const vec2& from, const vec2& to);

    void move(const vec2& from, const vec2& to);
    void promote(const vec2& pos, ChessPiece cp);

    void undo_move(const vec2& from, ChessPiece src_piece, bool src_1st_move,
                   const vec2& to, ChessPiece dst_piece, bool dst_1st_move);

    ChessPiece get(int x, int y) const                            {    return m_chess_board[x][y];   }
    bool is_king_under_attack() const                             {    return false;   }
private:
    ChessPiece (&m_chess_board)[8][8];
    bool (&m_1st_move_flags)[8][8];
};


/*
 *   ChessMove - base class for
 */

class ChessMove
{
public:
    typedef std::pair<vec2, ChessPiece> BoardCell;
    static const int ROWS = BoardMgr::ROWS;
    static const int COLS = BoardMgr::COLS;

    ChessMove()                                                           {  m_is_applied = false;   }

    //@param board is needed to check whether king is under attack
    virtual bool apply(BoardMgr& board)                             {  UNUSED(board); return false;  }
    //@param board is needed to check whether king is under attack
    virtual bool undo(BoardMgr& board)                              {  UNUSED(board); return false;  }

    //@ret returns changed cell positions with current pieces
    virtual BoardCell* get_changed_cells()                                {   return NULL;    }
    virtual int changed_cells_count()                                     {   return 0;   }

    virtual const vec2& get_src_pos() const = 0;
    virtual const vec2& get_dst_pos() const = 0;

    virtual ~ChessMove(){}
protected:
    bool m_is_applied;
    bool m_is_1st_move[2];
};

/*
 *   SimpleMove
 */

class SimpleMove:
        public ChessMove
{
public:
    SimpleMove(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp);
    virtual bool apply(BoardMgr& board);
    virtual bool undo(BoardMgr& board);

    virtual BoardCell* get_changed_cells() {   return m_changed_cells;    }
    virtual int changed_cells_count()      {   return 2;   }

    virtual const vec2& get_src_pos() const       {   return m_changed_cells[0].first;   }
    virtual const vec2& get_dst_pos() const       {   return m_changed_cells[1].first;   }
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
    virtual bool apply(BoardMgr& board);
private:
    const ChessPiece m_promote_to;
};

/*
 *   Castling
 */

class Castling:
        public ChessMove
{
public:
    Castling(const vec2& king_pos, ChessPiece king, const vec2& castle_pos, ChessPiece castle);

    virtual bool apply(BoardMgr& board);
    virtual bool undo(BoardMgr& board);

    virtual BoardCell* get_changed_cells()        {   return m_changed_cells;    }
    virtual int changed_cells_count()             {   return 4;   }

    virtual const vec2& get_src_pos() const       {   return m_changed_cells[0].first;   }
    virtual const vec2& get_dst_pos() const       {   return m_changed_cells[1].first;   }
public:
    //contains (king, castle) chess piece pair
    const std::pair<ChessPiece,ChessPiece> m_chess_pieces;
    BoardCell m_changed_cells[4];
};


#endif // CHESSPIECEMOVE_H
