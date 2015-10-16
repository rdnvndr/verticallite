#include "treeitem.h"
#include "converttype.h"
#include "loodsmansystem.h"
#include <QDebug>
#include <QMessageBox>
#include "midasdata.h"

TreeItem::TreeItem(long rootid, TreeItem *parent) :
    QObject(parent)
{
    id = rootid;
    m_parent = parent;
    m_fetchedMore = false;
}

TreeItem *TreeItem::childAt(int i) const
{
    return (m_children.count() > i)? m_children.at(i) : 0;
}

void TreeItem::setFectMore(bool flag)
{
    this->m_fetchedMore = flag;
}

bool TreeItem::canFetchMore() const
{
    return !m_fetchedMore;
}

void TreeItem::append(TreeItem *item)
{
    m_children.append(item);
}
