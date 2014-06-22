#ifndef TREEBUILDERTHREAD_H
#define TREEBUILDERTHREAD_H

#include "teasafe/TeaSafe.hpp"
#include <QThread>
#include <QTreeWidget>

class TreeBuilderThread : public QThread
{
    Q_OBJECT
public:
    explicit TreeBuilderThread(QObject *parent = 0);

    void setTeaSafe(teasafe::SharedTeaSafe const &teaSafe);

    QTreeWidgetItem *getRootItem();

signals:
    void finishedBuildingTreeSignal();

protected:
    void run();

public slots:

private:
    QTreeWidgetItem *m_root;
    teasafe::SharedTeaSafe m_teaSafe;

};

#endif // TREEBUILDERTHREAD_H
