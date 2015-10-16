#ifndef PROPEDITORDELEGATE_H
#define PROPEDITORDELEGATE_H

#include <QItemDelegate>
#include "attrloodsmanmodel.h"

class PropEditorDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit PropEditorDelegate(QObject *parent = 0);
    virtual QWidget * createEditor ( QWidget * parent,
                                     const QStyleOptionViewItem & option,
                                     const QModelIndex & index ) const;
    virtual void setEditorData( QWidget * editor,
                                 const QModelIndex & index ) const;
    virtual void setModelData( QWidget * editor, QAbstractItemModel * model,
                               const QModelIndex & index ) const;
    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex & index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;
    QColor calcBgColor(const QModelIndex &index) const;

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    void setAttrModel(AttrLoodsmanModel* model);
signals:
    
public slots:

private:
     QVector<QColor> m_colors;
     AttrLoodsmanModel* m_attrModel;
};

#endif // PROPEDITORDELEGATE_H
