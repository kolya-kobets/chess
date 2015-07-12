#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <utility>
#include <memory>
#include <list>
#include <iostream>

#include "chesspiecemove.h"

class ChessBoard
{
public:
    static const int ROWS = 8;
    static const int COLS = 8;

    enum Column{
        A=0,B,C,D,E,F,G,H
    };

public:
    ChessBoard();
    void reset_board();
    void clean_board();

    ChessPiece get_board_piece(int r, int c)
    {
        return m_chess_board[r][c];
    }

    //first 3 parameters - for common case,
    // 4,5 - in case of castling
    std::shared_ptr<ChessMove> make_move(const vec2& src, const vec2& dst);

    std::shared_ptr<ChessMove> undo();
    std::shared_ptr<ChessMove> redo();

    bool save_game(std::ostream& stream);
    bool load_game(std::istream& stream);

    bool is_king_under_attack() const;
private:
    std::shared_ptr<ChessMove> pawn_move(const vec2& src, const vec2& dst) const;
    std::shared_ptr<ChessMove> castle_move(const vec2& src, const vec2& dst);
    std::shared_ptr<ChessMove> bishop_move(const vec2& src, const vec2& dst);
    std::shared_ptr<ChessMove> knight_move(const vec2& src, const vec2& dst) const;
    std::shared_ptr<ChessMove> queen_move(const vec2& src, const vec2& dst);
    //src2 & dst2 is used if castling
    std::shared_ptr<ChessMove> king_move(const vec2& src, const vec2& dst);

    std::shared_ptr<ChessMove> simple_move(const vec2& src, const vec2& dst) const;

    ChessPiece m_chess_board[8][8];
    bool m_1st_move_flags[8][8];

    static const int BLACK = 0;
    static const int WHITE = 1;

    int m_current_side;

    typedef std::list<std::shared_ptr<ChessMove> > PieceMoves;
    PieceMoves m_moves;
    PieceMoves::iterator m_current_move;

    BoardMgr m_board_mgr;
};

#endif // CHESSBOARD_H
