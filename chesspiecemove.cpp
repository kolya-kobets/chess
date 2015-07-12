#include "chesspiecemove.h"

#include <sstream>
#include <unordered_map>

using std::pair;
using std::make_pair;
using std::string;
using std::ostringstream;
using std::istringstream;

using std::unordered_map;


/*
 *  BoardMgr implementation
 */

BoardMgr::BoardMgr(ChessPiece (&chess_board)[8][8], bool (&fst_move_flags)[8][8]):
    m_chess_board(chess_board), m_1st_move_flags(fst_move_flags)
{}

bool BoardMgr::is_1st_move(const vec2& pos) const
{
    return m_1st_move_flags[pos[0]][pos[1]];
}

void BoardMgr::change_piece_pos(const vec2& from, const vec2& to)
{
    m_chess_board[to[0]][to[1]] = m_chess_board[from[0]][from[1]];
    m_chess_board[from[0]][from[1]] = ChessPiece::NONE;
}

void BoardMgr::move(const vec2& from, const vec2& to)
{
    m_chess_board[to[0]][to[1]] = m_chess_board[from[0]][from[1]];
    m_chess_board[from[0]][from[1]] = ChessPiece::NONE;

    m_1st_move_flags[from[0]][from[1]] = m_1st_move_flags[to[0]][to[1]] = false;
}

void BoardMgr::promote(const vec2& pos, ChessPiece cp)
{
    m_chess_board[pos[0]][pos[1]] = cp;
}

void BoardMgr::undo_move(const vec2& from, ChessPiece src_piece, bool src_1st_move,
                         const vec2& to, ChessPiece dst_piece, bool dst_1st_move)
{
    m_chess_board[from[0]][from[1]] = src_piece;
    m_chess_board[to[0]][to[1]] = dst_piece;

    m_1st_move_flags[from[0]][from[1]] = src_1st_move;
    m_1st_move_flags[to[0]][to[1]] = dst_1st_move;
}

/*
 *  SimpleMove implementation
 */

SimpleMove::SimpleMove(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp):
    m_chess_pieces(make_pair(src_cp,dst_cp))
{
    m_changed_cells[0] = make_pair(src, src_cp);
    m_changed_cells[1] = make_pair(dst, dst_cp);
}

bool SimpleMove::apply(BoardMgr& board)
{
    if( m_is_applied ) {
        return true;
    }

    const vec2& from = m_changed_cells[0].first;
    const vec2& to = m_changed_cells[1].first;

    m_changed_cells[0].second = ChessPiece::NONE;
    m_changed_cells[1].second = m_chess_pieces.first;

    m_is_1st_move[0] = board.is_1st_move(from);
    m_is_1st_move[1] = board.is_1st_move(to);

    board.move(from, to);

    m_is_applied = true;

    if( board.is_king_under_attack() ) {
        undo(board);
    }

    return m_is_applied;
}

bool SimpleMove::undo(BoardMgr& board)
{
    if( !m_is_applied ) {
        return false;
    }

    const vec2& from = m_changed_cells[0].first;
    const vec2& to = m_changed_cells[1].first;

    m_changed_cells[0].second = m_chess_pieces.first;
    m_changed_cells[1].second = m_chess_pieces.second;

    board.undo_move(from, m_chess_pieces.first, m_is_1st_move[0],
                    to, m_chess_pieces.second, m_is_1st_move[1]);

    m_is_applied = false;

    return true;
}

/*
 * PawnMoveWithPromotion implementation
 */

PawnMoveWithPromotion::PawnMoveWithPromotion(const vec2& src, ChessPiece src_cp,
                                             const vec2& dst,  ChessPiece dst_cp, ChessPiece promote_to):
    SimpleMove(src, src_cp, dst, dst_cp), m_promote_to(promote_to)
{}

bool PawnMoveWithPromotion::apply(BoardMgr& board)
{
    if( SimpleMove::apply(board) ) {
        const vec2& m_to = m_changed_cells[1].first;
        m_changed_cells[1].second = m_promote_to;
        board.promote(m_to, m_promote_to);
    }
    return m_is_applied;
}

/*
 * PawnMoveWithPromotion implementation
 */
Castling::Castling(const vec2& king_pos, ChessPiece king, const vec2& castle_pos, ChessPiece castle):
    m_chess_pieces(make_pair(king,castle))
{
    m_changed_cells[0] = make_pair(king_pos, king);
    m_changed_cells[1] = make_pair(castle_pos, castle);
}

bool Castling::apply(BoardMgr& board)
{
    constexpr int A = 0;
    constexpr int H = 7;


    if( m_is_applied ) {
        return m_is_applied;
    }

    const vec2& src = m_changed_cells[0].first;
    const vec2& dst = m_changed_cells[1].first;

    bool ft_move1 = board.is_1st_move(src);
    bool ft_move2 = board.is_1st_move(dst);

    if( !ft_move1 || !ft_move2 ) {
        return false;
    }

    ChessPiece king = m_chess_pieces.first;
    ChessPiece castle = m_chess_pieces.second;

    //check whether it's castling
    const int side = (king == ChessPiece::WT_KING) ? 0 : 1;
    const int row = (side == 0) ? 0 : 7;
    if( src[0] != row || dst[0] != row || (dst[1] != A && dst[1] != H) ||
       !((castle == ChessPiece::WT_CASTLE && !side) || (castle == ChessPiece::BK_CASTLE && side)) )
    {
        return false;
    }

    //check that there is no piece between king & castle
    int inc = (dst[1] == H) ? 1 : -1;
    for(int i=src[1]+inc; i != dst[1]; i+=inc) {
        if(board.get(row,i) != ChessPiece::NONE)
        {
            return false;
        }
    }

    //check whether king is under attack every its shift
    int king_cln = src[1];
    if( board.is_king_under_attack() ) {
        return false;
    }
    for(int i=0; i<2; i++)
    {
        king_cln+=inc;
        board.change_piece_pos(make_vec2(row,king_cln-inc), make_vec2(row,king_cln));
        if(board.is_king_under_attack())
        {
            board.change_piece_pos(make_vec2(row,king_cln), make_vec2(row,src[1]));
            return false;
        }
    }

    board.change_piece_pos(make_vec2(row,king_cln), make_vec2(row,src[1]));
    board.move(make_vec2(row,src[1]), make_vec2(row,king_cln));

    auto king_pos = make_vec2(row, king_cln);
    auto castle_pos = make_vec2(row, king_cln-inc);
    board.move(make_vec2(row,dst[1]), make_vec2(row,castle_pos[1]));

    m_changed_cells[0].second = ChessPiece::NONE;
    m_changed_cells[1].second = ChessPiece::NONE;
    m_changed_cells[2] = make_pair(king_pos, king);
    m_changed_cells[3] = make_pair(castle_pos, castle);
    m_is_applied = true;

    return true;

}

bool Castling::undo(BoardMgr &board)
{
    const vec2& k1 = m_changed_cells[0].first;
    const vec2& k2 = m_changed_cells[2].first;
    const vec2& c1 = m_changed_cells[1].first;
    const vec2& c2 = m_changed_cells[3].first;

    if( !m_is_applied ) {
        return false;
    }

    board.undo_move(k1, m_chess_pieces.first, true, k2, ChessPiece::NONE, false);
    board.undo_move(c1, m_chess_pieces.second, true, c2, ChessPiece::NONE, false);

    m_changed_cells[0].second = m_chess_pieces.first;
    m_changed_cells[1].second = m_chess_pieces.second;
    m_changed_cells[2].second = m_changed_cells[3].second = ChessPiece::NONE;
    m_is_applied = false;

    return true;
}
