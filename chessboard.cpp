#include "chessboard.h"
#include "chesspiecemove.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

using std::get;
using std::pair;
using std::make_pair;
using std::abs;
using std::shared_ptr;
using std::make_shared;



/*
 *  Auxiliary functions
 */

bool read_vec2(std::istream& in, vec2& v)
{
    char cln = '\0', row = '\0';
    in >> cln >> row;
    if(cln >= 'a' && cln <= 'h' && row >= '1' && row <= '8' && in.good())
    {
        v = make_vec2(row -'1', cln - 'a');
        return true;
    }

    return false;
}

bool write_vec2(std::ostream& out, const vec2& v)
{
    out << static_cast<char>('a'+v[1]) << v[0]+1;
    return out.good();
}

inline bool is_white(ChessPiece cp)
{
    return cp >= ChessPiece::WT_KING && cp <= ChessPiece::WT_PAWN;
}
inline bool is_black(ChessPiece cp)
{
    return cp >= ChessPiece::BK_KING && cp <= ChessPiece::BK_PAWN;
}
inline bool is_pawn_ready_to_promotion(const vec2& idx, ChessPiece cp)
{
    return (cp == ChessPiece::WT_PAWN && idx[0] == 7) || (cp == ChessPiece::BK_PAWN && idx[0] == 0);
}


/*
 * ChessBoard implementation
 */

ChessBoard::ChessBoard():
    m_board_mgr(m_chess_board, m_1st_move_flags)
{
    clean_board();
}

std::shared_ptr<ChessMove> ChessBoard::make_move(const vec2& src, const vec2& dst)
{
    ChessPiece piece = m_chess_board[src[0]][src[1]];

    if( (m_current_side == WHITE && is_black(piece)) || (m_current_side == BLACK && is_white(piece)) ) {
        return shared_ptr<ChessMove>(NULL);
    }

    auto result = shared_ptr<ChessMove>(NULL);

    switch( piece ) {
        case ChessPiece::WT_PAWN: case ChessPiece::BK_PAWN:
            result = pawn_move(src,dst);
            break;
        case ChessPiece::WT_CASTLE: case ChessPiece::BK_CASTLE:
            result = castle_move(src,dst);
            break;
        case ChessPiece::WT_BISHOP: case ChessPiece::BK_BISHOP:
            result = bishop_move(src,dst);
            break;
        case ChessPiece::WT_KNIGHT: case ChessPiece::BK_KNIGHT:
            result = knight_move(src,dst);
            break;
        case ChessPiece::WT_QUEEN: case ChessPiece::BK_QUEEN:
            result = queen_move(src,dst);
            break;
        case ChessPiece::WT_KING: case ChessPiece::BK_KING:
            result = king_move(src,dst);
            break;
        default:
            //result = WRONG;
            break;
    }

    if( !result || !result->apply(m_board_mgr) ) {
        return shared_ptr<ChessMove>(NULL);
    }

    m_current_side = 1-m_current_side;

    // in case when we have loaded game
    m_moves.erase(m_current_move, m_moves.end());

    m_moves.push_back(result);
    m_current_move = m_moves.end();

    return result;
}

std::shared_ptr<ChessMove> ChessBoard::undo()
{

    if(m_current_move != m_moves.begin()) {
        auto prev = m_current_move; prev--;
        if ((*prev)->undo(m_board_mgr)) {
            m_current_move = prev;
            m_current_side = 1-m_current_side;
            return shared_ptr<ChessMove>(*(m_current_move));
        }
    }
    return shared_ptr<ChessMove>(NULL);
}

std::shared_ptr<ChessMove> ChessBoard::redo()
{
    if(m_current_move != m_moves.end() &&
      (*m_current_move)->apply(m_board_mgr))
    {
        m_current_side = 1-m_current_side;
        return shared_ptr<ChessMove>(*(m_current_move++));
    }
    return shared_ptr<ChessMove>(NULL);
}

