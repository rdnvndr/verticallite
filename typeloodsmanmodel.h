#ifndef TYPELOODSMANMODEL_H
#define TYPELOODSMANMODEL_H

#include <QAbstractItemModel>
#include "attrlinkkey.h"
#include "treeitem.h"
#include "midasdata.h"
#include "treenode.h"

struct TreeNodeType:public TreeNode
{

};

class TypeLoodsmanModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TypeLoodsmanModel(TreeItem *parent = 0);
    ~TypeLoodsmanModel();

    //! Возращает хранимые данные
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant data(long id, int column) const;

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
    TreeNodeType *getItem(const QModelIndex &index) const;

    //! Проверка можно ли провести ленивыю инициализацию
    bool canFetchMore(const QModelIndex &parent) const;

    //! Ленивая инициализация
    void fetchMore(const QModelIndex &parent);

    //!  Получение списка атрибутов
    QList<long> listAttr(long typeId);
    QList<long> listAttrLink(long masterTypeId, long slaveTypeId, QString linkName);

    //! Получение списка полей с данных
    QStringList fields() ;

    //! Возращает индекс поля или если не найдено поле -1
    int fieldIndex( const QString & fieldName ) const;

private:

    //! Установка списка полей данных
    void setFields(const QStringList& dataFields );

    //! Установить поле с иконкой
    void setFieldIcon(QString column);


    TreeNodeType* m_root;
    MidasData* dataSet;

    //! Список полей данных
    QStringList m_fieldNames;

    //! Поле с Иконкой
    QString m_fieldIcon;

    mutable QHash<long, TreeNodeType*> cache;
    QMap<long, QList<long> > m_attr;
    QMap<AttrLinkKey, QList<long> > m_attrLink;
    
signals:
    
public slots:
    
};

#endif // TYPELOODSMANMODEL_H
