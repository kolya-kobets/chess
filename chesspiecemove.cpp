#include "chesspiecemove.h"

using std::pair;
using std::make_pair;

/*
 * SimpleMove implementation
 */

SimpleMove::SimpleMove(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp):
    m_chess_pieces(make_pair(src_cp,dst_cp))
{
    m_changed_cells[0] = make_pair(src, src_cp);
    m_changed_cells[1] = make_pair(dst, dst_cp);
}

bool SimpleMove::apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board, bool* ft_move1, bool* ft_move2)
{
    if( m_is_applied ) {
        return true;
    }

    const vec2& m_from = m_changed_cells[0].first;
    const vec2& m_to = m_changed_cells[1].first;
    field[m_from[0]][m_from[1]] = m_changed_cells[0].second = ChessBoard::NONE;
    field[m_to[0]][m_to[1]] = m_changed_cells[1].second = m_chess_pieces.first;

    m_is_1st_move[0] = ft_move1 == NULL ? false : *ft_move1;
    m_is_1st_move[1] = ft_move2 == NULL ? false : *ft_move2;
    if( m_is_1st_move[0] ) *ft_move1 = false;
    if( m_is_1st_move[1] ) *ft_move2 = false;

    m_is_applied = true;

    if( board.is_king_under_attack() ) {
        undo(field, board);
    }

    return m_is_applied;
}

bool SimpleMove::undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board, bool* ft_move1, bool* ft_move2)
{
    UNUSED(board);

    if( !m_is_applied ) {
        return false;
    }

    const vec2& m_from = m_changed_cells[0].first;
    const vec2& m_to = m_changed_cells[1].first;
    field[m_from[0]][m_from[1]] = m_changed_cells[0].second = m_chess_pieces.first;
    field[m_to[0]][m_to[1]] = m_changed_cells[1].second = m_chess_pieces.second;

    m_is_applied = false;

    if( ft_move1 ) *ft_move1 = m_is_1st_move[0];
    if( ft_move2 ) *ft_move2 = m_is_1st_move[1];

    return true;
}

/*
 * PawnMoveWithPromotion implementation
 */

PawnMoveWithPromotion::PawnMoveWithPromotion(const vec2& src, ChessPiece src_cp,
                                             const vec2& dst,  ChessPiece dst_cp, ChessPiece promote_to):
    SimpleMove(src, src_cp, dst, dst_cp), m_promote_to(promote_to)
{}

bool PawnMoveWithPromotion::apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board, bool* ft_move1, bool* ft_move2)
{
    if( SimpleMove::apply(field, board, ft_move1, ft_move2) ) {
        const vec2& m_to = m_changed_cells[1].first;
        field[m_to[0]][m_to[1]] = m_changed_cells[1].second = m_promote_to;
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
    //m_changed_cells[2] = make_pair(king_pos, king);
    //m_changed_cells[3] = make_pair(king_pos, king);
}

bool Castling::apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board, bool* ft_move1, bool* ft_move2)
{
    static const int A = ChessBoard::A;
    static const int H = ChessBoard::H;

    static const int WT_CASTLE = ChessBoard::WT_CASTLE;
    static const int BK_CASTLE = ChessBoard::BK_CASTLE;

    if( m_is_applied ) {
        return m_is_applied;
    }

    if( !ft_move1 || !ft_move2 || !*ft_move1 || !*ft_move2 ) {
        return false;
    }

    ChessPiece king = m_chess_pieces.first;
    ChessPiece castle = m_chess_pieces.second;
    const vec2& src = m_changed_cells[0].first;
    const vec2& dst = m_changed_cells[1].first;

    //check whether it's castling
    const int side = (king == ChessBoard::WT_KING) ? 0 : 1;
    const int row = (side == 0) ? 0 : 7;
    if( src[0] != row || dst[0] != row || (dst[1] != A && dst[1] != H) ||
       !((castle == WT_CASTLE && !side) || (castle == BK_CASTLE && side)) )
    {
        return false;
    }

    //check that there is no piece between king & castle
    int inc = (dst[1] == H) ? 1 : -1;
    for(int i=src[1]+inc; i != dst[1]; i+=inc) {
        if(field[row][i] != ChessBoard::NONE)
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
        std::swap(field[row][king_cln-inc], field[row][king_cln]);
        if(board.is_king_under_attack())
        {
            std::swap(field[row][src[1]], field[row][king_cln]);
            return false;
        }
    }

    auto king_pos = make_vec2(row, king_cln);
    auto castle_pos = make_vec2(row, king_cln-inc);
    std::swap(field[row][dst[1]], field[row][castle_pos[1]]);


    m_changed_cells[0].second = ChessBoard::NONE;
    m_changed_cells[1].second = ChessBoard::NONE;
    m_changed_cells[2] = make_pair(king_pos, king);
    m_changed_cells[3] = make_pair(castle_pos, castle);

    *ft_move1 = *ft_move2 = false;
    m_is_applied = true;

    return true;

}

bool Castling::undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board, bool* ft_move1, bool* ft_move2)
{
    UNUSED(board);

    if( !m_is_applied || !ft_move1 || !ft_move2 ) {
        return false;
    }
    *ft_move1 = *ft_move2 = true;

    const vec2& k1 = m_changed_cells[0].first;
    const vec2& k2 = m_changed_cells[1].first;
    const vec2& c1 = m_changed_cells[3].first;
    const vec2& c2 = m_changed_cells[4].first;

    field[k2[0]][k2[1]] = m_changed_cells[1].second = ChessBoard::NONE;
    field[c2[0]][c2[1]] = m_changed_cells[3].second = ChessBoard::NONE;

    field[k1[0]][k1[1]] =  m_changed_cells[0].second = m_chess_pieces.first;
    field[c1[0]][c1[1]] =  m_changed_cells[2].second = m_chess_pieces.second;

    m_is_applied = false;

    return true;
}

/*

    //contains (king, castle) chess piece pair
    //const std::pair<ChessPiece,ChessPiece> m_chess_pieces;
    BoardCell m_changed_cells[2];

 *
 * */