std::shared_ptr<ChessMove> ChessBoard::pawn_move(const vec2& src, const vec2& dst) const
{
    ChessPiece piece = m_chess_board[src[0]][src[1]];
    ChessPiece dst_piece = m_chess_board[dst[0]][dst[1]];
    std::shared_ptr<ChessMove> ret = shared_ptr<ChessMove>(NULL);

    bool is_allowed = false;
    if ( piece == ChessPiece::WT_PAWN ) {
        //first move
        if( dst[0] == src[0]+2 ) {
            is_allowed = dst[1] == src[1] && m_chess_board[src[0]+1][src[1]] == ChessPiece::NONE &&
                m_chess_board[dst[0]][src[1]] == ChessPiece::NONE && src[0] == 1;
        } else if( dst[0] == src[0]+1 ) {
            //replace
            is_allowed = (abs(dst[1] - src[1]) == 1 && m_chess_board[dst[0]][dst[1]] != ChessPiece::NONE &&
                         is_black(dst_piece)) ||
            //move only
                         (dst[1] == src[1]  && m_chess_board[dst[0]][dst[1]] == ChessPiece::NONE);
        }
    } else {
        //first move
        if( dst[0] == src[0]-2 ) {
            is_allowed = dst[1] == src[1] && m_chess_board[src[0]-1][src[1]] == ChessPiece::NONE &&
                m_chess_board[dst[0]][src[1]] == ChessPiece::NONE && src[0] == 6;
        } else if( dst[0] == src[0]-1 ) {
            //replace
            is_allowed = (abs(dst[1] - src[1]) == 1 && m_chess_board[dst[0]][dst[1]] != ChessPiece::NONE &&
                         is_white(dst_piece)) ||
            //simple move
                         (dst[1] == src[1] && m_chess_board[dst[0]][dst[1]] == ChessPiece::NONE );
        }
    }

    if( is_allowed ) {
        if( is_pawn_ready_to_promotion(dst,piece) ) {
            ChessPiece promote_to = is_black(piece) ? ChessPiece::BK_QUEEN : ChessPiece::WT_QUEEN;
            return make_shared<PawnMoveWithPromotion>(src,piece, dst, dst_piece, promote_to);
        } else {
            return make_shared<SimpleMove>(src,piece, dst, dst_piece);
        }
    }

    return shared_ptr<ChessMove>(NULL);
}

std::shared_ptr<ChessMove> ChessBoard::castle_move(const vec2& src, const vec2& dst)
{
    if ( std::abs(src[0] - dst[0]) >= 1 && std::abs(src[1] - dst[1]) >= 1 ) {
        return std::shared_ptr<ChessMove>(NULL);
    }
    using std::min;
    using std::max;
    //check whether any piece is located between our positions
    if( std::abs(src[0] - dst[0]) >= 1 ) {
        int beg = min(src[0],dst[0]);
        int end = max(src[0],dst[0]);
        for(int i=beg+1; i<end; i++) {
            if( m_chess_board[i][src[1]] != ChessPiece::NONE ) {
                return shared_ptr<ChessMove>(NULL);
            }
        }
    } else {
        int beg = min(src[1],dst[1]);
        int end = max(src[1],dst[1]);
        for(int i=beg+1; i<end; i++) {
            if( m_chess_board[src[0]][i] != ChessPiece::NONE ) {
                return shared_ptr<ChessMove>(NULL);
            }
        }
    }
    return simple_move(src,dst);
}

std::shared_ptr<ChessMove> ChessBoard::bishop_move(const vec2& from, const vec2& to)
{
    vec2 src = from, dst = to;
    if( abs(src[0]-dst[0]) != abs(src[1]-dst[1]) ) {
        return shared_ptr<ChessMove>(NULL);
    }
    if(src[0] > dst[0]) {
        std::swap(src,dst);
    }
    //check whether any piece is located between our positions
    int inc = (src[1]<dst[1])?1:-1;
    for(int i=src[0]+1, j=src[1]+inc; i<dst[0]; i++, j+=inc){
        if( m_chess_board[i][j] != ChessPiece::NONE ) {
            return shared_ptr<ChessMove>(NULL);
        }
    }

    return simple_move(from,to);
}

std::shared_ptr<ChessMove> ChessBoard::knight_move(const vec2& src, const vec2& dst) const
{
    if( (abs(src[0]-dst[0]) == 2 && abs(src[1]-dst[1]) == 1) ||
        (abs(src[0]-dst[0]) == 1 && abs(src[1]-dst[1]) == 2) ) {
        return simple_move(src,dst);
    }
    return shared_ptr<ChessMove>(NULL);
}

