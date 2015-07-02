#include "chessfieldmodel.h"

#include <utility>
#include <string>
#include <iostream>
#include <fstream>

inline int from_board_to_list(const vec2& ind)
{
    return (7-ind[0])*8+ind[1];
}

ChessFieldModel::ChessFieldModel(QObject *parent) :
    QAbstractListModel(parent), m_chess_piece_images(ChessBoard::BK_PAWN+1)
{
    m_role_names[CELL_COLOR] = "cell_color";
    m_role_names[IMAGE_PATH] = "image_path";

    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            m_list.append(std::pair<QString,QString>("white",""));
            m_list.append(std::pair<QString,QString>("brown",""));
        }
        for(int j=0; j<4; j++) {
            m_list.append(std::pair<QString,QString>("brown",""));
            m_list.append(std::pair<QString,QString>("white",""));
        }
    }

    m_chess_piece_images[ChessBoard::NONE]      = "";
    m_chess_piece_images[ChessBoard::WT_KING]   = "img/assets/wt_king.png";
    m_chess_piece_images[ChessBoard::WT_QUEEN]  = "img/assets/wt_queen.png";
    m_chess_piece_images[ChessBoard::WT_BISHOP] = "img/assets/wt_bishop.png";
    m_chess_piece_images[ChessBoard::WT_KNIGHT] = "img/assets/wt_knight.png";
    m_chess_piece_images[ChessBoard::WT_CASTLE] = "img/assets/wt_castle.png";
    m_chess_piece_images[ChessBoard::WT_PAWN]   = "img/assets/wt_pawn.png";

    m_chess_piece_images[ChessBoard::BK_KING]   = "img/assets/bk_king.png";
    m_chess_piece_images[ChessBoard::BK_QUEEN]  = "img/assets/bk_queen.png";
    m_chess_piece_images[ChessBoard::BK_BISHOP] = "img/assets/bk_bishop.png";
    m_chess_piece_images[ChessBoard::BK_KNIGHT] = "img/assets/bk_knight.png";
    m_chess_piece_images[ChessBoard::BK_CASTLE] = "img/assets/bk_castle.png";
    m_chess_piece_images[ChessBoard::BK_PAWN]   = "img/assets/bk_pawn.png";

    clean_board();
}

void ChessFieldModel::reset_board()
{
    m_chess_board.reset_board();
    update_model();
}

void ChessFieldModel::clean_board()
{
    m_chess_board.clean_board();
    update_model();
}

void ChessFieldModel::make_move(int src_cell, int dst_cell)
{
    if( m_chess_board.get_board_piece(7 - src_cell / 8, src_cell % 8) == ChessBoard::NONE ) {
        return;
    }
    auto src = std::make_pair(7 - src_cell/8, src_cell%8);
    auto dst = std::make_pair(7 - dst_cell/8, dst_cell%8);

    auto res = m_chess_board.make_move(src, dst);
    if( !res ) {
        return ;
    }
    update_cells(res);
}

bool ChessFieldModel::save_game(QUrl file)
{
    QString qstr = file.path();
    QByteArray ba = qstr.toLatin1();
    const char *fname = ba.data();
    if(!fname || !*fname) {
        return false;
    }
    //const char* fname = qstr.toUtf8().constData();
    std::ofstream out(fname+1);
    if(out) {
        bool ret = m_chess_board.save_game(out);
        out.close();
        return ret;
    }
    return false;
}
bool ChessFieldModel::load_game(QUrl file)
{
    QString qstr = file.path();
    QByteArray ba = qstr.toLatin1();
    const char *fname = ba.data();
    if(!fname || !*fname) {
        return false;
    }
    std::ifstream in(fname+1);// because of trailing '/'
    if(in) {
        bool ret = m_chess_board.load_game(in);
        in.close();
        update_model();
        return ret;
    }
    return false;
}


bool ChessFieldModel::undo()
{
    auto res = m_chess_board.undo();
    if( !res ) {
        return false;
    }
    update_cells(res);
    return true;
}

bool ChessFieldModel::redo()
{
    auto res = m_chess_board.redo();
    if( !res ) {
        return false;
    }
    update_cells(res);
    return true;
}

QVariantMap ChessFieldModel::get(int row) const
{
    QVariantMap res;
    if( row >= 0 && row < m_list.count() ) {
        res["cell_color"] = m_list[row].first;
        res["image_path"] = m_list[row].second;
    }
    return res;
}
/*
void ChessFieldModel::setImagePath(int row, const QVariant& val)
{
    if( row < 0 || row > m_list.count() ) {
        return;
    }
    m_list[row].second = val.toString();

    QModelIndex index;
    index.child(row,0);
    QVector<int> roles(1,IMAGE_PATH);

    emit dataChanged(index, index, roles);
}
*/
QVariant ChessFieldModel::data(const QModelIndex &index, int role) const
{
    if( index.row() < 0 || index.row() > m_list.count() ) {
        return QVariant();
    }
    switch(role) {
        case CELL_COLOR:
            return QVariant(m_list[index.row()].first);
        case IMAGE_PATH:
            return QVariant(m_list[index.row()].second);
        default:
            return QVariant();
    }

}
bool ChessFieldModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( index.row() < 0 || index.row() > m_list.count() ) {
        return false;
    }
    switch(role) {
        case CELL_COLOR:
            m_list[index.row()].first = value.toString();
            emit dataChanged(index, index);
            return true;
        case IMAGE_PATH:
            m_list[index.row()].second = value.toString();
            emit dataChanged(index, index);
            return true;
        default:
            return false;
    }
}

void ChessFieldModel::update_cells(std::shared_ptr<ChessPieceMove> move)
{
    QVector<int> roles(1, IMAGE_PATH);
    const int count = move->changed_cells_count();
    auto cells = move->get_changed_cells();

    for(int i=0; i<count; i++) {
        ChessBoard::ChessPiece cp = cells[i].second;
        int ind = from_board_to_list(cells[i].first);
        m_list[ind].second = m_chess_piece_images[cp];
        emit dataChanged(index(ind), index(ind), roles);
    }
}

void ChessFieldModel::update_model()
{
    QList<std::pair<QString,QString> >::iterator iter = m_list.begin();
    for(int i=0; iter != m_list.end(); i++, iter++) {
        iter->second = m_chess_piece_images[m_chess_board.get_board_piece(7 - i / 8, i % 8)];
    }
    QVector<int> roles(1, IMAGE_PATH);
    emit dataChanged(index(0), index(m_list.size()-1), roles);
}
