#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <utility>


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


class ChessBoard
{
public:
    enum Column{
        A=0,B,C,D,E,F,G,H
    };
    enum ChessPiece {
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
        BK_PAWN
    };

    enum MoveResult {
        WRONG     = 1,
        SIMPLE    = 2,
        CASTLING  = 4,
        REPLACE   = 8,
        PROMOTION = 16 // for now pawn is replaced by queen only
    }; //2^n because of replace & promotion

public:
    ChessBoard(){   reset_board();  }
    void reset_board();
    void clean_board();

    ChessPiece get_board_piece(int r, int c)
    {
        return m_chess_board[r][c];
    }

    //first 3 parameters - for common case,
    // 4,5 - in case of castling
    MoveResult make_move(const std::pair<int,int>& src, const std::pair<int,int>& dst, std::pair<ChessPiece,ChessPiece>& res,
                         std::pair<int,int>& src2, std::pair<int,int>& dst2);
private:
    MoveResult is_pawn_move_allowed(const vec2& src, const vec2& dst) const;
    MoveResult check_move_as_castle(const vec2& src, const vec2& dst) const;
    MoveResult check_move_as_bishop(const vec2& src, const vec2& dst) const;
    MoveResult check_knight_move(const vec2& src, const vec2& dst) const;
    MoveResult check_queen_move(const vec2& src, const vec2& dst) const;
    //src2 & dst2 is used if castling
    MoveResult move_king(const vec2& src, const vec2& dst, vec2& src2, vec2& dst2);

    MoveResult check_replace(const vec2& src, const vec2& dst) const;
    void check_castle_first_move(const vec2& from, ChessPiece cp);
    bool is_king_under_attack() const;

    ChessPiece m_chess_board[8][8];

    static const int BLACK = 0;
    static const int WHITE = 1;
    bool m_is_king_moved[2];
    bool m_is_castle_moved[2][2]; //[0-black,1-white][0-a,1-h]
    int m_current_side;
};
#endif // CHESSBOARD_H
