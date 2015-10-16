#include "propeditordelegate.h"
#include <QPainter>
#include <QApplication>
#include <QDateEdit>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDebug>


PropEditorDelegate::PropEditorDelegate(QObject *parent) :
    QItemDelegate(parent)
{
    m_colors.reserve(6);
    m_colors.push_back(QColor(255, 230, 191));
    m_colors.push_back(QColor(255, 255, 191));
    m_colors.push_back(QColor(191, 255, 191));
    m_colors.push_back(QColor(199, 255, 255));
    m_colors.push_back(QColor(234, 191, 255));
    m_colors.push_back(QColor(255, 191, 239));
}

QWidget *PropEditorDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{

    long id = index.sibling(index.row(),1/*_ID*/).data().toInt();

    // Единицы измерения
    if (index.column()==9/*_UNIT*/)
        if (!m_attrModel->data(id, m_attrModel->fieldIndex("_ID_NATURE")).toString().isEmpty())
        {
            QComboBox* pRes = new  QComboBox(parent);
            QList<UnitsAttr> listUnits = m_attrModel->listUnits(
                        m_attrModel->data(id, m_attrModel->fieldIndex("_ID_NATURE")).toString());

            QStandardItemModel *model = new QStandardItemModel(listUnits.count(), 2);

            for (int row = 0; row < listUnits.count(); ++row) {
                QStandardItem *item = new QStandardItem(listUnits.at(row).id);
                model->setItem(row, 1, item);
                item = new QStandardItem(listUnits.at(row).name);
                model->setItem(row, 0, item);
            }

            pRes->setModel(model);
            return pRes;
        } else
            return 0;

    // Список значений
    if (index.column()==7/*_VALUE*/ &&
            !m_attrModel->data(id, m_attrModel->fieldIndex("_LIST")).toString().isEmpty())
    {
        QComboBox* pRes = new  QComboBox(parent);
        QStringList list = m_attrModel->data(id, m_attrModel->fieldIndex("_LIST")).toString().split("\r\n");
        QStringListModel *model = new QStringListModel(pRes);
        model->setStringList(list);
        pRes->setModel(model);
        return pRes;
    }

    // Календарь
    if (index.data(Qt::EditRole).type() == QVariant::Date) {
        QDateEdit* pRes = new  QDateEdit(parent);
        pRes->setCalendarPopup(true);
        pRes->setDisplayFormat("dd.MM.yyyy HH:mm");
        return pRes;
    }

    // Логическое
    if (index.data(Qt::EditRole).type() == QVariant::Bool) {
        QCheckBox* pRes = new  QCheckBox(parent);
        return pRes;
    }

    return QItemDelegate::createEditor(parent,option,index);
}

void PropEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    long id = index.sibling(index.row(),1/*_ID*/).data().toInt();

    // Установка значения по умолчанию
    if (!m_attrModel->data(id, m_attrModel->fieldIndex("_DEFAULT")).isNull() &&
            index.column()==7/*_VALUE*/ && index.data().isNull()){
        if (editor->inherits("QLineEdit"))
            (qobject_cast<QLineEdit*>(editor))->setText(index.data().toString());
        else if (editor->inherits("QSpinBox"))
            (qobject_cast<QSpinBox*>(editor))->setValue(index.data().toInt());
        else if (editor->inherits("QDoubleSpinBox"))
            (qobject_cast<QDoubleSpinBox*>(editor))->setValue(index.data().toDouble());
        return;
    }

    // Список значений
    if (index.column()==7/*_VALUE*/ &&
            !m_attrModel->data(id, m_attrModel->fieldIndex("_LIST")).toString().isEmpty())
    {
        QComboBox *pRes = static_cast<QComboBox*>(editor);
        for (int row=0;row<pRes->model()->rowCount();row++)
            if (pRes->model()->index(row,0).data()==index.sibling(index.row(),7/*_ID_UNIT*/).data()){
                pRes->setCurrentIndex(row);
                return;
            }
        pRes->setCurrentIndex(0);
        return;
    }

    // Единицы измерения
    if (index.column()==9/*_UNIT*/){
        QComboBox *pRes = static_cast<QComboBox*>(editor);
        for (int row=0;row<pRes->model()->rowCount();row++)
            if (pRes->model()->index(row,1).data()==index.sibling(index.row(),8/*_ID_UNIT*/).data()){
                pRes->setCurrentIndex(row);
                return;
            }
        pRes->setCurrentIndex(0);
        return;
    }

    // Календарь
    if (index.data(Qt::EditRole).type() == QVariant::Date) {
        QDateEdit *pRes = static_cast<QDateEdit*>(editor);
        pRes->setDate(index.model()->data(index, Qt::EditRole).toDate());
        return;
    }

    // Логическое
    if (index.data(Qt::EditRole).type() == QVariant::Bool) {
        QCheckBox *pRes = static_cast<QCheckBox*>(editor);
        pRes->setChecked(index.model()->data(index, Qt::EditRole).toBool());
        return;
    }

    QItemDelegate::setEditorData(editor,index);
    return;
}

void PropEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    // Единицы измерения
    if (index.column()==9/*_UNIT*/ && editor->inherits("QComboBox")){
        QComboBox *pRes = static_cast<QComboBox*>(editor);
        model->setData(index,pRes->currentText(),Qt::EditRole);
        model->setData(index.sibling(index.row(),8/*_ID_UNIT*/),
                       pRes->model()->index(pRes->currentIndex(),1).data().toString(),
                       Qt::EditRole
                       );
        return;
    }

    // Список значений
    if (editor->inherits("QComboBox")) {
        QComboBox *pRes = static_cast<QComboBox*>(editor);
        model->setData(index,pRes->currentText(),Qt::EditRole);
        return;
    }

    // Календарь
    if (index.data(Qt::EditRole).type() == QVariant::Date) {
        QDateEdit *pRes = static_cast<QDateEdit*>(editor);
        model->setData(index,pRes->date(),Qt::EditRole);
        return;
    }

    // Логическое
    if (index.data(Qt::EditRole).type() == QVariant::Bool) {
        QCheckBox *pRes = static_cast<QCheckBox*>(editor);
        model->setData(index,pRes->isChecked(),Qt::EditRole);
        return;
    }

    QItemDelegate::setModelData(editor,model,index);
    return;
}

void PropEditorDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
    return;
}

void PropEditorDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyleOptionViewItemV3 opt = option;

    bool isHeader = false;
    if (!index.parent().isValid())
        isHeader = true;

    bool isModified = false;

    if (index.column() == 0 && isModified) {
        opt.font.setBold(true);
        opt.fontMetrics = QFontMetrics(opt.font);
    }

    QColor c;
    if (isHeader) {
        c = opt.palette.color(QPalette::Dark);
        opt.palette.setColor(QPalette::Text, opt.palette.color(QPalette::BrightText));
    } else {
        c = QColor(255, 230, 191);
        if (c.isValid() && (opt.features & QStyleOptionViewItemV2::Alternate))
            c = c.lighter(112);
    }
    if (c.isValid())
        painter->fillRect(option.rect, c);
    opt.state &= ~QStyle::State_HasFocus;

    // Логический
    if (index.model()->data(index, Qt::EditRole).type() == QVariant::Bool){
        QSize size = QSize(5,5);//check(option, option.rect, Qt::Checked).size();
        QRect checkboxRect = QStyle::alignedRect(option.direction, (Qt::AlignVCenter|Qt::AlignLeft), size, option.rect);
        drawCheck(painter,option,checkboxRect,Qt::Checked);
    } else
        QItemDelegate::paint(painter, opt, index);

    opt.palette.setCurrentColorGroup(QPalette::Active);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    if (/*!lastColumn(index.column()) &&*/ !isHeader) {
        int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
        painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
    }
    painter->restore();
}

QColor PropEditorDelegate::calcBgColor(const QModelIndex &index) const
{
    QModelIndex i = index;
    while (i.parent().isValid())
        i = i.parent();

    long rowColor;
    if (i.row()<m_colors.count())
        rowColor = i.row();
    else
        rowColor = i.row()%m_colors.count();

    return m_colors.at(rowColor);
}

QSize PropEditorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(option, index) + QSize(3, 4);
}

void PropEditorDelegate::setAttrModel(AttrLoodsmanModel *model)
{
    m_attrModel = model;
}
