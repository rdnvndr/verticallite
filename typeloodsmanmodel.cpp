#include "typeloodsmanmodel.h"
#include "loodsmansystem.h"
#include "converttype.h"
#include <QMessageBox>
#include <QIcon>
#include <QDebug>

TypeLoodsmanModel::TypeLoodsmanModel(TreeItem *parent) :
    QAbstractItemModel(parent)
{
    dataSet = new MidasData();
    m_root =  new TreeNodeType;
    m_root->id = 0;
    m_root->isFetch = false;
    cache.insert(0,m_root);
    m_fieldIcon = -1;

    QStringList fields;
    fields  << "_ID_TYPE" << "_TYPENAME" <<  "_IS_ABSTRACT"
            << "_DOCUMENT" << "_ID_PARENT_TYPE";
    setFields(fields);
    setFieldIcon("_ICON");
}

TypeLoodsmanModel::~TypeLoodsmanModel()
{
    delete dataSet;
}

int TypeLoodsmanModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_fieldNames.count();
}

TreeNodeType *TypeLoodsmanModel::getItem(const QModelIndex & index) const
{
    if (index.isValid())
        return static_cast<TreeNodeType*>(index.internalPointer());
    else
        return m_root;
}

QVariant TypeLoodsmanModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole && index.column()==0 && !m_fieldIcon.isEmpty()){
        QPixmap pixmap;
        const TreeNodeType *item = getItem(index);
        if (item) {
            pixmap.loadFromData(item->data.at(m_fieldNames.count()).toByteArray());
            return QIcon(pixmap);
        }
    }

    if(role == Qt::DisplayRole){
        const TreeNodeType *item = getItem(index);
        if (item)
            return item->data.at(index.column());
    }
    return QVariant();
}

QVariant TypeLoodsmanModel::data(long id, int column) const
{
    if(cache.contains(id)){
        TreeNodeType *item = cache[id];
        if (item)
            return item->data.at(column);
    }
    return QVariant();
}

bool TypeLoodsmanModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(index);
    if (role != Qt::EditRole)
        return false;
    return 0;
}

Qt::ItemFlags TypeLoodsmanModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    /*if ( d->isEditable( index.internalId(), index.column() ) )
        fl |= Qt::ItemIsEditable;*/
    return fl;
}

QVariant TypeLoodsmanModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return m_fieldNames.at(section);
    }
    return QVariant();
}

bool TypeLoodsmanModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(section);

    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return false;
}

QModelIndex TypeLoodsmanModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    const TreeNodeType* parentItem = getItem(parent);

    if (!parentItem)
        return QModelIndex();

    TreeNodeType* childItem = cache[parentItem->children.at(row)];

    if (childItem)
            return createIndex(row, column, (void*)childItem);
    else
            return QModelIndex();
}

QModelIndex TypeLoodsmanModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    const TreeNodeType *childItem = getItem(index);
    TreeNodeType *parentItem = cache[childItem->parent];

    if (!parentItem || parentItem == m_root)
        return QModelIndex();

    TreeNodeType *grandparent = cache[parentItem->parent];
    if (!grandparent)
        return QModelIndex();

    for (int i = 0; i < grandparent->children.count(); i++)
        if (grandparent->children.at(i) == parentItem->id)
            return createIndex(i, 0, (void*) parentItem);

    return QModelIndex();
}

int TypeLoodsmanModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0)
        return 0;

    return getItem(parent)->children.count();
}

bool TypeLoodsmanModel::hasChildren(const QModelIndex & parent) const
{
    TreeNodeType* node = getItem(parent);
    if (node->isFetch && node->children.count()==0)
        return  false;

    return true;
}

bool TypeLoodsmanModel::canFetchMore(const QModelIndex &parent) const
{
    return !getItem(parent)->isFetch;
}

void TypeLoodsmanModel::fetchMore(const QModelIndex &parent)
{
    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;

    TreeNodeType *item;

    if(parent.isValid()){
        item = getItem(parent);
        if(!item)
            return;
    }

    _variant_t data = loodsman->main->GetInfoAboutCurrentBase(61,&inErrorCode, &stErrorMessage);
    dataSet->setData((unsigned char *)data.parray->pvData);

    if (inErrorCode.lVal!=0)
        QMessageBox::warning(NULL, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));

    if (dataSet->first()){
        do {
            int childId = dataSet->fieldValue("_ID_TYPE").toInt();
            long parentId = dataSet->fieldValue("_ID_PARENT_TYPE").toInt();

            TreeNodeType *item = cache[parentId];
            if (item==NULL){
                if (parentId == 0)
                    item = m_root;
                else {
                    item = new TreeNodeType;
                    item->id = parentId;
                    cache.insert(parentId, item);
                    item->isFetch = true;
                }
            }

            TreeNodeType* childItem;
            if (!cache.contains(childId)){
                childItem = new TreeNodeType;
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

QList<long> TypeLoodsmanModel::listAttr(long typeId){

    if (m_attr.contains(typeId))
        return m_attr.value(typeId);

    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;
    QString objType = data(typeId,fieldIndex("_TYPENAME")).toString();

    _variant_t testData = loodsman->main->GetInfoAboutType(to_bstr_t(objType),
                                                           1,
                                                           &inErrorCode,
                                                           &stErrorMessage);

    if (inErrorCode.lVal!=0)
        QMessageBox::warning(NULL, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));

    unsigned char *p = (unsigned char *)testData.parray->pvData;
    MidasData* mData = new MidasData();
    mData->setData(p);
    QList<long> attrMap;
    if (mData->first()) do
        attrMap.append(mData->fieldValue("_ID").toInt());
    while (mData->next());

    m_attr[typeId] = attrMap;

    return attrMap;
}

QList<long> TypeLoodsmanModel::listAttrLink(long masterTypeId, long slaveTypeId, QString linkName)
{
    AttrLinkKey key;
    key.id = masterTypeId;
    key.pid = slaveTypeId;
    key.lid = linkName;

    if (m_attrLink.contains(key))
        return m_attrLink.value(key);

    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;
    QString objType = data(masterTypeId,fieldIndex("_TYPENAME")).toString();
    QString parentObjType = data(slaveTypeId,fieldIndex("_TYPENAME")).toString();;
    QString objLink = linkName;

    _variant_t testData = loodsman->main->GetLinkAttrForTypes(to_bstr_t(parentObjType),
                                                              to_bstr_t(objType),
                                                              to_bstr_t(objLink),
                                                              &inErrorCode,
                                                              &stErrorMessage);

    unsigned char *p = (unsigned char *)testData.parray->pvData;
    MidasData* mData = new MidasData();
    mData->setData(p);

    QList<long> attrMap;
    if (mData->first()) do
        attrMap.append(mData->fieldValue("_ID").toInt());
    while (mData->next());


    m_attrLink[key] = attrMap;

    return attrMap;
}

QStringList TypeLoodsmanModel::fields()
{
    return m_fieldNames;
}

void TypeLoodsmanModel::setFields(const QStringList &dataFields)
{
    m_fieldNames = dataFields;
}

void TypeLoodsmanModel::setFieldIcon(QString column)
{
    m_fieldIcon  = column;
}

int TypeLoodsmanModel::fieldIndex(const QString &fieldName) const
{
    for (int i = 0;i<m_fieldNames.count();i++)
        if (m_fieldNames.at(i)==fieldName)
            return i;

    return -1;
}

/*TreeNodeType *TypeLoodsmanModel::node(long id) const
{
    return cache[id];
}*/
