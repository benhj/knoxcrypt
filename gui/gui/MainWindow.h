/*
  Copyright (c) <2014>, <BenHJ>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WorkThread.h"
#include "LoaderThread.h"
#include "ContainerBuilderThread.h"
#include "GUICipherCallback.h"
#include "utility/EventType.hpp"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <QMainWindow>
#include <QProgressDialog>
#include <QMenu>
#include <QAction>
#include <QMovie>
#include <set>

namespace Ui
{
    class MainWindow;
}

namespace teasafe
{
    class TeaSafe;
}

typedef boost::shared_ptr<teasafe::TeaSafe> SharedTeaSafe;

enum class WorkType { RemoveItem, CreateFolder, ExtractItem, AddFile, AddFolder };

class QTreeWidgetItem;
class ItemAdder;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

  public slots:
    /**
     * @brief loadFileButtonHandler for loading a TeaSafe image
     */
    void loadFileButtonHandler();
    void newButtonHandler();

    void updateProgressSlot(long);
    void setMaximumProgressSlot(long value);
    void setProgressLabel(QString const &str);
    void extractClickedSlot();
    void removedClickedSlot();
    void newFolderClickedSlot();
    void addFileClickedSlot();
    void addFolderClickedSlot();
    void itemExpanded(QTreeWidgetItem *);
    void itemFinishedExpanding();
    bool eventFilter(QObject* o, QEvent* e);

    void closeProgressSlot();
    void openProgressSlot();

    void getTeaSafeFromLoader();
    void getTeaSafeFromBuilder();

    void setBusyIndicator();
    void setReadyIndicator();

    void loggerCallback(std::string const &str);

    void updateStatusTextSlot(QString const &str);

  signals:
    void updateProgressSignal(long);
    void setMaximumProgressSignal(long);
    void setProgressLabelSignal(QString);
    void closeProgressSignal();
    void updateStatusTextSignal(QString);

  private:
    Ui::MainWindow *ui;
    SharedTeaSafe m_teaSafe;
    LoaderThread m_loaderThread;
    ContainerBuilderThread m_builderThread;
    WorkThread m_workThread;
    GUICipherCallback m_cipherCallback;
    boost::shared_ptr<QMenu> m_contextMenu;
    boost::shared_ptr<QAction> m_extractAction;
    boost::shared_ptr<QAction> m_removeAction;
    boost::shared_ptr<QAction> m_newFolderAction;
    boost::shared_ptr<QAction> m_addFileAction;
    boost::shared_ptr<QAction> m_addFolderAction;
    typedef boost::shared_ptr<QProgressDialog> SharedDialog;
    SharedDialog m_sd;
    std::set<std::string> m_populatedSet;
    boost::shared_ptr<ItemAdder> m_itemAdder;
    boost::shared_ptr<QMovie> m_spinner;
    void doWork(WorkType workType);
    void createRootFolderInTree();
};

#endif // MAINWINDOW_H
