#ifndef TREENODE_H
#define TREENODE_H
struct TreeNode
{
    long id;
    long parent;
    QList<long> children;
    QList<QVariant> data;
    bool isFetch;
};
#endif // TREENODE_H
