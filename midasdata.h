#ifndef MIDASDATA_H
#define MIDASDATA_H

#include <QObject>
#include <QVariant>
#include <QtGlobal>
#include <QVector>

enum MidasStsBit{
    bitNull = 1,        //! Поле пустое
    bitNotChange = 2,   //! Поле не изменено
    bitNotAll = 3       //! Поле пустое и не изменено
};

enum MidasRecordStatus{
    dsRecUnmodified = 0x0000,//! { Unmodified record }
    dsRecOrg = 0x0001,       //! { Original record (was changed) }
    dsRecDeleted = 0x0002,   //! { Record was deleted }
    dsRecNew = 0x0004,       //! { Record was inserted }
    dsRecModified = 0x0008,  //! { Record was changed }
    dsUnused = 0x0020,       //! { Record not used anymore (hole) }
    dsDetUpd = 0x0040        //! { Detail modification Ins/Del/Mod. }{ Can be combined with other status. }
};

enum MidasAttrs{
    fldAttrHIDDEN =   0x0001, //! { Field is hidden }
    fldAttrREADONLY = 0x0002, //! { Field is readonly }
    fldAttrREQUIRED = 0x0004, //! { Field value required }
    fldAttrLINK =     0x0008  //! { Linking field }
};

enum MidasFieldType{
    dsfldUNKNOWN = 0,      //! { Unknown }
    dsfldINT = 1,          //! { signed integer }
    dsfldUINT = 2,         //! { Unsigned integer }
    dsfldBOOL = 3,         //! { Boolean }
    dsfldFLOATIEEE = 4,    //! { IEEE float }
    dsfldBCD = 5,          //! { BCD }
    dsfldDATE = 6,         //! { Date (32 bit) }
    dsfldTIME = 7,         //! { Time (32 bit) }
    dsfldTIMESTAMP = 8,    //! { Time-stamp (64 bit) }
    dsfldZSTRING = 9,      //! { Multi-byte string }
    dsfldUNICODE = 10,     //! { unicode string }
    dsfldBYTES = 11,       //! { bytes }
    dsfldADT = 12,         //! { ADT (Abstract Data Type) }
    dsfldARRAY = 13,       //! { Array type (not attribute) }
    dsfldEMBEDDEDTBL = 14, //! { Embedded (nested table type) }
    dsfldREF = 15          //! { Reference }
};

enum MidasMaskField {
    MaskFieldType = 0x3F,  //! { mask to retrieve Field Type  }
    MaskVaryingFld = 0x40, //! { Varying attribute type. }
    MaskArrayFld = 0x80    //! { Array attribute type. } - Данные массива предваряются длиной Len: Int32, за которой следуют его элементы  Data[Len].
};

/* Пример структуры пакета MIDAS/DATASNAP

struct MidasProp{
    quint32 CHANGE_LOG[]; // UInt32[] - массив из целых чисел.
    quint32 DATASET_DELTA;// UInt32 = 1, если в пакете передается delta-набор (набор изменений).
};

struct MidasRecord{
    quint8 recordStatus;                 // Статус записи: MidasRecordStatus
    unsigned char statusBit[fieldCount/4]; // На каждое поле в массиве статусных битов отводится 2 бита от младшего к старшему. Данные полей содержащих Null или  NotChanged в запись не попадают.
                                         // BLANK_NULL = 1; { 'real' NULL }
                                         // BLANK_NOTCHANGED = 2; { Not changed , compared to original value }
    unsigned char fieldData[];           // FieldData[] - пишутся только not null & changed значения.
                                         // Для полей фиксированного размера: FixedFieldData = Byte[FieldSize]
                                         // Для полей переменного размера: VarFieldData = (DataLen: UInt(of FieldSize)) Byte[DataLen] - те обычно данные предваряются 1-2 байтами размера.
};

struct MidasFieldProp{
    quint8 len;                   // Длина имени свойства
    char   propName[len];         // Имя свойства
    qint16 propSize;              // Для строк - длина фиксированной части, те размера
    qint16 propType;              // Тип свойства: MidasFieldType
    unsigned char data[propSize]; // Данные свойства
};

struct MidasField{
    quint8 len;                      // Длина имени поля
    char   fieldName[len];           // Имя поля
    qint16 fieldSize;                // Для строк - длина фиксированной части, те размера
    qint16 fieldType;                // Тип Поля: MidasFieldType
    qint16 fieldAttrs;               // Атрибуты (флаги) поля: MidasAttrs
    quint16 propCount;               // Количество свойств поля
    MidasFieldProp props[propCount]; // Cвойства поля
                                     // WIDTH: UInt16 - для полей переменного размера, например строк, равно Field.Size. Те это предельный размер данных, хранимых в поле.
};

struct MidasPacket{
    unsigned char signature[4];          // Сигнатура пакета MIDAS/DataSnap [$96, $19, $E0, $BD]
    qint32  skip1;                       // Int32, обычно равно 1
    qint32  skip2;                       // Int32, обычно равно 18
    quint16 fieldCount;                  // UInt16 - количество полей
    qint32  recordCount;                 // Int32 - количество записей в пакете
    qint32  skip3;                       // Int32, обычно равно 3
    quint16 headerSize;                  // UInt16 - вероятно, размер всего заголовка включая описания полей и свойств.
    MidasField fieldList[fieldCount];    // Структура описания полей.
    quint16 propCount;                   // UInt16 - количество свойств
    MidasProp props[propCount];          // Свойства набора
    MidasRecord recordList[recordCount]; // Данные записей (начинается с headerSize)
};
*/

struct MidasField{
    QString fieldName;        //! Имя поля
    qint16  fieldSize;        //! Для строк - длина фиксированной части, те размера
    qint16  fieldType;        //! Тип Поля: MidasFieldType
    qint16  fieldAttrs;       //! Атрибуты (флаги) поля: MidasAttrs
    quint16 propCount;        //! Количество свойств поля
};

typedef  QVector<QVariant> MidasRecord;

class MidasData : public QObject
{
    Q_OBJECT
public:
    explicit MidasData(QObject *parent = 0);

    //! Переход на начало
    bool first();
    //! Переход в конец
    void last();
    //! Переход на следующую
    bool next();
    //! Переход на предыдущую
    bool prior();

    //! Очистка полей и записей
    void clear();

    //! Заполнение полей
    void fillField(unsigned char* data);

    //! Заполнение записей
    /*! Не реализована обработка: dsfldADT,dsfldARRAY, dsfldEMBEDDEDTBL, dsfldREF и MaskArrayFld
    */
    void fillRecord(unsigned char* data);

    //! Текущая запись
    long currentRecord();
    //! Установить текущую запись
    bool setCurrentRecord(long value);

    //! Получение значения поля по его имени
    QVariant fieldValue(QString name);
    //! Получение значения поля по индексу
    QVariant fieldValue(long index);

    //! Получение имени поля по индексу
    QString fieldName(long index);

    //! Чтение данных пакета MIDAS
    void setData(unsigned char* data);
    //! Получение данных пакета MIDAS
    unsigned char* data();

    //! Кол-во полей
    quint16 fieldCount();
    //! Кол-во записей
    qint32  recordCount();

    //! Получение описания поля по индексу
    MidasField field(long index);

    //! Получение индекса поля по его имени
    int indexOf(QString name);
private:
    //! Получение статусных битов значения записи
    unsigned char getStsBit(unsigned char *stsBit, long index);
    //! Список полей
    QVector<MidasField> m_field;
    //! Список записей
    QVector<MidasRecord> m_record;
    //! Текущая запись
    long m_current;

signals:
    
public slots:
    
};

#endif // MIDASDATA_H
