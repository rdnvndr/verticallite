#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include "mainwindow.h"
#include "loodsmansystem.h"
#include "application.h"

int main(int argc, char *argv[])
{
    Application *app = new Application(argc,argv);
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    // Подключение перевода для Qt
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator(app);
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app->installTranslator(translator);

    // Подключение переводов приложения
    // Переводы находятся в <Каталог приложения>\translations\<Системная локализация>
    QDir translationDir = QDir(qApp->applicationDirPath());
    if (translationDir.cd("translations\\"+QLocale::system().name()))
        foreach(QString fileName, translationDir.entryList(QDir::Files))
            if (translator->load(fileName,translationDir.absolutePath()))
                app->installTranslator(translator);


    LoodsmanSystem loodsmanSystem;
    MainWindow *window = new MainWindow();
    if (window->onActionConnect()==false)
        return 0;
    window->show();
        
    return app->exec();
}
