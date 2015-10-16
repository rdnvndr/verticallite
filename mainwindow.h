#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "typeloodsmanmodel.h"
#include "linkloodsmanmodel.h"
#include "statloodsmanmodel.h"
#include "attrloodsmanmodel.h"
#include "treeloodsmanmodel.h"
#include "attreditloodsmanmodel.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    bool onActionConnect();

private slots:
    void listProject();
    void testModel();
    void testType();
    void testLink();
    void testStat();
    void testAttr();
    void editAttr(QModelIndex index);
private:
    TreeLoodsmanModel* ldsmnModel;
    TypeLoodsmanModel* ldsmnTypeModel;
    LinkLoodsmanModel* ldsmnLinkModel;
    AttrLoodsmanModel* ldsmnAttrModel;
    AttrEditLoodsmanModel* ldsmnEditAttrModel;
};

#endif // MAINWINDOW_H
