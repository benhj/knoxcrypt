#include "ItemAdder.h"
#include "teasafe/EntryInfo.hpp"
#include <vector>

ItemAdder::ItemAdder(QObject *parent) :
    QObject(parent)
{
}

void ItemAdder::populate(QTreeWidgetItem *parent,
                         teasafe::SharedTeaSafe const &teaSafe,
                         std::string const &path)
{
    teasafe::TeaSafeFolder f = teaSafe->getTeaSafeFolder(path);
    std::vector<teasafe::EntryInfo> entryInfos = f.listAllEntries();
    std::vector<teasafe::EntryInfo>::iterator it = entryInfos.begin();
    for(; it != entryInfos.end(); ++it) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        if(it->type() == teasafe::EntryType::FolderType) {
            item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
        }
        item->setText(0, QString(it->filename().c_str()));
    }

    emit finished();
}
