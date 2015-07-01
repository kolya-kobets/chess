#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <utility>
#include <memory>

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


class ChessPieceMove;

class ChessBoard
{
public:
    static const int ROWS = 8;
    static const int COLS = 8;

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
    std::shared_ptr<ChessPieceMove> make_move(const std::pair<int,int>& src, const std::pair<int,int>& dst);
    bool is_king_under_attack() const;
private:
    std::shared_ptr<ChessPieceMove> pawn_move(const vec2& src, const vec2& dst) const;
    std::shared_ptr<ChessPieceMove> castle_move(const vec2& src, const vec2& dst);
    std::shared_ptr<ChessPieceMove> bishop_move(const vec2& src, const vec2& dst);
    std::shared_ptr<ChessPieceMove> knight_move(const vec2& src, const vec2& dst) const;
    std::shared_ptr<ChessPieceMove> queen_move(const vec2& src, const vec2& dst);
    //src2 & dst2 is used if castling
    std::shared_ptr<ChessPieceMove> move_king(const vec2& src, const vec2& dst, vec2& src2, vec2& dst2);

    std::shared_ptr<ChessPieceMove> simple_move(const vec2& src, const vec2& dst, bool *first_move = NULL) const;
    bool* get_castle_first_move_flag(const vec2& src);

    ChessPiece m_chess_board[8][8];

    static const int BLACK = 0;
    static const int WHITE = 1;
    bool m_is_king_moved[2];
    bool m_is_castle_moved[2][2]; //[0-black,1-white][0-a,1-h]
    int m_current_side;
};
#endif // CHESSBOARD_H
