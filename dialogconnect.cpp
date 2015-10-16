/*
 *      mainwindow.cpp
 *
 *      Copyright 2009 Rodionov Andrey <andrey@roand.ru>
 *
 *
 */

#include "converttype.h"
#include "loodsmansystem.h"
#include "dialogconnect.h"
#include <QMessageBox>

DialogConnect::DialogConnect(QWidget* pwgt) : QDialog(pwgt)
{
    setupUi(this);

    textLabelHostname->hide();
    editHostname->hide();
    textLabelBD->hide();
    editDatabase->hide();
    line->hide();

    QSettings settings("NONAME", "VerticalLite");
    settings.beginGroup("DialogConnect");
    editDatabase->setText(settings.value("database").toString());
    editHostname->setText(settings.value("hostname").toString());
    editUsername->setText(settings.value("username").toString());
    settings.endGroup();

    this->adjustSize();
    connect(pushButtonProp, SIGNAL(clicked()), this, SLOT(onClickButtonProp()));
    connect(pushButtonOk, SIGNAL(clicked()), this, SLOT(onClickButtonOk()));
}

void DialogConnect::onClickButtonProp()
{
    if (textLabelHostname->isHidden()) {
        pushButtonProp->setText(tr("Кратко"));
        textLabelHostname->show();
        editHostname->show();
        textLabelBD->show();
        editDatabase->show();
        line->show();
    } else {
        pushButtonProp->setText(tr("Подробно"));
        textLabelHostname->hide();
        editHostname->hide();
        textLabelBD->hide();
        editDatabase->hide();
        line->hide();

    }
    this->adjustSize();
}

void DialogConnect::onClickButtonOk()
{
    this->setDisabled(true);
    COSERVERINFO ServerInfo = {0};
    ServerInfo.pwszName = toLPWSTR(editHostname->text());

    MULTI_QI MultiQi = {0};
    MultiQi.pIID = &__uuidof(LoodsmanServerApplication::IMainSystem);
    HRESULT Hr = ::CoCreateInstanceEx(__uuidof(LoodsmanServerApplication::MainSystem), NULL,
                                      CLSCTX_REMOTE_SERVER, &ServerInfo, 1, &MultiQi);
    if (Hr == S_OK)
    {
        LoodsmanSystem* loodsman = LoodsmanSystem::instance();
        loodsman->main.Attach((LoodsmanServerApplication::IMainSystem*)MultiQi.pItf);

        VARIANT inErrorCode;
        VARIANT stErrorMessage;
        loodsman->main->ConnectToDBEx(to_bstr_t(editDatabase->text()),
                           to_bstr_t(editUsername->text()),
                           to_bstr_t(editPassword->text()),
                           &inErrorCode,&stErrorMessage);

        if (inErrorCode.lVal!=0){
            QMessageBox::warning(this, tr("Ошибка соединения"), from_bstr_t(stErrorMessage.bstrVal));
        } else {
            QSettings settings("NONAME", "VerticalLite");
            settings.beginGroup("DialogConnect");
            settings.setValue("database", editDatabase->text());
            settings.setValue("hostname", editHostname->text());
            settings.setValue("username", editUsername->text());
            settings.endGroup();
            this->accept();
        }
    } else {
        QMessageBox::warning(this, tr("Ошибка соединения"), QString("%1: %2").
                             arg(Hr,0, 16).arg(tr("Ошибка соединения с сервером приложений")));
    }
    this->setEnabled(true);
}
