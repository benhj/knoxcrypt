#include "FileWidget.h"

#include <QDebug>
#include <QDragEnterEvent>

FileWidget::FileWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DropOnly);
}

void
FileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void
FileWidget::dragLeaveEvent(QDragLeaveEvent *)
{
}

bool
FileWidget::dropMimeData(QTreeWidgetItem * parent,
                         int index,
                         const QMimeData * data,
                         Qt::DropAction action )
{
    qDebug() << "Dropped";
    return true;
}

void
FileWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "Drop event!";
}

void
FileWidget::dragMoveEvent(QDragMoveEvent *)
{
}

