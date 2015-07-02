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
        case WT_KING: case BK_KING:
            result = king_move(src,dst);
            break;
        default:
            //result = WRONG;
            break;
    }

    if( !result || !result->apply(m_chess_board, m_1st_move_flags, *this) ) {
        return shared_ptr<ChessPieceMove>(NULL);
    }

    m_current_side = 1-m_current_side;

    // in case when we have loaded game
    m_moves.erase(m_current_move, m_moves.end());

    m_moves.push_back(result);
    m_current_move = m_moves.end();

    return result;
}

std::shared_ptr<ChessPieceMove> ChessBoard::undo()
{

    if(m_current_move != m_moves.begin()) {
        auto prev = m_current_move; prev--;
        if ((*prev)->undo(m_chess_board, m_1st_move_flags)) {
            m_current_move = prev;
            m_current_side = 1-m_current_side;
            return shared_ptr<ChessPieceMove>(*(m_current_move));
        }
    }
    return shared_ptr<ChessPieceMove>(NULL);
}

std::shared_ptr<ChessPieceMove> ChessBoard::redo()
{
    if(m_current_move != m_moves.end() &&
      (*m_current_move)->apply(m_chess_board, m_1st_move_flags, *this))
    {
        m_current_side = 1-m_current_side;
        return shared_ptr<ChessPieceMove>(*(m_current_move++));
    }
    return shared_ptr<ChessPieceMove>(NULL);
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
    return simple_move(src,dst);
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

std::shared_ptr<ChessPieceMove> ChessBoard::king_move(const vec2& src, const vec2& dst)
{
    if( abs(src[0]-dst[0]) <=1 && abs(src[1]-dst[1]) <=1 ) {
        return simple_move(src,dst);
    }
    ChessPiece king = m_chess_board[src[0]][src[1]];
    ChessPiece castle = m_chess_board[dst[0]][dst[1]];
    if( castle == WT_CASTLE || castle == BK_CASTLE ){
        return make_shared<Castling>(src, king, dst, castle);
    } else {
        return shared_ptr<ChessPieceMove>(NULL);
    }
}

std::shared_ptr<ChessPieceMove> ChessBoard::simple_move(const vec2& src, const vec2& dst) const
{
    ChessPiece src_piece = m_chess_board[src[0]][src[1]];
    ChessPiece dst_piece = m_chess_board[dst[0]][dst[1]];
    if( dst_piece != NONE && ((is_black(src_piece) && is_black(dst_piece)) || (is_white(src_piece) && is_white(dst_piece))) ) {
        return shared_ptr<ChessPieceMove>(NULL);
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
    for(auto iter=m_moves.begin(); iter!=m_current_move; iter++) {
        stream << (*iter)->to_string() << "\n";
    }
    return true;
}
bool ChessBoard::load_game(std::istream& stream)
{
    if( !stream ) {
        return false;
    }

    reset_board();

    static char line[100];
    while( stream ) {
        stream.getline(line, 100);
        auto val = ChessPieceMove::from_string(line);
        if( !val || !val->apply(m_chess_board, m_1st_move_flags, *this)) {
            return false;
        }
        m_current_side = 1-m_current_side;
        m_moves.push_back(val);
        m_current_move = m_moves.end();
    }
    return true;
}
