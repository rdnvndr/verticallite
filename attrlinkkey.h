#ifndef ATTRLINKKEY_H
#define ATTRLINKKEY_H
#include <QString>

class AttrLinkKey;

class AttrLinkKey
{
public:
    AttrLinkKey();
    bool operator<(const AttrLinkKey &key2) const;
public:
    long id;
    long pid;
    QString lid;
};

#endif // ATTRLINKKEY_H
