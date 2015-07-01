#include "chesspiecemove.h"

using std::pair;
using std::make_pair;

/*
 * SimpleMove implementation
 */

SimpleMove::SimpleMove(const vec2& src, ChessPiece src_cp, const vec2& dst,  ChessPiece dst_cp, bool* first_move_indicator):
    m_chess_pieces(make_pair(src_cp,dst_cp))
{
    m_changed_cells[0] = make_pair(src, src_cp);
    m_changed_cells[1] = make_pair(dst, dst_cp);

    if(first_move_indicator == NULL || *first_move_indicator) {
        m_check_when_applied = NULL;
    } else {
        m_check_when_applied = first_move_indicator;
    }
}

bool SimpleMove::apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board)
{
    if( m_is_applied ) {
        return true;
    }

    const vec2& m_from = m_changed_cells[0].first;
    const vec2& m_to = m_changed_cells[1].first;
    field[m_from[0]][m_from[1]] = m_changed_cells[0].second = ChessBoard::NONE;
    field[m_to[0]][m_to[1]] = m_changed_cells[1].second = m_chess_pieces.first;

    m_is_applied = true;

    if( board.is_king_under_attack() ) {
        undo(field, board);
    }

    if( m_check_when_applied != NULL ) {
        *m_check_when_applied = m_is_applied;
    }
    return m_is_applied;
}

bool SimpleMove::undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board)
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

    if( m_check_when_applied != NULL ) {
        *m_check_when_applied = false;
    }
    return true;
}

/*
 * PawnMoveWithPromotion implementation
 */

PawnMoveWithPromotion::PawnMoveWithPromotion(const vec2& src, ChessPiece src_cp,
                                             const vec2& dst,  ChessPiece dst_cp, ChessPiece promote_to):
    SimpleMove(src, src_cp, dst, dst_cp), m_promote_to(promote_to)
{}

bool PawnMoveWithPromotion::apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board)
{
    if( SimpleMove::apply(field, board) ) {
        const vec2& m_to = m_changed_cells[1].first;
        field[m_to[0]][m_to[1]] = m_changed_cells[1].second = m_promote_to;
    }
    return m_is_applied;
}

/*
 * PawnMoveWithPromotion implementation
 */
Castling::Castling(const vec2& king_pos, ChessPiece king, const vec2& castle_pos, ChessPiece castle,
                   bool& king_first_move, bool (&castles_first_move)[2]):
    m_chess_pieces(make_pair(king,castle)), m_king_first_move(king_first_move),
    m_castles_first_move(castles_first_move)
{}

bool Castling::apply(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board)
{
}

bool Castling::undo(ChessPiece (&field)[ROWS][COLS], const ChessBoard& board)
{
    UNUSED(board);

    if( !m_is_applied ) {
        return false;
    }

    const vec2& k1 = m_changed_cells[0].first;
    const vec2& k2 = m_changed_cells[1].first;
    const vec2& c1 = m_changed_cells[3].first;
    const vec2& c2 = m_changed_cells[4].first;

    field[k2[0]][k2[1]] = field[c2[0]][c2[1]] = ChessBoard::NONE;
    m_changed_cells[1].first = m_changed_cells[3].first = ChessBoard::NONE;

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
