#ifndef CHESSFIELDMODEL_H
#define CHESSFIELDMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QUrl>

#include <utility>
#include "chessboard.h"
#include "chesspiecemove.h"

class ChessFieldModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        CELL_COLOR = Qt::UserRole+1,
        IMAGE_PATH
    };
    explicit ChessFieldModel(QObject *parent = 0);
    virtual ~ChessFieldModel(){}

    Q_INVOKABLE QVariantMap get(int row) const;
    Q_INVOKABLE void setImagePath(int row, const QVariant& val);

    Q_INVOKABLE void clean_board();
    Q_INVOKABLE void reset_board();
    Q_INVOKABLE void make_move(int src_cell, int dest_cell);
    Q_INVOKABLE bool load_file(QUrl file);


    virtual QHash<int,QByteArray> roleNames() const                                     {   return m_role_names;    }
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const               {    Q_UNUSED(parent); return m_list.count();   }
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const            {    Q_UNUSED(parent); return 1;    }
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        /*
        if (!index.isValid())
            return Qt::ItemIsEnabled;
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
        */
        Q_UNUSED(index);
        return Qt::ItemIsEditable;
    }


signals:

public slots:

private:
    QVector<QString> m_chess_piece_images;
    Q_DISABLE_COPY(ChessFieldModel)
    QList<std::pair<QString, QString> > m_list;
    QHash<int,QByteArray> m_role_names;
    ChessBoard m_chess_board;
};


#endif // CHESSFIELDMODEL_H
