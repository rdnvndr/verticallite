#include "attrloodsmanmodel.h"

#include "loodsmansystem.h"
#include "converttype.h"
#include <QMessageBox>
#include <QIcon>
#include <QString>
#include <QDebug>

AttrLoodsmanModel::AttrLoodsmanModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    dataSet = new MidasData();
    m_root =  new TreeNode;
    m_root->id = 0;
    m_root->isFetch = false;
    cache.insert(0,m_root);

    QStringList fields;
    fields << "_NAME" << "_ID" << "_ATTRTYPE" << "_DEFAULT" << "_LIST" << "_SYSTEM"
    << "_ONLYLISTITEMS" << "_SYSTEM_1" << "_ACCESSLEVEL" << "_ID_NATURE";
    setFields(fields);
}

AttrLoodsmanModel::~AttrLoodsmanModel()
{
    delete dataSet;
}

int AttrLoodsmanModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_fieldNames.count();
}

TreeNode *AttrLoodsmanModel::getItem(const QModelIndex & index) const
{
    if (index.isValid())
        return static_cast<TreeNode*>(index.internalPointer());
    else
        return m_root;
}

QVariant AttrLoodsmanModel::data(const QModelIndex &index, int role) const
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

QVariant AttrLoodsmanModel::data(long id, int column) const
{
    if(cache.contains(id)){
        TreeNode *item = cache[id];
        if (item)
            return item->data.at(column);
    }
    return QVariant();
}

bool AttrLoodsmanModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(index);
    if (role != Qt::EditRole)
        return false;
    return 0;
}

Qt::ItemFlags AttrLoodsmanModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    /*if ( d->isEditable( index.internalId(), index.column() ) )
        fl |= Qt::ItemIsEditable;*/
    return fl;
}

QVariant AttrLoodsmanModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_fieldNames.at(section);
    }
    return QVariant();
}

bool AttrLoodsmanModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(section);

    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return false;
}

QModelIndex AttrLoodsmanModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex AttrLoodsmanModel::parent(const QModelIndex &index) const
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

int AttrLoodsmanModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0)
        return 0;

    return getItem(parent)->children.count();
}

bool AttrLoodsmanModel::hasChildren(const QModelIndex & parent) const
{
    TreeNode* node = getItem(parent);
    if (node->isFetch && node->children.count()==0)
        return  false;

    return true;
}

bool AttrLoodsmanModel::canFetchMore(const QModelIndex &parent) const
{
    return !getItem(parent)->isFetch;
}

void AttrLoodsmanModel::fetchMore(const QModelIndex &parent)
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

    _variant_t data = loodsman->main->GetInfoAboutCurrentBase(8,&inErrorCode, &stErrorMessage);
    dataSet->setData((unsigned char *)data.parray->pvData);

    if (inErrorCode.lVal!=0)
        QMessageBox::warning(NULL, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));

    if (dataSet->first())
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
            if (m_fieldNames[j] == "_LIST")
                childItem->data.append(
                            QString::fromLocal8Bit(
                                dataSet->fieldValue(
                                    m_fieldNames[j]).toByteArray().data()));
            else
                childItem->data.append(dataSet->fieldValue(m_fieldNames[j]));

        if (!m_fieldIcon.isEmpty())
            childItem->data.append(dataSet->fieldValue(m_fieldIcon));


    } while (dataSet->next());
    dataSet->clear();

    m_root->isFetch = true;
    beginInsertRows(QModelIndex(), 0,cache[0]->children.count());
    endInsertRows();
}

QStringList AttrLoodsmanModel::fields()
{
    return m_fieldNames;
}

void AttrLoodsmanModel::setFields(const QStringList &dataFields)
{
    m_fieldNames = dataFields;
}

void AttrLoodsmanModel::setFieldIcon(QString column)
{
   m_fieldIcon  = column;
}

int AttrLoodsmanModel::fieldIndex(const QString &fieldName) const
{
    for (int i = 0;i<m_fieldNames.count();i++)
        if (m_fieldNames.at(i)==fieldName)
            return i;

    return -1;
}

QList<UnitsAttr> AttrLoodsmanModel::listUnits(const QString& measure)
{
    if (m_units.contains(measure))
        return m_units.value(measure);

    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;

    _variant_t testData = loodsman->main->GetMUnitList(to_bstr_t(measure),
                                                           &inErrorCode,
                                                           &stErrorMessage);

    if (inErrorCode.lVal!=0)
        QMessageBox::warning(NULL, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));

    unsigned char *p = (unsigned char *)testData.parray->pvData;
    MidasData* mData = new MidasData();
    mData->setData(p);
    QList<UnitsAttr> attrUnit;
    if (mData->first()) do {
        UnitsAttr unit;
        unit.id = mData->fieldValue("_ID_UNIT").toString();
        unit.name = mData->fieldValue("_NAME").toString();
        unit.baseUnits = mData->fieldValue("_BASICUNIT").toBool();
        attrUnit.append(unit);
    }while (mData->next());

    m_units[measure] = attrUnit;

    return attrUnit;
}
