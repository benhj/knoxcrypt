#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <boost/shared_ptr.hpp>
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

namespace teasafe {
    class TeaSafe;
}

typedef boost::shared_ptr<teasafe::TeaSafe> SharedTeaSafe;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    /**
     * @brief testItems for testing how QTreeWidgetItems works
     */
    void testItems();
    ~MainWindow();

public slots:

    /**
     * @brief loadFileButtonHandler for loading a TeaSafe image
     */
    void loadFileButtonHandler();


private:
    Ui::MainWindow *ui;
    SharedTeaSafe m_teaSafe;
};

#endif // MAINWINDOW_H
