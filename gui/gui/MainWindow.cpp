#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "TeaSafeQTreeVisitor.h"
#include "TreeItemPathDeriver.h"

#include "teasafe/EntryInfo.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/OpenDisposition.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeFolder.hpp"
#include "utility/ExtractToPhysical.hpp"
#include "utility/RecursiveFolderExtractor.hpp"
#include "utility/RemoveEntry.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>

#include <string>
#include <fstream>
#include <vector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_loaderThread(this),
    m_workThread(this)
{
    ui->setupUi(this);
    QObject::connect(ui->loadButton, SIGNAL(clicked()),
                     this, SLOT(loadFileButtonHandler()));
    QObject::connect(&m_loaderThread, SIGNAL(finishedLoadingSignal()), this,
                     SLOT(finishedLoadingSlot()));
    QObject::connect(this, SIGNAL(updateProgressSignal()), this,
                     SLOT(updateProgressSlot()));
    QObject::connect(this, SIGNAL(cipherGeneratedSIgnal()), this,
                     SLOT(cipherGeneratedSlot()));
    QObject::connect(this, SIGNAL(setMaximumProgressSignal(long)), this,
                     SLOT(setMaximumProgressSlot(long)));

    QObject::connect(&m_workThread, SIGNAL(startedSignal()), this, SLOT(extractBegin()));
    QObject::connect(&m_workThread, SIGNAL(finishedSignal()), this, SLOT(extractEnd()));

    ui->fileTree->setAttribute(Qt::WA_MacShowFocusRect, 0);

    m_contextMenu = boost::make_shared<QMenu>(ui->fileTree);
    ui->fileTree->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_extractAction = boost::make_shared<QAction>("Extract", m_contextMenu.get());
    m_removeAction = boost::make_shared<QAction>("Remove", m_contextMenu.get());
    m_newFolderAction = boost::make_shared<QAction>("Create folder", m_contextMenu.get());
    ui->fileTree->addAction(m_extractAction.get());
    ui->fileTree->addAction(m_removeAction.get());
    ui->fileTree->addAction(m_newFolderAction.get());
    QObject::connect(m_extractAction.get(), SIGNAL(triggered()), this, SLOT(extractClickedSlot()));
    QObject::connect(m_removeAction.get(), SIGNAL(triggered()), this, SLOT(removedClickedSlot()));
    QObject::connect(m_newFolderAction.get(), SIGNAL(triggered()), this, SLOT(newFolderClickedSlot()));

    // will process any 'jobs' (e.g. extractions, adds, removals etc.)
    m_workThread.start();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFileButtonHandler()
{
    teasafe::SharedCoreIO io(boost::make_shared<teasafe::CoreTeaSafeIO>());
    io->path = QFileDialog::getOpenFileName().toStdString();
    io->password = QInputDialog::getText(this, tr("Password dialog"),
                                         tr("Password:"), QLineEdit::NoEcho).toStdString();
    io->rootBlock = 0;

    // give the cipher generation process a gui callback
    long const amount = teasafe::detail::CIPHER_BUFFER_SIZE / 100000;
    boost::function<void(teasafe::EventType)> f(boost::bind(&MainWindow::cipherCallback, this, _1, amount));
    io->ccb = f;

    // create a progress dialog to display progress of cipher generation
    m_sd = boost::make_shared<QProgressDialog>("Generating key...", "Cancel", 0, 0, this);
    m_sd->setWindowModality(Qt::WindowModal);

    // start loading of TeaSafe image
    m_loaderThread.setSharedIO(io);
    m_loaderThread.start();
    m_sd->exec();
}

void MainWindow::updateProgressSlot()
{
    static long value(0);
    m_sd->setValue(value++);
}

void MainWindow::cipherGeneratedSlot()
{
    m_sd->close();
    m_teaSafe = m_loaderThread.getTeaSafe();
    m_treeThread = boost::make_shared<TreeBuilderThread>();
    m_treeThread->setTeaSafe(m_teaSafe);
    m_treeThread->start();
    QObject::connect(m_treeThread.get(), SIGNAL(finishedBuildingTreeSignal()),
                     this, SLOT(finishedTreeBuildingSlot()));

    m_sd = boost::make_shared<QProgressDialog>("Reading image...", "Cancel", 0, 0, this);
    m_sd->setWindowModality(Qt::WindowModal);
    m_sd->exec();
}

void MainWindow::setMaximumProgressSlot(long value)
{
    m_sd->close();
    m_sd = boost::make_shared<QProgressDialog>("Building cipher...", "Cancel", 0, value, this);
    m_sd->setWindowModality(Qt::WindowModal);
    m_sd->exec();
}

void MainWindow::finishedTreeBuildingSlot()
{
    m_sd->close();
    QTreeWidgetItem *rootItem = m_treeThread->getRootItem();
    ui->fileTree->setColumnCount(1);
    ui->fileTree->addTopLevelItem(rootItem);
}


void MainWindow::extractClickedSlot()
{
    this->doWork(WorkType::ExtractItem);
}

void MainWindow::removedClickedSlot()
{
    this->doWork(WorkType::RemoveItem);
}

void MainWindow::newFolderClickedSlot()
{
    this->doWork(WorkType::CreateFolder);
}

void MainWindow::extractBegin()
{
    /*
    m_sd = boost::make_shared<QProgressDialog>("Extracting...", "Cancel", 0, 0, this);
    m_sd->setWindowModality(Qt::WindowModal);
    m_sd->exec();
    */
}

void MainWindow::extractEnd()
{
    //m_sd->close();
}

void MainWindow::doWork(WorkType workType)
{
    QList<QTreeWidgetItem*> selectedItems = ui->fileTree->selectedItems();
    QList<QTreeWidgetItem*>::iterator it = selectedItems.begin();
    for (; it != selectedItems.end(); ++it) {
        std::string teaPath(detail::getPathFromCurrentItem(*it));
        qDebug() << teaPath.c_str();
        boost::function<void()> f;
        if(       workType == WorkType::RemoveItem) {
            qDebug() << "RemoveItem";
            f = boost::bind(teasafe::utility::removeEntry, boost::ref(*m_teaSafe),
                            teaPath);

            delete *it;
        } else if(workType == WorkType::ExtractItem) {
            std::string fsPath = QFileDialog::getExistingDirectory().toStdString();
            qDebug() << "ExtractItem";
            f = boost::bind(teasafe::utility::extractToPhysical, boost::ref(*m_teaSafe),
                            teaPath, fsPath);


        } else if(workType == WorkType::CreateFolder) {
            qDebug() << "CreateFolder";
            if(m_teaSafe->folderExists(teaPath)) {
                std::string folderName = QInputDialog::getText(this, tr("New folder name dialog"),
                                                               tr("Folder name:"), QLineEdit::Normal).toStdString();

                std::string path((*it)->text(0).toStdString() == "/" ?
                                 teaPath.append(folderName) :
                                 teaPath.append("/").append(folderName));

                f = boost::bind(&teasafe::TeaSafe::addFolder, m_teaSafe, path);
                QTreeWidgetItem *item = new QTreeWidgetItem(*it);
                item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
                item->setText(0, QString(folderName.c_str()));
            }
        }
        m_workThread.addWorkFunction(f);
    }
}

void MainWindow::cipherCallback(teasafe::EventType eventType, long const amount)
{
    if (eventType == teasafe::EventType::BigCipherBuildBegin) {
        qDebug() << "Got event";
        emit setMaximumProgressSignal(amount);
    }
    if (eventType == teasafe::EventType::CipherBuildUpdate) {
        //m_sd->setValue(value++);
        emit updateProgressSignal();
    }
    if (eventType == teasafe::EventType::BigCipherBuildEnd) {
        emit cipherGeneratedSIgnal();
    }
}

void MainWindow::finishedLoadingSlot()
{
    qDebug() << "Finished loading!";
}
