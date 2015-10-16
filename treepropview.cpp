#include "treepropview.h"
#include <QPainter>
#include <QApplication>
#include <QMouseEvent>
#include <QHeaderView>
#include <QDebug>
#include <QRect>

TreePropView::TreePropView(QWidget *parent) :
    QTreeView(parent)
{
    m_colors.reserve(6);
    m_colors.push_back(QColor(255, 230, 191));
    m_colors.push_back(QColor(255, 255, 191));
    m_colors.push_back(QColor(191, 255, 191));
    m_colors.push_back(QColor(199, 255, 255));
    m_colors.push_back(QColor(234, 191, 255));
    m_colors.push_back(QColor(255, 191, 239));

    QPixmap pix(14, 14);
    pix.fill(Qt::transparent);
    QStyleOption branchOption;
    QRect r(QPoint(0, 0), pix.size());
    branchOption.rect = QRect(2, 2, 9, 9);
    branchOption.palette = this->palette();
    branchOption.state = QStyle::State_Children;
    QPainter p;
    // Закрытое состояние
    p.begin(&pix);
    style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
    p.end();
    QIcon rc = pix;
    rc.addPixmap(pix, QIcon::Selected, QIcon::Off);
    m_iconClosed = QIcon(pix);
    // Открытое состояние
    branchOption.state |= QStyle::State_Open;
    pix.fill(Qt::transparent);
    p.begin(&pix);
    style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
    p.end();
    m_iconOpen = QIcon(pix);

    setRootIsDecorated(false);
    setAlternatingRowColors(true);
}

void TreePropView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV3 opt = option;

    if (!index.parent().isValid()) {
        const QColor c = option.palette.color(QPalette::Dark);
        painter->fillRect(option.rect, c);
        opt.font.setBold(true);
    } else {
        const QColor c = calcBgColor(index);
        if (c.isValid()) {
            painter->fillRect(option.rect, c);
            opt.palette.setColor(QPalette::AlternateBase, c.lighter(112));
        }
    }
    QTreeView::drawRow(painter, opt, index);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();
}

void TreePropView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()&& (event->button() == Qt::LeftButton)) {
        if (index.parent().isValid()) {
            if (!index.column()==0)
                edit(index);
        } else if (!rootIsDecorated()) {
            setExpanded(index,!isExpanded(index));
            if (isExpanded(index))
                model()->setData(index,m_iconOpen,Qt::DecorationRole);
            else
                model()->setData(index,m_iconClosed,Qt::DecorationRole);
        }
        setCurrentIndex(index);
        return;
    }
    if (index.column()==0)
        return;

    QTreeView::mousePressEvent(event);
}

void TreePropView::setModel(QAbstractItemModel *model)
{
    connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(updateHeaderRow(QModelIndex,QModelIndex)));
    QTreeView::setModel(model);
    this->expandAll();
    for (int i=0;i<model->rowCount();i++){
        this->setFirstColumnSpanned(i,QModelIndex(),true);
        QModelIndex index = model->index(i,0);
        if (isExpanded(index))
            model->setData(index,m_iconOpen,Qt::DecorationRole);
        else
            model->setData(index,m_iconClosed,Qt::DecorationRole);
    }
}

void TreePropView::updateHeaderRow(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(bottomRight)

    if (!this->rootIsDecorated() && !topLeft.parent().isValid())
        setFirstColumnSpanned(topLeft.row(),QModelIndex(),true);
}

QColor TreePropView::calcBgColor(const QModelIndex &index) const
{
    QModelIndex i = index;
    while (i.parent().isValid())
        i = i.parent();

    long rowColor;
    if (i.row()<m_colors.count())
        rowColor = i.row();
    else
        rowColor = i.row()%m_colors.count();

    return m_colors.at(rowColor);
}
