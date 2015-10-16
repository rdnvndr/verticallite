#include "treeloodsmanmodel.h"
#include "loodsmansystem.h"
#include "converttype.h"
#include <QMessageBox>
#include <QDebug>
#include <QStandardItemModel>
#include <QDateTime>

TreeLoodsmanModel::TreeLoodsmanModel(TreeItem *parent) :
    QAbstractItemModel(parent)
{
    m_root = new TreeItem(0);
    dataSet = new MidasData();
}

TreeLoodsmanModel::~TreeLoodsmanModel()
{
    delete dataSet;
}

int TreeLoodsmanModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_fieldNames.count();
}

TreeItem *TreeLoodsmanModel::getItem(const QModelIndex & index) const
{
    if (index.isValid())
        return static_cast<TreeItem*>(index.internalPointer());
    else
        return m_root;
}

QVariant TreeLoodsmanModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    if(role == Qt::DisplayRole){
        TreeItem *item = getItem(index);
        if (item)
            return  m_data[item->id]->at(index.column());
    }
    return QVariant();
}

bool TreeLoodsmanModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(index);
    if (role != Qt::EditRole)
        return false;
    return 0;
}

Qt::ItemFlags TreeLoodsmanModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    /*if ( d->isEditable( index.internalId(), index.column() ) )
        fl |= Qt::ItemIsEditable;*/
    return fl;
}

QVariant TreeLoodsmanModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_fieldNames.at(section);
    }
    return QVariant();
}

bool TreeLoodsmanModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(section);

    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return false;
}

QModelIndex TreeLoodsmanModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    const TreeItem* parentItem = getItem(parent);

    if (!parentItem)
        return QModelIndex();

    TreeItem* childItem = parentItem->childAt(row);

    if (childItem)
            return createIndex(row, column, (void*)childItem);
    else
            return QModelIndex();
}

QModelIndex TreeLoodsmanModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();

    const TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == m_root)
        return QModelIndex();

    TreeItem *grandparent = parentItem->parent();
    if (!grandparent)
        return QModelIndex();

    for (int i = 0; i < grandparent->childCount(); i++)
        if (grandparent->childAt(i) == parentItem)
            return createIndex(i, 0, (void*) parentItem);

    return QModelIndex();
}

int TreeLoodsmanModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0)
        return 0;

    /*if(getItem(parent)->canFetchMore()){
        TreeItem* item = parent.isValid()? static_cast<TreeItem*>(parent.internalPointer()) : m_root;
        if(item)
            item->populateChildren();
    }*/

    return getItem(parent)->childCount();
}

bool TreeLoodsmanModel::hasChildren(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    //bool result = getItem(parent)->hasChildren();

    return getItem(parent)->hasChildren();
}

bool TreeLoodsmanModel::canFetchMore(const QModelIndex &parent) const
{
    return getItem(parent)->canFetchMore();
}

void TreeLoodsmanModel::fetchMore(const QModelIndex &parent)
{
    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;

    TreeItem *item;

    if(parent.isValid()){
        item = getItem(parent);
        if(!item)
            return;

        _variant_t data = loodsman->main->GetTree2(item->id,
                                                   to_bstr_t(m_linkType),
                                                   0,
                                                   &inErrorCode, &stErrorMessage);
        dataSet->setData((unsigned char *)data.parray->pvData);
    }else{
        item = m_root;

        _variant_t data = loodsman->main->GetProjectList2(item->id,&inErrorCode, &stErrorMessage);
        dataSet->setData((unsigned char *)data.parray->pvData);
    }


    if (inErrorCode.lVal!=0)
        QMessageBox::warning(NULL, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));

    for (int i=0;i<dataSet->recordCount();i++){
        if (i==0)
            dataSet->first();
        else
            dataSet->next();

        int childId = dataSet->fieldValue(QString("_ID_VERSION")).toInt();
        TreeItem *childItem = new TreeItem(childId,item);

        QList<QVariant>* listData = new QList<QVariant>;

        // Запись данных
        if (m_data[childId] == NULL){
            if (item->id == 0)
                for ( int j=0; j < m_prjFieldNames.count(); j++ ){
                    listData->append(dataSet->fieldValue(m_prjFieldNames[j]));
                }
            else
                for ( int j=0; j < m_fieldNames.count(); j++ ){
                    listData->append(dataSet->fieldValue(m_fieldNames[j]));
                }
            m_data[childId] = listData;
        }
        item->append(childItem);
    }
    dataSet->clear();
    item->setFectMore(true);

    // Обновление
    if (item->id == 0)
        beginInsertRows(QModelIndex(), 0, item->childCount());
    else
        beginInsertRows(parent, 0, item->childCount());
    endInsertRows();
}

