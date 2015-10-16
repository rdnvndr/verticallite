#ifndef TREEPROPVIEW_H
#define TREEPROPVIEW_H

#include <QTreeView>
#include <QIcon>

class  TreePropView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreePropView(QWidget *parent = 0);
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void mousePressEvent(QMouseEvent *event);
    void setModel(QAbstractItemModel *model);

signals:
    
public slots:
    void updateHeaderRow(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    QColor calcBgColor(const QModelIndex &index) const;

    QIcon m_iconClosed;
    QIcon m_iconOpen;
    QVector<QColor> m_colors;
};

#endif // TREEVIEWPROP_H
