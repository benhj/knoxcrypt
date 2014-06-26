#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ItemAdder.h"
#include "TreeItemPathDeriver.h"

#include "teasafe/EntryInfo.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/OpenDisposition.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeFolder.hpp"
#include "utility/CopyFromPhysical.hpp"
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
    m_workThread(this),
    m_populatedSet(),
    m_itemAdder(boost::make_shared<ItemAdder>())
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

    ui->fileTree->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QObject::connect(ui->fileTree, SIGNAL(itemExpanded(QTreeWidgetItem*)),
                     this, SLOT(itemExpanded(QTreeWidgetItem*)));

    m_contextMenu = boost::make_shared<QMenu>(ui->fileTree);
    ui->fileTree->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_extractAction = boost::make_shared<QAction>("Extract", m_contextMenu.get());
    m_removeAction = boost::make_shared<QAction>("Remove", m_contextMenu.get());
    m_newFolderAction = boost::make_shared<QAction>("Create folder", m_contextMenu.get());
    m_addFileAction = boost::make_shared<QAction>("Add file", m_contextMenu.get());
    m_addFolderAction = boost::make_shared<QAction>("Add folder", m_contextMenu.get());
    ui->fileTree->addAction(m_extractAction.get());
    ui->fileTree->addAction(m_removeAction.get());
    ui->fileTree->addAction(m_newFolderAction.get());
    ui->fileTree->addAction(m_addFileAction.get());
    ui->fileTree->addAction(m_addFolderAction.get());
    QObject::connect(m_extractAction.get(), SIGNAL(triggered()), this, SLOT(extractClickedSlot()));
    QObject::connect(m_removeAction.get(), SIGNAL(triggered()), this, SLOT(removedClickedSlot()));
    QObject::connect(m_newFolderAction.get(), SIGNAL(triggered()), this, SLOT(newFolderClickedSlot()));
    QObject::connect(m_addFileAction.get(), SIGNAL(triggered()), this, SLOT(addFileClickedSlot()));
    QObject::connect(m_addFolderAction.get(), SIGNAL(triggered()), this, SLOT(addFolderClickedSlot()));
    QObject::connect(m_itemAdder.get(), SIGNAL(finished()), this, SLOT(itemFinishedExpanding()));

    ui->fileTree->setDragEnabled(true);
    ui->fileTree->setDropIndicatorShown(true);
    ui->fileTree->setAcceptDrops(true);
    ui->fileTree->setDragDropMode(QAbstractItemView::DropOnly);
    ui->fileTree->viewport()->installEventFilter( this ); // that's what you need

    // not sure why I need this, but it prevents errors of the type
    // QObject::connect: Cannot queue arguments of type 'QVector<int>'
    // (Make sure 'QVector<int>' is registered using qRegisterMetaType().)
    qRegisterMetaType<QVector<int> >("QVector<int>");

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

    // set root '/' tree item
    QTreeWidgetItem *parent = new QTreeWidgetItem(ui->fileTree);
    parent->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
    parent->setText(0, QString("/"));
}

void MainWindow::setMaximumProgressSlot(long value)
{
    m_sd->close();
    m_sd = boost::make_shared<QProgressDialog>("Building cipher...", "Cancel", 0, value, this);
    m_sd->setWindowModality(Qt::WindowModal);
    m_sd->exec();
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

void MainWindow::addFileClickedSlot()
{
    this->doWork(WorkType::AddFile);
}

void MainWindow::addFolderClickedSlot()
{
    this->doWork(WorkType::AddFolder);
}

void MainWindow::itemExpanded(QTreeWidgetItem *parent)
{
    std::string pathOfExpanded(detail::getPathFromCurrentItem(parent));

    if(m_populatedSet.find(pathOfExpanded) == m_populatedSet.end()) {
        boost::function<void()> f(boost::bind(&ItemAdder::populate, m_itemAdder, parent,
                                              m_teaSafe, pathOfExpanded));
        m_workThread.addWorkFunction(f);
        m_populatedSet.insert(pathOfExpanded);
    }
}

void MainWindow::itemFinishedExpanding()
{
    ui->fileTree->repaint();
}

bool MainWindow::eventFilter( QObject* o, QEvent* e )
{
    if( o == ui->fileTree->viewport() && e->type() == QEvent::Drop )
    {
        // do what you would do in the slot
        qDebug() << "drop!";
    }

    return false;
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
        } else if(workType == WorkType::AddFolder) {

            if(m_teaSafe->folderExists(teaPath)) {
                std::string fsPath = QFileDialog::getExistingDirectory().toStdString();
                f = boost::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                                teaPath, fsPath);
                QTreeWidgetItem *item = new QTreeWidgetItem(*it);
                item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
                item->setText(0, QString(boost::filesystem::path(fsPath).filename().c_str()));
            }
        }  else if(workType == WorkType::AddFile) {

            if(m_teaSafe->folderExists(teaPath)) {
                std::string fsPath = QFileDialog::getOpenFileName().toStdString();
                f = boost::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                                teaPath, fsPath);
                QTreeWidgetItem *item = new QTreeWidgetItem(*it);
                item->setText(0, QString(boost::filesystem::path(fsPath).filename().c_str()));
            }
        }
        if(f) {
            m_workThread.addWorkFunction(f);
        }
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