void TreeLoodsmanModel::fetchMoreAttr(const QModelIndex &parent)
{
    TreeItem *item;
    if(parent.isValid()){
        item = getItem(parent);
        if(!item)
            return;
    }

    long objId = item->id;
    if (m_attrVal.contains(objId))
        return;

    LoodsmanSystem* loodsman = LoodsmanSystem::instance();
    VARIANT inErrorCode;
    VARIANT stErrorMessage;

    /* ЗАГРУЖАЕТ ВСЕ АТРИБУТЫ ВЫБРАННОГО ТИПА БЕЗ ЗНАЧЕНИЙ */
    AttrMap attrMap;
    long typeId = data(parent.sibling(parent.row(),fieldIndex("_ID_TYPE"))).toInt();

    if (typeId >0){
        QList<long> listIdAttr = m_typeModel->listAttr(typeId);
        for (int i=0;i<listIdAttr.count();i++){
            long idAttr = listIdAttr.value(i);
            QList<QVariant> listData;
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_NAME")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_ID")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_ATTRTYPE")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_DEFAULT")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_LIST")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_ACCESSLEVEL")));
            // isAttrLink
            listData.append(false);
            attrMap.insert(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_NAME")).toString(),
                           listData);
        }
    }

    long parentTypeId = data(parent.parent().sibling(parent.parent().row(),fieldIndex("_ID_TYPE"))).toInt();
    long linkId =  data(parent.sibling(parent.row(),fieldIndex("_ID_LINKTYPE"))).toInt();

    if (typeId >0 && parentTypeId>0 && linkId>0){
        QString objLink = m_linkModel->data(linkId,m_linkModel->fieldIndex("_NAME")).toString();
        QList<long> listIdAttr = m_typeModel->listAttrLink(typeId,parentTypeId,objLink);

        for (int i=0;i<listIdAttr.count();i++){
            long idAttr = listIdAttr.value(i);
            QList<QVariant> listData;
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_NAME")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_ID")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_ATTRTYPE")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_DEFAULT")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_LIST")));
            listData.append(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_ACCESSLEVEL")));
            // isAttrLink
            listData.append(true);
            attrMap.insert(m_attrModel->data(idAttr,m_attrModel->fieldIndex("_NAME")).toString(),
                           listData);
        }
    }

    /* ЗАГРУЖАЕТ ЗНАЧЕНИЕ АТРИБУТОВ
       0 - строковое значение;
       1 - целочисленное  значение;
       2 - действительное число;
       3 - дата и время;
       5 - текст;
       6 - изображение);

        0 - BSTR
        1 - int
        2 - double
        3 - DateTime
        5 - массив байт (Значение представляет собой одномерный вариантный массив (тип VT_UI1 (VC++), varByte(Delphi)))
        6 - массив байт (Значение представляет собой одномерный вариантный массив (тип VT_UI1 (VC++), varByte(Delphi)))
    */

    long objLinkId = data(parent.sibling(parent.row(),fieldIndex("_ID_LINK"))).toInt();

    _variant_t testData = loodsman->main->GetAttributesValues2(
                to_bstr_t(QString("%1").arg(objId)),
                "",&inErrorCode, &stErrorMessage);

    unsigned char *p = (unsigned char *)testData.parray->pvData;
    MidasData* mData = new MidasData();
    mData->setData(p);
    if (mData->first()){
        do{
            QString name = mData->fieldValue("_NAME").toString();
            QList<QVariant> listData = attrMap.value(name);
            QStringList list;
            QDateTime dt;
            switch (listData.at(2).toInt()) {
                case 0:
                    listData.append(mData->fieldValue("_VALUE").toString());
                    break;
                case 1:
                    listData.append(mData->fieldValue("_VALUE").toInt());
                    break;
                case 2:
                    listData.append(mData->fieldValue("_VALUE").toDouble());
                    break;
                case 3:
                    list = mData->fieldValue("_VALUE").toString().split(QRegExp("\\s+"));
                    if (list.count()>0)
                        dt.setDate(QDate::fromString(list.at(0),"dd.MM.yyyy"));
                    if (list.count()>1)
                        dt.setTime(QTime::fromString(list.at(1),"HH:mm"));
                    listData.append(dt);
                    break;
                default:
                    listData.append(mData->fieldValue("_VALUE"));
            }
            listData.append(mData->fieldValue("_ID_UNIT"));
            listData.append(mData->fieldValue("_UNIT"));
            attrMap[name] = listData;
        }while (mData->next());
    }

    if (objLinkId > 0)
    {
        testData = loodsman->main->GetLinkAttributes(
                    objLinkId,
                    &inErrorCode, &stErrorMessage);

        p = (unsigned char *)testData.parray->pvData;
        mData->clear();
        mData->setData(p);
        if (mData->first()){
            do{
                QString name = mData->fieldValue("_NAME").toString();
                QList<QVariant> listData = attrMap.value(name);
                QStringList list;
                QDateTime dt;
                switch (listData.at(2).toInt()) {
                    case 0:
                        listData.append(mData->fieldValue("_VALUE").toString());
                        break;
                    case 1:
                        listData.append(mData->fieldValue("_VALUE").toInt());
                        break;
                    case 2:
                        listData.append(mData->fieldValue("_VALUE").toDouble());
                        break;
                    case 3:
                        list = mData->fieldValue("_VALUE").toString().split(QRegExp("\\s+"));
                        if (list.count()>0)
                            dt.setDate(QDate::fromString(list.at(0),"dd.MM.yyyy"));
                        if (list.count()>1)
                            dt.setTime(QTime::fromString(list.at(1),"HH:mm"));
                        listData.append(dt);
                        break;
                    default:
                        listData.append(mData->fieldValue("_VALUE"));
                }
                listData.append(mData->fieldValue("_ID_UNIT"));
                listData.append(mData->fieldValue("_UNIT"));
                attrMap[name] = listData;
            }while (mData->next());
        }
    }
    m_attrVal.insert(objId,attrMap);
}

