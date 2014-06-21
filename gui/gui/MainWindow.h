#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "LoaderThread.h"
#include "utility/EventType.hpp"
#include <boost/shared_ptr.hpp>
#include <QMainWindow>
#include <QProgressDialog>

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

    void cipherCallback(teasafe::EventType eventType, long const amount);

public slots:
    void finishedLoadingSlot();
    /**
     * @brief loadFileButtonHandler for loading a TeaSafe image
     */
    void loadFileButtonHandler();

    void updateProgressSlot();
    void closeProgressSlot();
    void setMaximumProgressSlot(long value);

signals:
    void updateProgressSignal();
    void closeProgressSignal();
    void setMaximumProgressSignal(long);

private:
    Ui::MainWindow *ui;
    SharedTeaSafe m_teaSafe;
    LoaderThread m_loaderThread;
    typedef boost::shared_ptr<QProgressDialog> SharedDialog;
    SharedDialog m_sd;
};

#endif // MAINWINDOW_H
