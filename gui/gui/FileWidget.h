#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QTreeWidget>

class FileWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FileWidget(QWidget *parent = 0);

signals:
    // the widget item that was hovered over
    // the string is the path
    void fileDroppedSignal(QTreeWidgetItem*, std::string);

protected:

    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    void dragMoveEvent(QDragMoveEvent *event);

    void dragLeaveEvent(QDragLeaveEvent *event);
private:
    QTreeWidgetItem * m_currentlyOver;
};

#endif // FILEWIDGET_H