std::shared_ptr<ChessMove> ChessBoard::queen_move(const vec2& src, const vec2& dst)
{
    std::shared_ptr<ChessMove> result = castle_move(src,dst);
    if(!result) {
        result = bishop_move(src,dst);
    }
    return result;
}

std::shared_ptr<ChessMove> ChessBoard::king_move(const vec2& src, const vec2& dst)
{
    if( abs(src[0]-dst[0]) <=1 && abs(src[1]-dst[1]) <=1 ) {
        return simple_move(src,dst);
    }
    ChessPiece king = m_chess_board[src[0]][src[1]];
    ChessPiece castle = m_chess_board[dst[0]][dst[1]];
    if( castle == ChessPiece::WT_CASTLE || castle == ChessPiece::BK_CASTLE ){
        return make_shared<Castling>(src, king, dst, castle);
    } else {
        return shared_ptr<ChessMove>(NULL);
    }
}

std::shared_ptr<ChessMove> ChessBoard::simple_move(const vec2& src, const vec2& dst) const
{
    ChessPiece src_piece = m_chess_board[src[0]][src[1]];
    ChessPiece dst_piece = m_chess_board[dst[0]][dst[1]];
    if( dst_piece != ChessPiece::NONE && ((is_black(src_piece) && is_black(dst_piece)) ||
      ( is_white(src_piece) && is_white(dst_piece))) )
    {
        return shared_ptr<ChessMove>(NULL);
    } else {
        return make_shared<SimpleMove>(src,src_piece,dst,dst_piece);
    }
}

bool ChessBoard::is_king_under_attack() const
{
    // TODO: implement
    return false;
}

void ChessBoard::reset_board()
{
    clean_board();

    m_chess_board[0][A] = m_chess_board[0][H] = ChessPiece::WT_CASTLE;
    m_chess_board[0][B] = m_chess_board[0][G] = ChessPiece::WT_KNIGHT;
    m_chess_board[0][C] = m_chess_board[0][F] = ChessPiece::WT_BISHOP;
    m_chess_board[0][D] = ChessPiece::WT_QUEEN; m_chess_board[0][E] = ChessPiece::WT_KING;

    m_chess_board[7][A] = m_chess_board[7][H] = ChessPiece::BK_CASTLE;
    m_chess_board[7][B] = m_chess_board[7][G] = ChessPiece::BK_KNIGHT;
    m_chess_board[7][C] = m_chess_board[7][F] = ChessPiece::BK_BISHOP;
    m_chess_board[7][D] = ChessPiece::BK_QUEEN; m_chess_board[7][E] = ChessPiece::BK_KING;

    for(int i=0; i<8; i++) {
        m_chess_board[1][i] = ChessPiece::WT_PAWN;
        m_chess_board[6][i] = ChessPiece::BK_PAWN;
        std::fill_n(m_1st_move_flags[i], COLS, true);
    }

    m_current_side = WHITE;
}

void ChessBoard::clean_board()
{
    memset(m_chess_board,0,sizeof(m_chess_board));

    m_moves.clear();
    m_current_move = m_moves.end();
}

bool ChessBoard::save_game(std::ostream& stream)
{
    bool ret = true;
    for(auto iter=m_moves.begin(); iter!=m_current_move; iter++)
    {
        stream << "[";
        ret = ret && write_vec2(stream, (*iter)->get_src_pos());
        stream << ",";
        ret = ret && write_vec2(stream, (*iter)->get_dst_pos());
        stream << "] ";
    }
    return ret;
}

bool ChessBoard::load_game(std::istream& stream)
{
    if( !stream ) {
        return false;
    }
    reset_board();

    bool res = true;
    vec2 src, dst;
    while( stream )
    {
        char ch;
        stream >> ch;
        if( stream.eof() ) {
            break;
        }
        res = res && read_vec2(stream, src);
        stream >> ch;
        res = res && read_vec2(stream, dst);
        stream >> ch;
        if( !res || !make_move(src, dst)) {
            return false;
        }
    }
    return true;
}
