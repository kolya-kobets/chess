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

inline bool is_white(ChessBoard::ChessPiece cp)
{
    return cp >= ChessBoard::WT_KING && cp <= ChessBoard::WT_PAWN;
}
inline bool is_black(ChessBoard::ChessPiece cp)
{
    return cp >= ChessBoard::BK_KING && cp <= ChessBoard::BK_PAWN;
}
inline bool is_pawn_ready_to_promotion(const vec2& idx, ChessBoard::ChessPiece cp)
{
    return (cp == ChessBoard::WT_PAWN && idx[0] == 7) || (cp == ChessBoard::BK_PAWN && idx[0] == 0);
}

std::shared_ptr<ChessPieceMove> ChessBoard::make_move(const std::pair<int,int>& from, const std::pair<int,int>& to)
{
    vec2 src = make_vec2(from);
    vec2 dst = make_vec2(to);
    ChessPiece piece = m_chess_board[src[0]][src[1]];
    if( (m_current_side == WHITE && is_black(piece)) || (m_current_side == BLACK && is_white(piece)) ) {
        return shared_ptr<ChessPieceMove>(NULL);
    }

    auto result = shared_ptr<ChessPieceMove>(NULL);

    switch( piece ) {
        case WT_PAWN: case BK_PAWN:
            result = pawn_move(src,dst);
            break;
        case WT_CASTLE: case BK_CASTLE:
            result = castle_move(src,dst);
            break;
        case WT_BISHOP: case BK_BISHOP:
            result = bishop_move(src,dst);
            break;
        case WT_KNIGHT: case BK_KNIGHT:
            result = knight_move(src,dst);
            break;
        case WT_QUEEN: case BK_QUEEN:
            result = queen_move(src,dst);
            break;
        case WT_KING: case BK_KING: {
            /*vec2 src2 = make_vec2(from2);
            vec2 dst2 = make_vec2(to2);
            result = move_king(src,dst, src2, dst2);
            // in case of castling all actions have been performed
            if(result == CASTLING) {
                m_current_side = 1-m_current_side;
                m_is_king_moved[piece == WT_KING] = true;
                from2 = make_pair(src2[0],src2[1]);
                to2 = make_pair(dst2[0],dst2[1]);
                res = make_pair(piece, m_chess_board[dst2[0]][dst2[1]]);
                return CASTLING;
            }
            */
            break;
        }
        case NONE:
            //result = WRONG;
            break;
    }

    if( !result || !result->apply(m_chess_board, *this) ) {
        return result;
    }

    //res = tmp;
    m_current_side = 1-m_current_side;
    //check_castle_first_move(src, piece);

    //if(piece == WT_KING || piece == BK_KING) {
//        m_is_king_moved[piece == WT_KING] = true;
  //  }

    return result;
}

std::shared_ptr<ChessPieceMove> ChessBoard::pawn_move(const vec2& src, const vec2& dst) const
{
    ChessPiece piece = m_chess_board[src[0]][src[1]];
    ChessPiece dst_piece = m_chess_board[dst[0]][dst[1]];
    std::shared_ptr<ChessPieceMove> ret = shared_ptr<ChessPieceMove>(NULL);

    bool is_allowed = false;
    if ( piece == WT_PAWN ) {
        //first move
        if( dst[0] == src[0]+2 ) {
            is_allowed = dst[1] == src[1] && m_chess_board[src[0]+1][src[1]] == NONE &&
                m_chess_board[dst[0]][src[1]] == NONE && src[0] == 1;
        } else if( dst[0] == src[0]+1 ) {
            //replace
            is_allowed = (abs(dst[1] - src[1]) == 1 && m_chess_board[dst[0]][dst[1]] != NONE &&
                         is_black(dst_piece)) ||
            //move only
                         (dst[1] == src[1]  && m_chess_board[dst[0]][dst[1]] == NONE);
        }
    } else {
        //first move
        if( dst[0] == src[0]-2 ) {
            is_allowed = dst[1] == src[1] && m_chess_board[src[0]-1][src[1]] == NONE &&
                m_chess_board[dst[0]][src[1]] == NONE && src[0] == 6;
        } else if( dst[0] == src[0]-1 ) {
            //replace
            is_allowed = (abs(dst[1] - src[1]) == 1 && m_chess_board[dst[0]][dst[1]] != NONE &&
                         is_white(dst_piece)) ||
            //simple move
                         (dst[1] == src[1] && m_chess_board[dst[0]][dst[1]] == NONE );
        }
    }

    if( is_allowed ) {
        if( is_pawn_ready_to_promotion(dst,piece) ) {
            ChessPiece promote_to = is_black(piece)?BK_QUEEN:WT_QUEEN;
            return make_shared<PawnMoveWithPromotion>(src,piece, dst, dst_piece, promote_to);
        } else {
            return make_shared<SimpleMove>(src,piece, dst, dst_piece);
        }
    }

    return shared_ptr<ChessPieceMove>(NULL);
}

