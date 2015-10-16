#include "attrlinkkey.h"

AttrLinkKey::AttrLinkKey()
{
}

bool AttrLinkKey::operator <(const AttrLinkKey &key2) const
{
    if (id != key2.id) return id < key2.id;
    if (pid != key2.pid) return pid < key2.pid;
    return lid < key2.lid;

}
