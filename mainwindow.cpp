#include "mainwindow.h"
#include "dialogconnect.h"

#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSettings>
#include <QByteArray>
#include <QModelIndex>
#include "converttype.h"
#include "loodsmansystem.h"
#include "treeloodsmanmodel.h"
#include "propeditordelegate.h"
#include "midasdata.h"
#include "attrlinkkey.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    ::CoInitialize(NULL);
    ::CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                           RPC_C_IMP_LEVEL_DELEGATE, NULL, 0, NULL);

    ldsmnTypeModel = new TypeLoodsmanModel();    
    ldsmnLinkModel = new LinkLoodsmanModel();
    ldsmnAttrModel = new AttrLoodsmanModel();
    ldsmnEditAttrModel = new AttrEditLoodsmanModel();

    connect(pshBtnTestProject,SIGNAL(clicked()),this, SLOT(listProject()));
    connect(pshBtnModelTest,SIGNAL(clicked()),this, SLOT(testModel()));
    connect(pshBtnTypeTest,SIGNAL(clicked()),this, SLOT(testType()));
    connect(pshBtnLinkTest,SIGNAL(clicked()),this, SLOT(testLink()));
    connect(pshBtnStatTest,SIGNAL(clicked()),this, SLOT(testStat()));
    connect(pshBtnAttrTest,SIGNAL(clicked()),this, SLOT(testAttr()));
}

MainWindow::~MainWindow()
{
    delete  ldsmnTypeModel;
    delete  ldsmnLinkModel;
    delete  ldsmnAttrModel;
    delete  ldsmnEditAttrModel;
    ::CoUninitialize();
}

bool MainWindow::onActionConnect()
{
    DialogConnect* windowConnect = new DialogConnect(this);
    if (windowConnect->exec() == QDialog::Accepted) {
        delete windowConnect;
        return true;
    }
    delete windowConnect;
    return false;
}

void MainWindow::listProject()
{
    VARIANT inErrorCode;
    VARIANT stErrorMessage;




    LoodsmanSystem* loodsman = LoodsmanSystem::instance();

/* QTime timer;
   timer.start();
    IDataSetPtr dataSet;
qDebug() << "Time: " << timer.elapsed() << "ms";
    dataSet.CreateInstance("Loodsman.DataSet");

    dataSet->DATA = loodsman->main->GetProjectList2(false,&inErrorCode, &stErrorMessage);
    dataSet->First();

    long idVersion;

    for (int i=0;i<dataSet->FieldCount;i++){
        _bstr_t    fieldName  = dataSet->FieldName[i];
        _variant_t fieldValue = dataSet->FieldValue[fieldName];
        if (fieldValue.vt == 8)
            qDebug() << fieldName << ": " << from_bstr_t(fieldValue.bstrVal);
        else if (fieldValue.vt == 3){
            qDebug() << fieldName << ": " << fieldValue.intVal;
            if (i==0)
                idVersion = fieldValue.intVal;
        }else
            qDebug() << fieldName;
    }
    qDebug() << "Count" << dataSet->FieldCount;

*/
    //_variant_t testData = loodsman->main->GetInfoAboutAttribute(to_bstr_t(QString("Масса")),2,&inErrorCode, &stErrorMessage);
    _variant_t testData = loodsman->main->GetMUnitList(to_bstr_t(QString("V6CDB8F46D23C4E5CB16863CC70079F1C")),&inErrorCode, &stErrorMessage);

     //_variant_t testData = loodsman->main->GetInfoAboutCurrentBase(8,&inErrorCode, &stErrorMessage);
/*    testData = loodsman->main->GetLinkAttrForTypes(to_bstr_t(QString("Технология")),
                                                   to_bstr_t(QString("Техоперация")),
                                                   to_bstr_t(QString("Состоит из ...")),
                                                   &inErrorCode,
                                                   &stErrorMessage);
*/
    unsigned char *p = (unsigned char *)testData.parray->pvData;
    MidasData *mData = new MidasData();
    mData->setData(p);

    qDebug() << "2-----------------------";
    if (mData->first())
    do
        for (int i=0;i<mData->fieldCount();i++){
            qDebug() << mData->fieldName(i) << ":" <<  mData->fieldValue(i);
        }
    while (mData->next());

}

void MainWindow::testModel(){

    ldsmnModel = new TreeLoodsmanModel();

    ldsmnModel->setLinkModel(ldsmnLinkModel);
    ldsmnModel->setTypeModel(ldsmnTypeModel);
    ldsmnModel->setAttrModel(ldsmnAttrModel);
    ldsmnLinkModel->fetchMore(QModelIndex());
    ldsmnTypeModel->fetchMore(QModelIndex());
    ldsmnAttrModel->fetchMore(QModelIndex());
    ldsmnModel->select();
    ldsmnEditAttrModel->setSourceModel(ldsmnModel);
    treeViewObjAttr->setModel(ldsmnEditAttrModel);
    PropEditorDelegate *propDelegate = new PropEditorDelegate();
    propDelegate->setAttrModel(ldsmnAttrModel);

    treeViewObjAttr->setItemDelegate(propDelegate);
    treeViewObjAttr->model()->setHeaderData(0, Qt::Horizontal, tr("Наименование"));
    treeViewObjAttr->model()->setHeaderData(7, Qt::Horizontal, tr("Значение"));
    treeViewObjAttr->model()->setHeaderData(9, Qt::Horizontal, tr("ЕИ"));
    treeViewObjAttr->hideColumn(1);
    treeViewObjAttr->hideColumn(2);
    treeViewObjAttr->hideColumn(3);
    treeViewObjAttr->hideColumn(4);
    treeViewObjAttr->hideColumn(5);
    treeViewObjAttr->hideColumn(6);
    treeViewObjAttr->hideColumn(8);
    treeViewObjAttr->hideColumn(10);
    treeViewObjAttr->setColumnWidth(9,40);

    treeView->setModel(ldsmnModel);
    treeView->setExpandsOnDoubleClick(false);
    connect(treeView,SIGNAL(clicked(QModelIndex)),this,SLOT(editAttr(QModelIndex)));
}

void MainWindow::testType()
{
    treeViewType->setModel(ldsmnTypeModel);
    //treeViewType->hideColumn(0);
}

void MainWindow::testLink()
{

    treeViewLink->setModel(ldsmnLinkModel);
}

void MainWindow::testStat()
{
    StatLoodsmanModel* ldsmnStatModel = new StatLoodsmanModel();

    QStringList fields;
    fields << "_ID" << "_NAME";
    ldsmnStatModel->setFields(fields);
    ldsmnStatModel->setFieldIcon("_ICON");
    treeViewStat->setModel(ldsmnStatModel);
}

void MainWindow::testAttr()
{
    treeViewAttr->setModel(ldsmnAttrModel);
}

void MainWindow::editAttr(QModelIndex index)
{
    ldsmnEditAttrModel->setCurrentNode(index);
}
