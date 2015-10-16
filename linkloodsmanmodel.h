#ifndef LINKLOODSMANMODEL_H
#define LINKLOODSMANMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>

#include "loodsmansystem.h"
#include "midasdata.h"
#include "treenode.h"

class LinkLoodsmanModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit LinkLoodsmanModel(QObject *parent = 0);
    ~LinkLoodsmanModel();
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
    TreeNode *getItem(const QModelIndex &index) const;

    //! Проверка можно ли провести ленивыю инициализацию
    bool canFetchMore(const QModelIndex &parent) const;

    //! Ленивая инициализация
    void fetchMore(const QModelIndex &parent);

    //! Получение списка полей с данных
    QStringList fields() ;

    int fieldIndex(const QString &fieldName) const;
private:

    //! Установка списка полей данных
    void setFields(const QStringList& dataFields );

    void setFieldIcon(QString column);

    TreeNode* m_root;
    MidasData* dataSet;

    //! Список полей данных
    QStringList m_fieldNames;

    //! Поле с Иконкой
    QString m_fieldIcon;

    mutable QHash<long, TreeNode*> cache;
    
signals:
    
public slots:
    
};

#endif // LINKLOODSMANMODEL_H
