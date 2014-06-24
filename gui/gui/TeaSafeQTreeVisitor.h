#ifndef TEASAFEQTREEVISITOR_H
#define TEASAFEQTREEVISITOR_H

#include "utility/TeaSafeFolderVisitor.hpp"

#include <QTreeWidget>
#include <QTreeWidgetItem>

class TeaSafeQTreeVisitor : public teasafe::utility::TeaSafeFolderVisitor
{
  public:
    TeaSafeQTreeVisitor(QTreeWidgetItem *parent,
                        std::string const& teaPath);

    void enterFolder(teasafe::EntryInfo const&);
    void enterFile(teasafe::EntryInfo const&);
    void exitFolder(teasafe::EntryInfo const&);

  private:
    std::string m_teaPath;
    QTreeWidgetItem *m_parent;


};

#endif // TEASAFEQTREEVISITOR_H
