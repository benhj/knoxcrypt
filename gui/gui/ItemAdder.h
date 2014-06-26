#ifndef ITEMADDER_H
#define ITEMADDER_H

#include "teasafe/TeaSafe.hpp"
#include <QObject>
#include <QTreeWidgetItem>

class ItemAdder : public QObject
{
    Q_OBJECT
public:
    explicit ItemAdder(QObject *parent = 0);

    void populate(QTreeWidgetItem *parent,
                  teasafe::SharedTeaSafe const &teaSafe,
                  std::string const &path);

signals:

    void finished();

public slots:


};

#endif // ITEMADDER_H
