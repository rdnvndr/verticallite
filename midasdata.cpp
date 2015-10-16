#include "midasdata.h"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

MidasData::MidasData(QObject *parent) :
    QObject(parent)
{
}

bool MidasData::first()
{
    if (recordCount()>0){
        m_current = 0;
        return true;
    }
    return false;

}

void MidasData::last()
{
    m_current = recordCount()-1;
}

quint16 MidasData::fieldCount()
{

    return m_field.count();
}

qint32 MidasData::recordCount()
{
    return m_record.count();
}


MidasField MidasData::field(long index){

    return m_field.at(index);
}

QString MidasData::fieldName(long index)
{   
    return m_field.at(index).fieldName;
}

void MidasData::setData(unsigned char *data)
{
    m_field.clear();
    fillField(data);
    m_record.clear();
    fillRecord(data);
    first();
}

unsigned char *MidasData::data()
{
    return NULL;
}

unsigned char MidasData::getStsBit(unsigned char* stsBit, long index){

    int idxChar = (int)(index/4);
    int shiftBit = index - idxChar * 4;
    unsigned char status = stsBit[idxChar];
    status = (status >> shiftBit*2) & 3;
    return status;
}

int MidasData::indexOf(QString name){
    for (int i=0;i<m_field.count();i++)
        if (m_field.at(i).fieldName==name){

            return i;
        }
    return -1;
}

QVariant MidasData::fieldValue(long index)
{
    return m_record.at(m_current).at(index);
}

bool MidasData::next()
{
    if (m_current+1 < recordCount()){
        m_current++;
        return true;
    }
    return false;

}

bool MidasData::prior()
{
    if (m_current-1 >= 0){
        m_current--;
        return true;
    }
    return false;
}

void MidasData::clear()
{
    m_field.clear();
    m_record.clear();
}

void MidasData::fillField(unsigned char *data)
{
    MidasField retField;
    long offsetField = 24;

    quint16 fieldCount;
    memcpy(&fieldCount,(data+12),sizeof(fieldCount));

    for (long i=0;i<fieldCount;i++){
        quint8 lenField;
        memcpy(&lenField,(data+offsetField),sizeof(lenField));

        char* fieldName = new char[lenField+1];
        memcpy(fieldName,(data+offsetField+1),lenField);
        fieldName[lenField] = '\0';
        retField.fieldName = QString::fromLocal8Bit(fieldName);

        qint16 fieldSize;
        memcpy(&fieldSize,(data+offsetField+lenField+1),sizeof(fieldSize));
        retField.fieldSize = fieldSize;

        qint16 fieldType;
        memcpy(&fieldType,(data+offsetField+lenField+3),sizeof(fieldType));
        retField.fieldType = fieldType;

        quint16 fieldPropCount;
        memcpy(&fieldPropCount,(data+offsetField+7+lenField),sizeof(fieldPropCount));
        retField.propCount = fieldPropCount;

        m_field.append(retField);

        // Чтение свойств поля

        long offsetProp = offsetField + lenField + 9;

        for (int j=0;j<fieldPropCount;j++){
            quint8 lenPropName;
            memcpy(&lenPropName,(data+offsetProp),sizeof(lenPropName));

            char* propName = new char[lenPropName+1];
            memcpy(propName,(data+offsetProp+1),lenPropName);
            propName[lenPropName] = '\0';

            qint16 propSize;
            memcpy(&propSize,(data+offsetProp+lenPropName+1),sizeof(propSize));

            qint16 propType;
            memcpy(&propType,(data+offsetProp+lenPropName+3),sizeof(propType));

            long lenData;
            if (propType&MaskVaryingFld){
                //  VarFieldData
                if (propSize==1){
                    qint8 len;
                    memcpy(&len,(data+offsetProp+lenPropName+5),sizeof(len));
                    lenData = len + propSize;
                } else {
                    qint16 len;
                    memcpy(&len,(data+offsetProp+lenPropName+5),sizeof(len));
                    lenData = len + propSize;
                }
            } else {
                lenData = propSize;
            }
            offsetProp = offsetProp + lenPropName + lenData+5;
        }
        offsetField = offsetProp;
        // Окончание чтения свойств поля
    }
}

