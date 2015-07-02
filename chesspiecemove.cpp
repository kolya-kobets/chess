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
 *  Auxiliary functions
 **/

static const std::string kPieceNames[ChessBoard::PIECES_COUNT] = {
    "NONE",
    "WT_KING",
    "WT_QUEEN",
    "WT_BISHOP",
    "WT_KNIGHT",
    "WT_CASTLE",
    "WT_PAWN",

    "BK_KING",
    "BK_QUEEN",
    "BK_BISHOP",
    "BK_KNIGHT",
    "BK_CASTLE",
    "BK_PAWN"
};

static unordered_map<string,ChessBoard::ChessPiece> kChessPieces ({
    { kPieceNames[ChessBoard::NONE],      ChessBoard::NONE },
    { kPieceNames[ChessBoard::WT_KING],   ChessBoard::WT_KING },
    { kPieceNames[ChessBoard::WT_QUEEN],  ChessBoard::WT_QUEEN },
    { kPieceNames[ChessBoard::WT_BISHOP], ChessBoard::WT_BISHOP },
    { kPieceNames[ChessBoard::WT_KNIGHT], ChessBoard::WT_KNIGHT },
    { kPieceNames[ChessBoard::WT_CASTLE], ChessBoard::WT_CASTLE },
    { kPieceNames[ChessBoard::WT_PAWN],   ChessBoard::WT_PAWN },

    { kPieceNames[ChessBoard::BK_KING],   ChessBoard::BK_KING },
    { kPieceNames[ChessBoard::BK_QUEEN],  ChessBoard::BK_QUEEN },
    { kPieceNames[ChessBoard::BK_BISHOP], ChessBoard::BK_BISHOP },
    { kPieceNames[ChessBoard::BK_KNIGHT], ChessBoard::BK_KNIGHT },
    { kPieceNames[ChessBoard::BK_CASTLE], ChessBoard::BK_CASTLE },
    { kPieceNames[ChessBoard::BK_PAWN],   ChessBoard::BK_PAWN }
});

string to_string(ChessBoard::ChessPiece v)
{
    ostringstream str;
    str << kPieceNames[v];
    return str.str();
}
string to_string(ChessPieceMove::ChessPiece cp, const vec2& pos)
{
    ostringstream str;
    str << kPieceNames[cp] << " [" << pos[0] << "," << pos[1] << "]";
    return str.str();
}
string to_string(const ChessPieceMove::BoardCell& v)
{
    return to_string(v.second, v.first);
}

bool read_piece(std::istream& in, ChessBoard::ChessPiece& cp)
{
    std::string str;
    in >> str;
    cp = kChessPieces[str];
    return true;
}

bool read_vec2(std::istream& in, vec2& v)
{
    char ch;
    in >> ch >> v[0] >> ch >> v[1] >> ch;
    return true;
}


/*
 *  ChessPieceMove implementation
 */

std::shared_ptr<ChessPieceMove> ChessPieceMove::from_string(const std::string& str)
{
    static const char* kMoveNames[] = {
        "SIMPLE", "PROMOTION", "CASTLING"
    };
    string move;
    istringstream in(str);
    in >> move;
    if(!move.compare(kMoveNames[0]) ) {
        return SimpleMove::from_string(str);
    } else if ( !move.compare(kMoveNames[1]) ) {
        return PawnMoveWithPromotion::from_string(str);
    } else if ( !move.compare(kMoveNames[2]) ) {
        return Castling::from_string(str);
    } else {
        return std::shared_ptr<ChessPieceMove>(NULL);
    }
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

bool SimpleMove::apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS], const ChessBoard& board)
{
    if( m_is_applied ) {
        return true;
    }

    const vec2& from = m_changed_cells[0].first;
    const vec2& to = m_changed_cells[1].first;
    bool* ft_move1 = &fst_move[from[0]][from[1]];
    bool* ft_move2 = &fst_move[to[0]][to[1]];

    field[from[0]][from[1]] = m_changed_cells[0].second = ChessBoard::NONE;
    field[to[0]][to[1]] = m_changed_cells[1].second = m_chess_pieces.first;

    m_is_1st_move[0] = *ft_move1;
    m_is_1st_move[1] = *ft_move2;
    *ft_move1 = false;
    *ft_move2 = false;

    m_is_applied = true;

    if( board.is_king_under_attack() ) {
        undo(field, fst_move);
    }

    return m_is_applied;
}

bool SimpleMove::undo(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS])
{
    if( !m_is_applied ) {
        return false;
    }

    const vec2& from = m_changed_cells[0].first;
    const vec2& to = m_changed_cells[1].first;

    bool* ft_move1 = &fst_move[from[0]][from[1]];
    bool* ft_move2 = &fst_move[to[0]][to[1]];

    field[from[0]][from[1]] = m_changed_cells[0].second = m_chess_pieces.first;
    field[to[0]][to[1]] = m_changed_cells[1].second = m_chess_pieces.second;

    m_is_applied = false;

    *ft_move1 = m_is_1st_move[0];
    *ft_move2 = m_is_1st_move[1];

    return true;
}

std::string SimpleMove::to_string()
{
    ostringstream str;
    str << "SIMPLE (" << " "
        << ::to_string(m_chess_pieces.first, m_changed_cells[0].first) << ", "
        << ::to_string(m_chess_pieces.second, m_changed_cells[1].first) << " )";

    return str.str();
}

