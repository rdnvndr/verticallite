#ifndef TREELOODSMANMODEL_H
#define TREELOODSMANMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include "treeitem.h"
#include "loodsmansystem.h"
#include "midasdata.h"

#include "typeloodsmanmodel.h"
#include "linkloodsmanmodel.h"
#include "statloodsmanmodel.h"
#include "attrloodsmanmodel.h"

typedef QMap<QString,  QVariantList> AttrMap;

class TreeLoodsmanModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeLoodsmanModel(TreeItem *parent = 0);
    ~TreeLoodsmanModel();

    //! Возращает хранимые данные
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    //! Устанавливает значение для указанной записи
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    //! Возвращает флаг записи
    Qt::ItemFlags flags(const QModelIndex &index) const;

    //! Возращает название заголовка
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole)  const;

    //! Устанавливает название заголовка
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    //! Возращает индекс модели для строки и колонки
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    //! Возращает индекс родителя
    QModelIndex parent(const QModelIndex &index) const ;

    //! Возращает количество строк в индексе родителя
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    //! Возращает количество столбцов в индексе родителя
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
/*
    //! Удаление строки
    bool removeRows(int position, int rows, const QModelIndex &parent);

    //! Вставка строки
    bool insertRows(int position, int rows, const QModelIndex &parent);
*/
    //! Проверка имеются ли дети
    bool hasChildren(const QModelIndex & parent) const ;

    //! Получение TreeItem по индексу
    TreeItem *getItem(const QModelIndex &index) const;

    //! Проверка можно ли провести ленивыю инициализацию
    bool canFetchMore(const QModelIndex &parent) const;

    //! Получение списка полей с данных
    QStringList fields(bool isProject) ;

    //! Ленивая инициализация
    void fetchMore(const QModelIndex &parent);

    //! Получение модели атрибутов для объекта
    QAbstractItemModel *getAttrModel(const QModelIndex &parent);

    //! Получение индекса поля
    long fieldIndex(const QString &fieldName, bool isProject = false);

    void setTypeModel(TypeLoodsmanModel *model);
    void setLinkModel(LinkLoodsmanModel *model);
    void setStatModel(StatLoodsmanModel *model);
    void setAttrModel(AttrLoodsmanModel *model);

    void select();

    //! Ленивое чтение атрибутов объекта
    void fetchMoreAttr(const QModelIndex &parent);

    //! Получение атрибутов
    AttrMap attrMap(const QModelIndex &parent);
private:



    //! Установка списка полей данных
    void setFields(const QStringList& dataFields , bool isProject);

    //! Установка списка полей данных
    void setLinkType(const QStringList& linkType);

    //! Данные узла дерева
    TreeItem* m_root;
    MidasData* dataSet;

    //! Список полей данных
    QStringList m_fieldNames;

    //! Список полей данных
    QStringList m_prjFieldNames;

    //! Список связей
    QString m_linkType;

    //! Значение атрибутов [id_obj,id_attr,список свойств атрибута]
    QMap <long, AttrMap> m_attrVal;

    TypeLoodsmanModel* m_typeModel;
    LinkLoodsmanModel* m_linkModel;
    StatLoodsmanModel* m_statModel;
    AttrLoodsmanModel* m_attrModel;

    mutable QMap<long,  QList<QVariant>* > m_data;
    
signals:
    
public slots:
    
};

#endif // TREELOODSMANMODEL_H
