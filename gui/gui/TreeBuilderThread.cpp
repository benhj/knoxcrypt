#include "TreeBuilderThread.h"
#include "TeaSafeQTreeVisitor.h"
#include "utility/RecursiveFolderExtractor.hpp"

#include <QDebug>

TreeBuilderThread::TreeBuilderThread(QObject *parent)
  : QThread(parent)
{
}

void TreeBuilderThread::setTeaSafe(teasafe::SharedTeaSafe const &teaSafe)
{
    m_teaSafe = teaSafe;
}

QTreeWidgetItem *TreeBuilderThread::getRootItem()
{
    return m_root;
}

void TreeBuilderThread::run()
{
    std::string path("/");
    m_root = new QTreeWidgetItem;
    TeaSafeQTreeVisitor visitor(m_root, path);
    while(!m_teaSafe){} // incredibly dirty hack. I'm embarassed.
    qDebug() << "after!";
    teasafe::utility::recursiveExtract(visitor, *m_teaSafe, path);
    emit finishedBuildingTreeSignal();
}
