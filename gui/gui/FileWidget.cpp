#include "FileWidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>

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

void
FileWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "Drop event!";

    QMimeData const * mimeData = event->mimeData();

   // check for our needed mime type, here a file or a list of files
   if (mimeData->hasUrls()) {
       QList<QUrl> urlList = mimeData->urls();

       // extract the local paths of the files
       for (int i = 0; i < urlList.size() && i < 32; ++i) {

           // call a function to add files (TODO)
           // try to insert item into position... (TODO)
           QTreeWidgetItem * destItem = itemAt(event->pos());
           if(destItem) {
               emit fileDroppedSignal(destItem, urlList.at(i).path().toStdString());
           }

       }
   }

}

void
FileWidget::dragMoveEvent(QDragMoveEvent *)
{
}

