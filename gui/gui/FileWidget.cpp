#include "FileWidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>

FileWidget::FileWidget(QWidget *parent)
  : QTreeWidget(parent)
  , m_currentlyOver(0)
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
    QMimeData const * mimeData = event->mimeData();

   // check for our needed mime type, here a file or a list of files
   if (mimeData->hasUrls()) {
       QList<QUrl> urlList = mimeData->urls();

       // extract the local paths of the files
       for (int i = 0; i < urlList.size() && i < 32; ++i) {
           QTreeWidgetItem * destItem = itemAt(event->pos());
           if(destItem) {
               emit fileDroppedSignal(destItem, urlList.at(i).path().toStdString());
               m_currentlyOver->setBackgroundColor(0, Qt::black);
               m_currentlyOver = 0;
           }

       }
   }

}


void
FileWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidgetItem * destItem = itemAt(event->pos());
    if(destItem != m_currentlyOver) {
        if(m_currentlyOver) { m_currentlyOver->setBackgroundColor(0, Qt::black); }
        if(destItem != 0) {
            m_currentlyOver = destItem;
            m_currentlyOver->setBackgroundColor(0, Qt::gray);
        }
    }
}

