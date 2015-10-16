#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QDebug>

class Application : public QApplication
{
    Q_OBJECT
public:
   Application(int &c, char **v);
   bool notify(QObject *rec, QEvent *ev) {
       // cDebug() << "Called Application::notify()" << endl;
       try {
           return QApplication::notify(rec, ev);
       }
       catch (char const *str) {
           qDebug() << "EXCEPTION: " << str << endl;
           return false;
       }
       catch (...) {
           qDebug() << "Unknown exception!" << endl;
           //abort();
       }
   }
};

#endif // APPLICATION_H