QStringList TreeLoodsmanModel::fields(bool isProject)
{
    return (isProject)?m_prjFieldNames:m_fieldNames;
}

void TreeLoodsmanModel::setFields(const QStringList &dataFields, bool isProject)
{
    if (isProject)
       m_prjFieldNames = dataFields;
    else
        m_fieldNames = dataFields;
}

void TreeLoodsmanModel::setLinkType(const QStringList &linkType)
{
    foreach(QString linkName,linkType){
        if (m_linkType.isEmpty())
            m_linkType = linkName;
        else
            m_linkType = m_linkType + QString(char(1)) + linkName;
    }
}

QAbstractItemModel* TreeLoodsmanModel::getAttrModel(const QModelIndex &parent)
{
    fetchMoreAttr(parent);
    TreeItem *item;
    if(parent.isValid()){
        item = getItem(parent);
        if(!item)
            return 0;
    }

    QStandardItemModel* attrModel = new QStandardItemModel(0,10);
    attrModel->setHeaderData(0, Qt::Horizontal, tr("_NAME"));
    attrModel->setHeaderData(1, Qt::Horizontal, tr("_ID"));
    attrModel->setHeaderData(2, Qt::Horizontal, tr("_ATTRTYPE"));
    attrModel->setHeaderData(3, Qt::Horizontal, tr("_DEFAULT"));
    attrModel->setHeaderData(4, Qt::Horizontal, tr("_LIST"));
    attrModel->setHeaderData(5, Qt::Horizontal, tr("_ACCESSLEVEL"));
    attrModel->setHeaderData(6, Qt::Horizontal, tr("_ISATTRLINK"));
    attrModel->setHeaderData(7, Qt::Horizontal, tr("_VALUE"));
    attrModel->setHeaderData(8, Qt::Horizontal, tr("_ID_UNIT"));
    attrModel->setHeaderData(9, Qt::Horizontal, tr("_UNIT"));

    QStandardItem *parentItem = attrModel->invisibleRootItem();
    QStandardItem *oitem = new QStandardItem(tr("Атрибуты объекта"));
    parentItem->appendRow(oitem);
    QStandardItem *litem = new QStandardItem(tr("Атрибуты связи"));
    parentItem->appendRow(litem);

    long objId = item->id;
    foreach (QList<QVariant> value, m_attrVal.value(objId)){
        QList<QStandardItem *> listoitem;
        for (int i=0;i<10;i++){
            QStandardItem *item = new QStandardItem();
            if (i<value.count())
                item->setData(value.at(i), Qt::EditRole);
            else
                if (i==7)
                    switch (value.at(2).toInt()) {
                        case 0:
                            item->setData(QVariant::String, Qt::EditRole);
                            break;
                        case 1:
                            item->setData(QVariant::Int, Qt::EditRole);
                            break;
                        case 2:
                            item->setData(QVariant::Double, Qt::EditRole);
                            break;
                        case 3:
                            item->setData(QVariant::DateTime, Qt::EditRole);
                            break;
                        default:
                            item->setData(QVariant::String, Qt::EditRole);
                    }
                else
                    item->setData(QVariant::String, Qt::EditRole);
            listoitem << item;
        }
        if (value.at(6)==true)
            litem->appendRow(listoitem);
        else
            oitem->appendRow(listoitem);
    }

    attrModel->submit();
    /*QStandardItemModel* model = new QStandardItemModel();
    QStandardItem *parentItem = model->invisibleRootItem();
    for (int i = 0; i < 4; ++i) {
        QStandardItem *item = new QStandardItem(QString("item %0").arg(i));
        parentItem->appendRow(item);
        parentItem = item;
    }
    return model;*/
    return attrModel;
}

