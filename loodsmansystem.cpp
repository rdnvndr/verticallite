#include "loodsmansystem.h"

LoodsmanSystem::LoodsmanSystem(QObject *parent) :
    QObject(parent)
{
    m_instance = this;
}

LoodsmanSystem *LoodsmanSystem::m_instance = 0;

LoodsmanSystem *LoodsmanSystem::instance(){
    return m_instance;
}


