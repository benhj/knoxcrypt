#include "TeaSafeQTreeVisitor.h"

TeaSafeQTreeVisitor::TeaSafeQTreeVisitor(QTreeWidget *treeWidget,
                                         std::string const& teaPath)
  : m_teaPath(teaPath)
  , m_parent(new QTreeWidgetItem(treeWidget))
{
    m_parent->setText(0, QObject::tr("/"));
}

void TeaSafeQTreeVisitor::enterFolder(teasafe::EntryInfo const &entryInfo)
{
    m_parent = new QTreeWidgetItem(m_parent);
    m_parent->setText(0, QString(entryInfo.filename().c_str()));
}

void TeaSafeQTreeVisitor::enterFile(teasafe::EntryInfo const &entryInfo)
{
    QTreeWidgetItem *child = new QTreeWidgetItem(m_parent);
    child->setText(0, QString(entryInfo.filename().c_str()));
}

void TeaSafeQTreeVisitor::exitFolder(teasafe::EntryInfo const &)
{
    m_parent = m_parent->parent();
}
