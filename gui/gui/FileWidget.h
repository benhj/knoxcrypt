#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QTreeWidget>

class FileWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FileWidget(QWidget *parent = 0);

protected:

    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    void dragMoveEvent(QDragMoveEvent *event);

    void dragLeaveEvent(QDragLeaveEvent *event);
};

#endif // FILEWIDGET_H
