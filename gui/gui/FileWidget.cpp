#include "FileWidget.h"

#include <QDebug>

FileWidget::FileWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setAcceptDrops(true);
//    setDragEnabled(true);
//    setDropIndicatorShown(true);
//    setDragDropMode(QAbstractItemView::DragDrop);
}

void
FileWidget::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "drag enter event";
    QTreeWidget::dragEnterEvent(event);

}

void
FileWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug() << "drag leave event";
    QTreeWidget::dragLeaveEvent(event);
}

bool
FileWidget::dropMimeData(QTreeWidgetItem * parent,
                         int index,
                         const QMimeData * data,
                         Qt::DropAction action )
{
    qDebug() << "Dropped";
    QTreeWidget::dropMimeData(parent, index, data, action);
    return true;
}

void
FileWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "Drop event!";
    QTreeWidget::dropEvent(event);

}

void
FileWidget::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug() << "Drag move event!";
    QTreeWidget::dragMoveEvent(event);

}