std::shared_ptr<ChessPieceMove> ChessBoard::castle_move(const vec2& src, const vec2& dst)
{
    if ( std::abs(src[0] - dst[0]) >= 1 && std::abs(src[1] - dst[1]) >= 1 ) {
        return std::shared_ptr<ChessPieceMove>(NULL);
    }
    using std::min;
    using std::max;
    //check whether any piece is located between our positions
    if( std::abs(src[0] - dst[0]) >= 1 ) {
        int beg = min(src[0],dst[0]);
        int end = max(src[0],dst[0]);
        for(int i=beg+1; i<end; i++) {
            if( m_chess_board[i][src[1]] != NONE ) {
                return shared_ptr<ChessPieceMove>(NULL);
            }
        }
    } else {
        int beg = min(src[1],dst[1]);
        int end = max(src[1],dst[1]);
        for(int i=beg+1; i<end; i++) {
            if( m_chess_board[src[0]][i] != NONE ) {
                return shared_ptr<ChessPieceMove>(NULL);
            }
        }
    }
    return simple_move(src,dst,get_castle_first_move_flag(src));
}

std::shared_ptr<ChessPieceMove> ChessBoard::bishop_move(const vec2& from, const vec2& to)
{
    vec2 src = from, dst = to;
    if( abs(src[0]-dst[0]) != abs(src[1]-dst[1]) ) {
        return shared_ptr<ChessPieceMove>(NULL);
    }
    if(src[0] > dst[0]) {
        std::swap(src,dst);
    }
    //check whether any piece is located between our positions
    int inc = (src[1]<dst[1])?1:-1;
    for(int i=src[0]+1, j=src[1]+inc; i<dst[0]; i++, j+=inc){
        if( m_chess_board[i][j] != NONE ) {
            return shared_ptr<ChessPieceMove>(NULL);
        }
    }

    return simple_move(from,to);
}

std::shared_ptr<ChessPieceMove> ChessBoard::knight_move(const vec2& src, const vec2& dst) const
{
    if( (abs(src[0]-dst[0]) == 2 && abs(src[1]-dst[1]) == 1) ||
        (abs(src[0]-dst[0]) == 1 && abs(src[1]-dst[1]) == 2) ) {
        return simple_move(src,dst);
    }
    return shared_ptr<ChessPieceMove>(NULL);
}

std::shared_ptr<ChessPieceMove> ChessBoard::queen_move(const vec2& src, const vec2& dst)
{
    std::shared_ptr<ChessPieceMove> result = castle_move(src,dst);
    if(!result) {
        result = bishop_move(src,dst);
    }
    return result;
}

