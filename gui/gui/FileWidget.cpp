#include "FileWidget.h"

#include <QDebug>

FileWidget::FileWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setAcceptDrops(true);
    setDragEnabled(true);
    //setDragDropMode(QAbstractItemView::InternalMove);
}

void
FileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeWidget::dragEnterEvent(event);
    qDebug() << "drag enter event";
}

bool
FileWidget::dropMimeData(QTreeWidgetItem * parent,
                         int index,
                         const QMimeData * data,
                         Qt::DropAction action )
{
    QTreeWidget::dropMimeData(parent, index, data, action);
    qDebug() << "Dropped";
    return true;
}

void
FileWidget::dropEvent(QDropEvent *event)
{
    QTreeWidget::dropEvent(event);
    qDebug() << "Drop event!";
}