void MidasData::fillRecord(unsigned char *data)
{
    quint16 headerSize;
    memcpy(&headerSize,(data+22),sizeof(headerSize));
    long offsetValue = headerSize;

    qint32 recordCount;
    memcpy(&recordCount,(data+14),sizeof(recordCount));

    for (int r=0;r<recordCount;r++){

        MidasRecord record;

        quint8 recordStatus;
        memcpy(&recordStatus,(data+offsetValue),sizeof(recordStatus));

        int lenStsBit = (int)(((fieldCount()-1)/4)+1);
        unsigned char* stsBit = new unsigned char[lenStsBit];
        memcpy(stsBit,(data+offsetValue+1),lenStsBit);

        offsetValue = offsetValue + lenStsBit+1;
        for (int i=0;i<fieldCount();i++){

            MidasField fieldIndex = field(i);
            qint16 typeField;
            long lenData;
            if (!(getStsBit(stsBit,i)&(bitNotAll))){
                if (fieldIndex.fieldType&MaskVaryingFld){
                    //  VarFieldData

                    if (fieldIndex.fieldSize==1){
                        qint8 len;
                        memcpy(&len,(data+offsetValue),sizeof(len));
                        lenData = len;
                    } else {
                        qint16 len;
                        memcpy(&len,(data+offsetValue),sizeof(len));
                        lenData = len;
                    }

                    offsetValue = offsetValue +fieldIndex.fieldSize;
                    typeField = fieldIndex.fieldType & MaskFieldType;
                } else {
                    lenData = fieldIndex.fieldSize;
                    typeField = fieldIndex.fieldType;
                }


                QVariant var;
                switch(typeField) {
                    case dsfldZSTRING:
                        {
                            char* fieldData;
                            fieldData = new char[lenData+1];
                            memcpy(fieldData,(data+offsetValue),lenData);
                            fieldData[lenData] = '\0';
                            var = QVariant(QString::fromLocal8Bit(fieldData));
                            delete fieldData;
                        }
                        break;
                    case dsfldINT:
                        if (fieldIndex.fieldSize==1){
                            qint8 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==2){
                            qint16 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==4){
                            qint32 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==8){
                            qint64 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else
                            QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип Int размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldUINT:
                        if (fieldIndex.fieldSize==1){
                            quint8 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==2){
                            quint16 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==4){
                            quint32 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==8){
                            quint64 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else
                            QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип UInt размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldBOOL:
                        {
                            bool fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        }
                        break;
                    case dsfldFLOATIEEE:
                        if (fieldIndex.fieldSize==4){
                            float fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else if (fieldIndex.fieldSize==8){
                            double fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            var = QVariant(fieldData);
                        } else
                            QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип FLOATIEEE размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldBCD:
                        QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип BCD размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldDATE:
                        if (fieldIndex.fieldSize==4){
                            quint32 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            QDate date(1,1,2);
                            date =date.addDays(fieldData);
                            var = QVariant(date);
                        } else
                            QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип Date размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldTIME:
                        if (fieldIndex.fieldSize==4){
                            quint32 fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            QTime time;
                            time = time.addMSecs(fieldData);
                            var = QVariant(time);
                        }else
                            QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип Time размером %1 не обработан").arg(lenData));

                        break;
                    case dsfldTIMESTAMP:
                        if (fieldIndex.fieldSize==8){
                            double fieldData;
                            memcpy(&fieldData,(data+offsetValue),lenData);
                            QDateTime timestamp;
                            timestamp.setDate(QDate(1,1,2));
                            timestamp = timestamp.addMSecs(fieldData);
                            var = QVariant(timestamp);
                        }else
                            QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип TimeShtamp размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldUNICODE:
                        {
                            wchar_t* fieldData;
                            fieldData = new wchar_t[(int)(lenData/2)];
                            memcpy(fieldData,(data+offsetValue),((int)(lenData/2))*2);
                            var = QVariant(QString::fromWCharArray(fieldData));
                            delete fieldData;
                        }
                        break;
                    case dsfldBYTES:
                        {
                            char* fieldData;
                            fieldData = new char[lenData];
                            memcpy(fieldData,(data+offsetValue),lenData);
                            var = QVariant(QByteArray(fieldData,lenData));
                            delete fieldData;
                        }
                        //QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип BYTES размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldADT:
                        QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип AbstractDataType размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldARRAY:
                        QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип ArrayType размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldEMBEDDEDTBL:
                        QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип Embedded (nested table type) размером %1 не обработан").arg(lenData));
                        break;
                    case dsfldREF:
                        QMessageBox::warning(NULL,tr("Предупреждение"),tr("Тип Reference размером %1 не обработан").arg(lenData));
                        break;
                    default:
                        QMessageBox::warning(NULL,tr("Предупреждение"),tr("Неизвестный тип %1 размером %2 не обработан").arg(typeField).arg(lenData));
                }
                record.append(var);
                offsetValue = offsetValue + lenData;
            } else
                record.append(QVariant());
        }
        m_record.append(record);
    }
}

long MidasData::currentRecord()
{
    return m_current;
}

bool MidasData::setCurrentRecord(long value)
{
    if (value>=0 && value < recordCount()){
        m_record.at(value);
        m_record.first();
        return true;
    }
    return false;

}

QVariant MidasData::fieldValue(QString name)
{
    int index = indexOf(name);

    if (index!=-1)
        return fieldValue(index);
    return QVariant();
}