std::shared_ptr<ChessPieceMove> ChessBoard::move_king(const vec2& src, const vec2& dst, vec2& src2, vec2& dst2)
{
    /*
    if( abs(src[0]-dst[0]) <=1 && abs(src[1]-dst[1]) <=1 ) {
        return check_replace(src,dst);
    }
    ChessPiece src_piece = m_chess_board[src[0]][src[1]];
    //ChessPiece dst_piece = m_chess_board[dst[0]][dst[1]];
    //check whether it's castling
    const int side = (src_piece == WT_KING) ? WHITE : BLACK;
    const int row = (side == WHITE) ? 0 : 7;
    if( src[0] != row || dst[0] != row || m_is_king_moved[side] ) {
        return WRONG;
    }
    if( !(( dst[1] == A  && !m_is_castle_moved[side][0] ) ||
          ( dst[1] == H  && !m_is_castle_moved[side][1] )) ) {
        return WRONG;
    }
    int inc = (dst[1] == H) ? 1 : -1;
    //check that there is no piece between king & castle
    for(int i=src[1]+inc; i != dst[1]; i+=inc) {
        if(m_chess_board[row][i] != NONE) {
            return WRONG;
        }
    }

    int king_cln = src[1];
    if(is_king_under_attack()) {
        return WRONG;
    }
    for(int i=0; i<2; i++) {
        king_cln+=inc;
        std::swap(m_chess_board[row][king_cln-inc],m_chess_board[row][king_cln]);
        if(is_king_under_attack()) {
            std::swap(m_chess_board[row][src[1]],m_chess_board[row][king_cln]);
            return WRONG;
        }
    }

    src2 = make_vec2(row,king_cln);
    dst2 = make_vec2(row,king_cln-inc);
    std::swap(m_chess_board[row][dst[1]], m_chess_board[row][dst2[1]]);

    return CASTLING;
    */
    return shared_ptr<ChessPieceMove>(NULL);
}

std::shared_ptr<ChessPieceMove> ChessBoard::simple_move(const vec2& src, const vec2& dst, bool *first_move) const
{
    ChessPiece src_piece = m_chess_board[src[0]][src[1]];
    ChessPiece dst_piece = m_chess_board[dst[0]][dst[1]];
    if( dst_piece != NONE &&(is_black(src_piece) && is_black(dst_piece)) || (is_white(src_piece) && is_white(dst_piece)) ) {
        return shared_ptr<ChessPieceMove>(NULL);
    } else {
        return make_shared<SimpleMove>(src,src_piece,dst,dst_piece,first_move);
    }
}


bool* ChessBoard::get_castle_first_move_flag(const vec2& src)
{
    ChessPiece cp = m_chess_board[src[0]][src[1]];
    if( cp == WT_CASTLE ) {
        if( src[0] == 0 ) {
            if( src[1] == A ) {
                return m_is_castle_moved[WHITE];
            } else if( src[1] == H ) {
                return m_is_castle_moved[WHITE]+1;
            }
        }
    } else if ( cp == BK_CASTLE ) {
        if( src[0] == 7 ) {
            if( src[1] == A ) {
                return m_is_castle_moved[BLACK];
            } else if( src[1] == H ) {
                return m_is_castle_moved[BLACK]+1;
            }
        }
    }
    return NULL;
}



bool ChessBoard::is_king_under_attack() const
{
    // TODO: implement
    return false;
}

void ChessBoard::reset_board()
{
    clean_board();
    m_chess_board[0][A] = m_chess_board[0][H] = WT_CASTLE;
    m_chess_board[0][B] = m_chess_board[0][G] = WT_KNIGHT;
    m_chess_board[0][C] = m_chess_board[0][F] = WT_BISHOP;
    m_chess_board[0][D] = WT_QUEEN; m_chess_board[0][E] = WT_KING;

    m_chess_board[7][A] = m_chess_board[7][H] = BK_CASTLE;
    m_chess_board[7][B] = m_chess_board[7][G] = BK_KNIGHT;
    m_chess_board[7][C] = m_chess_board[7][F] = BK_BISHOP;
    m_chess_board[7][D] = BK_QUEEN; m_chess_board[7][E] = BK_KING;

    for(int i=0; i<8; i++) {
        m_chess_board[1][i] = WT_PAWN;
        m_chess_board[6][i] = BK_PAWN;
    }

    std::fill_n(m_is_king_moved,2, false);
    std::fill_n(m_is_castle_moved[0],2, false);
    std::fill_n(m_is_castle_moved[1],2, false);

    m_current_side = WHITE;
}
void ChessBoard::clean_board()
{
    memset(m_chess_board,0,sizeof(m_chess_board));
}
