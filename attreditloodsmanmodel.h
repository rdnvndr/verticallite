#ifndef ATTREDITLOODSMANMODEL_H
#define ATTREDITLOODSMANMODEL_H

#include <QAbstractItemModel>
#include "typeloodsmanmodel.h"
#include "linkloodsmanmodel.h"
#include "statloodsmanmodel.h"
#include "attrloodsmanmodel.h"
#include "treeloodsmanmodel.h"

class AttrEditLoodsmanModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit AttrEditLoodsmanModel(QObject *parent = 0);
    virtual ~AttrEditLoodsmanModel();
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

    //! Получение списка полей с данных
    QStringList fields() ;

    int fieldIndex(const QString &fieldName) const;

    void setSourceModel(TreeLoodsmanModel *sourceModel);
    void setCurrentNode(const QModelIndex &index);

private:

    QList<QVariantList> m_listAttrLink;
    QList<QVariantList> m_listAttrObj;

    //! Установка списка полей данных
    void setFields(const QStringList& dataFields);

    //! Список полей данных
    QStringList m_fieldNames;
    TreeNode* m_root;

    TypeLoodsmanModel* m_typeModel;
    AttrLoodsmanModel* m_attrModel;
    TreeLoodsmanModel *m_treeModel;

    //mutable QHash<long, TreeNode*> cache;

    //! Список названий заголовков
    QMap<int, QString> m_header;

};

#endif // ATTREDITLOODSMANMODEL_H
