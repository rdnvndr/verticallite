#include "attreditloodsmanmodel.h"
#include "converttype.h"
#include <QMessageBox>
#include <QDebug>

#define OBJATTR  (quintptr)0
#define LINKATTR (quintptr)1
#define HEADROW  (quintptr)-1

AttrEditLoodsmanModel::AttrEditLoodsmanModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_fieldNames << tr("_NAME") << tr("_ID") << tr("_ATTRTYPE") <<
             tr("_DEFAULT") << tr("_LIST") <<  tr("_ACCESSLEVEL") <<
             tr("_ISATTRLINK") << tr("_VALUE") << tr("_ID_UNIT") <<
             tr("_UNIT");
}

AttrEditLoodsmanModel::~AttrEditLoodsmanModel()
{

}

int AttrEditLoodsmanModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_fieldNames.count();
}

QVariant AttrEditLoodsmanModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole || role == Qt::EditRole){
        if (index.parent().isValid()){
            QVariantList list;
            if (index.parent().row()==OBJATTR)
                list = m_listAttrObj.at(index.row());
            else
                list = m_listAttrLink.at(index.row());
            return list.at(index.column());
        } else
            if (index.column()==0){
                if (index.row()==OBJATTR)
                    return tr("Атрибуты объекта");
                else
                    return tr("Атрибуты связи");
            }
    }
    return QVariant();
}

bool AttrEditLoodsmanModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(index);
    if (role != Qt::EditRole)
        return false;

    if (index.parent().isValid()){
        QVariantList list;
        if (index.parent().row()==OBJATTR){
            list = m_listAttrObj.at(index.row());
            list.replace(index.column(),value);
            m_listAttrObj.replace(index.row(),list);
        } else {
            list = m_listAttrLink.at(index.row());
            list.replace(index.column(),value);
            m_listAttrLink.replace(index.row(),list);
        }
        return 1;
    }

    return 0;
}

Qt::ItemFlags AttrEditLoodsmanModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    /*if ( d->isEditable( index.internalId(), index.column() ) )*/
    if (index.column()!=0)
        fl |= Qt::ItemIsEditable;
    return fl;
}

QVariant AttrEditLoodsmanModel::headerData(int section, Qt::Orientation orientation, int role) const
{    
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        if (m_header[section].isNull())
            return m_fieldNames.at(section);
        else
            return  m_header[section];

    return  QVariant();
}

bool AttrEditLoodsmanModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(section);

    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    m_header[section] = value.toString();
    emit headerDataChanged(orientation, section, section);

    return true;
}
QModelIndex AttrEditLoodsmanModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    if (parent.isValid() ){
        if (parent.row()==OBJATTR && !parent.parent().isValid())
            return createIndex(row, column, OBJATTR);

        if (parent.row()==LINKATTR && !parent.parent().isValid())
            return createIndex(row, column,LINKATTR);

        return QModelIndex();
    }
    return createIndex(row, column, HEADROW);
}

QModelIndex AttrEditLoodsmanModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    if (index.internalId()==OBJATTR)
        return createIndex(OBJATTR, 0,HEADROW);

    if (index.internalId()==LINKATTR)
        return createIndex(LINKATTR, 0,HEADROW);

    return QModelIndex();
}

int AttrEditLoodsmanModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
            return 0;

    if (parent.isValid()){
        if (!parent.parent().isValid()){
            if (parent.row()==OBJATTR)
                return m_listAttrObj.count();
            else
                return m_listAttrLink.count();
        }
    } return 2;

    return 0;
}

bool AttrEditLoodsmanModel::hasChildren(const QModelIndex & parent) const
{
    if (parent.isValid()){
        if (!parent.parent().isValid())
            return true;
    } else
        return true;

    return false;
}

QStringList AttrEditLoodsmanModel::fields()
{
    return m_fieldNames;
}

void AttrEditLoodsmanModel::setFields(const QStringList &dataFields)
{
    m_fieldNames = dataFields;
}

int AttrEditLoodsmanModel::fieldIndex(const QString &fieldName) const
{
    for (int i = 0;i<m_fieldNames.count();i++)
        if (m_fieldNames.at(i)==fieldName)
            return i;

    return -1;
}

void AttrEditLoodsmanModel::setSourceModel(TreeLoodsmanModel *sourceModel)
{
    m_treeModel = sourceModel;
}

void AttrEditLoodsmanModel::setCurrentNode(const QModelIndex &index)
{
    //m_treeModel->fetchMoreAttr(index);
    if (m_listAttrObj.count()>0){
        beginRemoveRows(this->index(0,0),0,m_listAttrObj.count()-1);
        m_listAttrObj.clear();
        endRemoveRows();
    }

    if (m_listAttrLink.count()>0){
        beginRemoveRows(this->index(1,0),0,m_listAttrLink.count()-1);
        m_listAttrLink.clear();
        endRemoveRows();
    }

    foreach (QVariantList value,  m_treeModel->attrMap(index)){
        QVariantList item;
        for (int i=0;i<10;i++){
            if (i<value.count())
                item << value.at(i);
            else
                if (i==7)
                    switch (value.at(2).toInt()) {
                        case 0:
                            item << QVariant::String;
                            break;
                        case 1:
                            item << QVariant::Int;
                            break;
                        case 2:
                            item << QVariant::Double;
                            break;
                        case 3:
                            item << QVariant::DateTime;
                            break;
                        default:
                            item << QVariant::String;
                    }
                else
                    item << QVariant::String;
        }
        if (value.at(6)==true)
            m_listAttrLink.append(item);
        else
            m_listAttrObj.append(item);
    }

    if (m_listAttrObj.count()>0){
        beginInsertRows(this->index(0,0),0,m_listAttrObj.count()-1);
        endInsertRows();
    }

    if (m_listAttrLink.count()>0){
        beginInsertRows(this->index(1,0),0,m_listAttrLink.count()-1);
        endInsertRows();
    }
}