AttrMap TreeLoodsmanModel::attrMap(const QModelIndex &parent)
{
    fetchMoreAttr(parent);
    TreeItem *item;
    if(parent.isValid()){
        item = getItem(parent);
        if(!item)
            return AttrMap();
    }

    long objId = item->id;
    return m_attrVal.value(objId);
}

long TreeLoodsmanModel::fieldIndex(const QString &fieldName, bool isProject)
{
    if (isProject){
        for (long i = 0;i<m_prjFieldNames.count();i++)
            if (m_prjFieldNames.at(i)==fieldName)
                return i;
    }else{
        for (long i = 0;i<m_fieldNames.count();i++)
            if (m_fieldNames.at(i)==fieldName)
                return i;
    }
    return -1;
}

void TreeLoodsmanModel::setTypeModel(TypeLoodsmanModel *model)
{
    m_typeModel = model;
}

void TreeLoodsmanModel::setLinkModel(LinkLoodsmanModel *model)
{
    m_linkModel = model;
}

void TreeLoodsmanModel::setStatModel(StatLoodsmanModel *model)
{
    m_statModel = model;
}

void TreeLoodsmanModel::setAttrModel(AttrLoodsmanModel *model)
{
    m_attrModel = model;
}

void TreeLoodsmanModel::select()
{
    QStringList prjFields;
    prjFields << "_ID_VERSION" << "_PRODUCT" << "_ID_TYPE" << "_ID_STATE"
              << "_HAS_LINK" << "_ID_LOCK" << "_VERSION" << "_ACCESSLEVEL"
              << "_REVISION" << "_ID_LINKTYPE";
    setFields(prjFields,true);
    QStringList fields;
    fields << "_ID_VERSION" << "_PRODUCT" << "_ID_TYPE" <<"_ID_STATE"
           << "_HAS_LINK" << "_ID_LOCK" << "_VERSION" << "_ACCESSLEVEL"
           << "_ID_LINK" << "_ID_LINKTYPE";
    setFields(fields,false);

    QStringList linkList;
    m_linkModel->fetchMore(QModelIndex());

    for (int row = 0; row < m_linkModel->rowCount(); row++){
        QModelIndex indexName = m_linkModel->index(row, m_linkModel->fieldIndex("_NAME"));
        QModelIndex indexType = m_linkModel->index(row, m_linkModel->fieldIndex("_TYPE"));
        if (m_linkModel->data(indexType)==0)
            linkList << m_linkModel->data(indexName).toString();
    }
    setLinkType(linkList);
}