std::shared_ptr<ChessPieceMove> SimpleMove::from_string(const string& val)
{
    istringstream in(val);

    vec2 src, dst;
    ChessPiece src_cp, dst_cp;
    string str;
    char ch;
    in >> str >> ch ;
    if( str.compare("SIMPLE") != 0 ) {
        return std::shared_ptr<ChessPieceMove>(NULL);
    }

    read_piece(in, src_cp);
    read_vec2(in, src);
    in >> ch;
    read_piece(in, dst_cp);
    read_vec2(in, dst);
    in >> ch;

    return std::make_shared<SimpleMove>(src, src_cp, dst, dst_cp);
}

/*
 * PawnMoveWithPromotion implementation
 */

PawnMoveWithPromotion::PawnMoveWithPromotion(const vec2& src, ChessPiece src_cp,
                                             const vec2& dst,  ChessPiece dst_cp, ChessPiece promote_to):
    SimpleMove(src, src_cp, dst, dst_cp), m_promote_to(promote_to)
{}

bool PawnMoveWithPromotion::apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS], const ChessBoard& board)
{
    if( SimpleMove::apply(field, fst_move, board) ) {
        const vec2& m_to = m_changed_cells[1].first;
        field[m_to[0]][m_to[1]] = m_changed_cells[1].second = m_promote_to;
    }
    return m_is_applied;
}

std::string PawnMoveWithPromotion::to_string()
{
    ostringstream str;
    str << "PROMOTION (" << " " << ::to_string(m_promote_to) << " "
        << ::to_string(m_chess_pieces.first, m_changed_cells[0].first) << ", "
        << ::to_string(m_chess_pieces.second, m_changed_cells[1].first) << " )";

    return str.str();
}

std::shared_ptr<ChessPieceMove> PawnMoveWithPromotion::from_string(const string& val)
{
    istringstream in(val);

    vec2 src, dst;
    ChessPiece src_cp, dst_cp, promote_to;
    string str;
    char ch;

    in >> str >> ch ;
    if( str.compare("PROMOTION") != 0 ) {
        return std::shared_ptr<ChessPieceMove>(NULL);
    }

    read_piece(in, promote_to);

    read_piece(in, src_cp);
    read_vec2(in, src);
    in >> ch;
    read_piece(in, dst_cp);
    read_vec2(in, dst);
    in >> ch;

    return std::make_shared<PawnMoveWithPromotion>(src, src_cp, dst, dst_cp, promote_to);
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

bool Castling::apply(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS], const ChessBoard& board)
{
    static const int A = ChessBoard::A;
    static const int H = ChessBoard::H;

    static const int WT_CASTLE = ChessBoard::WT_CASTLE;
    static const int BK_CASTLE = ChessBoard::BK_CASTLE;

    if( m_is_applied ) {
        return m_is_applied;
    }

    const vec2& src = m_changed_cells[0].first;
    const vec2& dst = m_changed_cells[1].first;

    bool* ft_move1 = &fst_move[src[0]][src[1]];
    bool* ft_move2 = &fst_move[dst[0]][dst[1]];

    if( !*ft_move1 || !*ft_move2 ) {
        return false;
    }

    ChessPiece king = m_chess_pieces.first;
    ChessPiece castle = m_chess_pieces.second;

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

bool Castling::undo(ChessPiece (&field)[ROWS][COLS], bool (&fst_move)[ROWS][COLS])
{
    const vec2& k1 = m_changed_cells[0].first;
    const vec2& k2 = m_changed_cells[2].first;
    const vec2& c1 = m_changed_cells[1].first;
    const vec2& c2 = m_changed_cells[3].first;

    bool* ft_move1 = &fst_move[k1[0]][k1[1]];
    bool* ft_move2 = &fst_move[c1[0]][c1[1]];


    if( !m_is_applied || !ft_move1 || !ft_move2 ) {
        return false;
    }
    *ft_move1 = *ft_move2 = true;

    field[k2[0]][k2[1]] = m_changed_cells[2].second = ChessBoard::NONE;
    field[c2[0]][c2[1]] = m_changed_cells[3].second = ChessBoard::NONE;

    field[k1[0]][k1[1]] =  m_changed_cells[0].second = m_chess_pieces.first;
    field[c1[0]][c1[1]] =  m_changed_cells[1].second = m_chess_pieces.second;

    m_is_applied = false;

    return true;
}

std::string Castling::to_string()
{
    ostringstream str;
    str << "CASTLING ( " <<  ::to_string(m_chess_pieces.first, m_changed_cells[0].first) << ", "
        << ::to_string(m_chess_pieces.second, m_changed_cells[1].first) << " )";

    return str.str();

}

std::shared_ptr<ChessPieceMove> Castling::from_string(const std::string& val)
{
    istringstream in(val);

    vec2 src, dst;
    ChessPiece king, castle;
    string str;
    char ch;

    in >> str >> ch ;
    if( str.compare("CASTLING") != 0 ) {
        return std::shared_ptr<ChessPieceMove>(NULL);
    }

    read_piece(in, king);
    read_vec2(in, src);
    in >> ch;
    read_piece(in, castle);
    read_vec2(in, dst);
    in >> ch;

    return std::make_shared<Castling>(src, king, dst, castle);
}
