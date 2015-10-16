#ifndef LOODSMANSYSTEM_H
#define LOODSMANSYSTEM_H

// Loodsman Library: LoodsmanServerApplication.dll
#import "libid:36281981-5E66-4277-94EE-5F490981CE89"
#include "debug/LoodsmanServerApplication.tlh"
//#include "release/LoodsmanServerApplication.tlh"
using namespace LoodsmanServerApplication;

//#import "libid:7CC86059-0262-44D5-9AA3-033DB38F11EF"
//#include "debug/Loodsman.tlh"
//#include "release/Loodsman.tlh"
//using namespace Loodsman;

#include <QObject>

class LoodsmanSystem : public QObject
{
    Q_OBJECT
public:
    LoodsmanSystem(QObject *parent = 0);
    static LoodsmanSystem *instance();

    IMainSystemPtr main;
private:
    static LoodsmanSystem *m_instance;
};

#endif // LOODSMANSYSTEM_H
