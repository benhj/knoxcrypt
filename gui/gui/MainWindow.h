#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
    class MainWindow;
}

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
};

#endif // MAINWINDOW_H
