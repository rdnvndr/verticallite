#include "linkloodsmanmodel.h"
#include "loodsmansystem.h"
#include "converttype.h"
#include <QMessageBox>
#include <QIcon>

LinkLoodsmanModel::LinkLoodsmanModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    dataSet = new MidasData();
    m_root =  new TreeNode;
    m_root->id = 0;
    m_root->isFetch = false;
    cache.insert(0,m_root);

    QStringList fields;
    fields << "_ID" << "_NAME" << "_INVERSENAME" << "_TYPE" << "_ORDER";
    setFields(fields);
    setFieldIcon("_ICON");

}

LinkLoodsmanModel::~LinkLoodsmanModel()
{
    delete dataSet;
}

int LinkLoodsmanModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_fieldNames.count();
}

TreeNode *LinkLoodsmanModel::getItem(const QModelIndex & index) const
{
    if (index.isValid())
        return static_cast<TreeNode*>(index.internalPointer());
    else
        return m_root;
}

QVariant LinkLoodsmanModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole && index.column()==0 && !m_fieldIcon.isEmpty()){
        QPixmap pixmap;
        const TreeNode *item = getItem(index);
        if (item) {
            pixmap.loadFromData(item->data.at(m_fieldNames.count()).toByteArray());
            return QIcon(pixmap);
        }
    }

    if(role == Qt::DisplayRole){
        const TreeNode *item = getItem(index);
        if (item)
            return item->data.at(index.column());
    }
    return QVariant();
}

QVariant LinkLoodsmanModel::data(long id, int column) const
{

    if(cache.contains(id)){
        TreeNode *item = cache[id];
        if (item)
            return item->data.at(column);
    }
    return QVariant();
}

bool LinkLoodsmanModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(index);
    if (role != Qt::EditRole)
        return false;
    return 0;
}

Qt::ItemFlags LinkLoodsmanModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    /*if ( d->isEditable( index.internalId(), index.column() ) )
        fl |= Qt::ItemIsEditable;*/
    return fl;
}

QVariant LinkLoodsmanModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_fieldNames.at(section);
    }
    return QVariant();
}

bool LinkLoodsmanModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(section);

    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return false;
}

QModelIndex LinkLoodsmanModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    const TreeNode* parentItem = getItem(parent);

    if (!parentItem)
        return QModelIndex();

    TreeNode* childItem = cache[parentItem->children.at(row)];

    if (childItem)
            return createIndex(row, column, (void*)childItem);
    else
            return QModelIndex();
}

QModelIndex LinkLoodsmanModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    const TreeNode *childItem = getItem(index);
    TreeNode *parentItem = cache[childItem->parent];

    if (!parentItem || parentItem == m_root)
        return QModelIndex();

    TreeNode *grandparent = cache[parentItem->parent];
    if (!grandparent)
        return QModelIndex();

    for (int i = 0; i < grandparent->children.count(); i++)
        if (grandparent->children.at(i) == parentItem->id)
            return createIndex(i, 0, (void*) parentItem);

    return QModelIndex();
}

int LinkLoodsmanModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0)
        return 0;

    return getItem(parent)->children.count();
}

bool LinkLoodsmanModel::hasChildren(const QModelIndex & parent) const
{
    TreeNode* node = getItem(parent);
    if (node->isFetch && node->children.count()==0)
        return  false;

    return true;
}

bool LinkLoodsmanModel::canFetchMore(const QModelIndex &parent) const
{
    return !getItem(parent)->isFetch;
}

void LinkLoodsmanModel::fetchMore(const QModelIndex &parent)
{
    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;

    TreeNode *item;

    if(parent.isValid()){
        item = getItem(parent);
        if(!item)
            return;
    }

    _variant_t data = loodsman->main->GetInfoAboutCurrentBase(9,&inErrorCode, &stErrorMessage);
    dataSet->setData((unsigned char *)data.parray->pvData);

    if (inErrorCode.lVal!=0)
        QMessageBox::warning(NULL, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));


    if (dataSet->first()){
        do {
            int childId = dataSet->fieldValue("_ID").toInt();
            long parentId = 0/*dataSet->fieldValue("_ID_PARENT_TYPE").toInt()*/;

            TreeNode *item = /*cache[parentId]*/m_root;
            if (item==NULL){
                if (parentId == 0)
                    item = m_root;
                else {
                    item = new TreeNode;
                    item->id = parentId;
                    cache.insert(parentId, item);
                    item->isFetch = true;
                }
            }

            TreeNode* childItem;
            if (!cache.contains(childId)){
                childItem = new TreeNode;
                childItem->id = childId;
                childItem->isFetch = true;
                cache.insert(childId, childItem);
                item->children.append(childId);
            } else {
                childItem = cache[childId];
                childItem->data.clear();
            }

            childItem->parent = parentId;

            for ( int j=0; j < m_fieldNames.count(); j++ )
                childItem->data.append(dataSet->fieldValue(m_fieldNames[j]));
            if (!m_fieldIcon.isEmpty())
                childItem->data.append(dataSet->fieldValue(m_fieldIcon));

        } while (dataSet->next());
        dataSet->clear();

        m_root->isFetch = true;
        beginInsertRows(QModelIndex(), 0,cache[0]->children.count());
        endInsertRows();
    }
}

QStringList LinkLoodsmanModel::fields()
{
    return m_fieldNames;
}

void LinkLoodsmanModel::setFields(const QStringList &dataFields)
{
    m_fieldNames = dataFields;
}

void LinkLoodsmanModel::setFieldIcon(QString column)
{
   m_fieldIcon  = column;
}

int LinkLoodsmanModel::fieldIndex(const QString &fieldName) const
{
    for (int i = 0;i<m_fieldNames.count();i++)
        if (m_fieldNames.at(i)==fieldName)
            return i;

    return -1;
}
