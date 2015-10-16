#ifndef TREEITEM_H
#define TREEITEM_H
#include <QObject>
#include <QStringList>

class TreeItem : public QObject
{
    Q_OBJECT
public:
    explicit TreeItem(long rootid,TreeItem *parent = 0);
    
    TreeItem* childAt(int i) const;

    TreeItem* parent() const{
        return m_parent;
    }
    bool hasChildren(){
            return (!(m_fetchedMore && m_children.empty()));
    }
    int childCount()const{
        return m_children.count();
    }

    void setFectMore(bool flag);

    bool canFetchMore() const;

    void append(TreeItem *item);

    long id;

private:
    QList<TreeItem*> m_children;
    TreeItem* m_parent;
    bool m_fetchedMore;

    int fieldNumber;
};

#endif // TREEITEM_H
