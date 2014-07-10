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



#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ItemAdder.h"
#include "TreeItemPathDeriver.h"

#include "cipher/IByteTransformer.hpp"
#include "teasafe/EntryInfo.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/OpenDisposition.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeFolder.hpp"
#include "utility/CopyFromPhysical.hpp"
#include "utility/ExtractToPhysical.hpp"
#include "utility/RecursiveFolderExtractor.hpp"
#include "utility/RemoveEntry.hpp"
#include "utility/RandomNumberGenerator.hpp"

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
    m_teaSafe(),
    m_loaderThread(this),
    m_builderThread(this),
    m_workThread(this),
    m_populatedSet(),
    m_itemAdder(boost::make_shared<ItemAdder>())
{
    ui->setupUi(this);
    QObject::connect(ui->loadButton, SIGNAL(clicked()),
                     this, SLOT(loadFileButtonHandler()));
    QObject::connect(ui->newButton, SIGNAL(clicked()),
                     this, SLOT(newButtonHandler()));
    QObject::connect(&m_loaderThread, SIGNAL(finishedLoadingSignal()), this,
                     SLOT(finishedLoadingSlot()));
    QObject::connect(this, SIGNAL(updateProgressSignal(long)), this,
                     SLOT(updateProgressSlot(long)));
    QObject::connect(this, SIGNAL(cipherGeneratedSignal()), this,
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
    QFileDialog dlg( NULL, tr("Open container"));
    dlg.setAcceptMode( QFileDialog::AcceptOpen );
    dlg.setFileMode(QFileDialog::ExistingFile);

    if (dlg.exec()) {

        // reset state
        ui->fileTree->clear();
        teasafe::cipher::IByteTransformer::m_init = false;
        m_teaSafe.reset();
        std::set<std::string>().swap(m_populatedSet);

        // build new state
        teasafe::SharedCoreIO io(boost::make_shared<teasafe::CoreTeaSafeIO>());
        io->path = dlg.selectedFiles().at(0).toStdString();

        bool ok;
        io->password = QInputDialog::getText(this, tr("Password dialog"),
                                             tr("Password:"), QLineEdit::NoEcho, "", &ok).toStdString();

        if((!io->password.empty() && ok)) {
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
    }
}

void MainWindow::newButtonHandler()
{
    qDebug() << "New button placeholder!";

    QFileDialog dlg( NULL, tr("Container save name"));
    dlg.setAcceptMode( QFileDialog::AcceptSave );
    if (dlg.exec()) {
        // build new state
        teasafe::SharedCoreIO io(boost::make_shared<teasafe::CoreTeaSafeIO>());
        io->path = dlg.selectedFiles().at(0).toStdString();

        bool ok;
        QInputDialog input;
        input.setWindowModality(Qt::WindowModal);
        io->password = input.getText(this, tr("Password dialog"),
                                     tr("Password:"), QLineEdit::NoEcho, "", &ok).toStdString();

        if((!io->password.empty() && ok)) {
            io->rootBlock = 0;
            io->rounds = 64;
            io->iv = teasafe::utility::random();

            // note, getInt arguably too constraining
            io->blocks = input.getInt(this, tr("#4096 byte blocks"),
                                      tr("Blocks:"), QLineEdit::Normal, 12800);
            io->freeBlocks = io->blocks;

            // give the cipher generation process a gui callback
            long const amount = teasafe::detail::CIPHER_BUFFER_SIZE / 100000;
            boost::function<void(teasafe::EventType)> f(boost::bind(&MainWindow::cipherCallback, this, _1, amount));
            io->ccb = f;

            // create a progress dialog to display progress of cipher generation
            m_sd = boost::make_shared<QProgressDialog>("Generating key...", "Cancel", 0, 0, this);
            m_sd->setWindowModality(Qt::WindowModal);

            m_builderThread.setSharedIO(io);

        }
    }
}

void MainWindow::updateProgressSlot(long const value)
{
    m_sd->setValue(value);
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
    m_sd->setLabelText("Building cipher...");
    m_sd->setMaximum(value);
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
            QFileDialog dlg( NULL, tr("Filesystem folder to extract to.."));
            dlg.setAcceptMode( QFileDialog::AcceptOpen );
            dlg.setFileMode(QFileDialog::DirectoryOnly);
            if(dlg.exec()) {
                std::string fsPath = dlg.selectedFiles().at(0).toStdString();

                qDebug() << "ExtractItem";
                f = boost::bind(teasafe::utility::extractToPhysical, boost::ref(*m_teaSafe),
                                teaPath, fsPath);
            }

        } else if(workType == WorkType::CreateFolder) {
            qDebug() << "CreateFolder";
            if(m_teaSafe->folderExists(teaPath)) {
                bool ok;
                std::string folderName = QInputDialog::getText(this, tr("New folder name dialog"),
                                                               tr("Folder name:"), QLineEdit::Normal,"",&ok).toStdString();

                if((!folderName.empty()) && ok) {
                    std::string path((*it)->text(0).toStdString() == "/" ?
                                     teaPath.append(folderName) :
                                     teaPath.append("/").append(folderName));

                    f = boost::bind(&teasafe::TeaSafe::addFolder, m_teaSafe, path);
                    QTreeWidgetItem *item = new QTreeWidgetItem(*it);
                    item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
                    item->setText(0, QString(folderName.c_str()));
                }
            }
        } else if(workType == WorkType::AddFolder) {

            if(m_teaSafe->folderExists(teaPath)) {
                QFileDialog dlg( NULL, tr("Folder to add.."));
                dlg.setAcceptMode( QFileDialog::AcceptOpen );
                dlg.setFileMode(QFileDialog::DirectoryOnly);
                if(dlg.exec()) {
                    std::string fsPath = dlg.selectedFiles().at(0).toStdString();
                    f = boost::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                                    teaPath, fsPath);
                    QTreeWidgetItem *item = new QTreeWidgetItem(*it);
                    item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
                    item->setText(0, QString(boost::filesystem::path(fsPath).filename().c_str()));
                }
            }
        }  else if(workType == WorkType::AddFile) {

            if(m_teaSafe->folderExists(teaPath)) {
                QFileDialog dlg( NULL, tr("File to add.."));
                dlg.setAcceptMode( QFileDialog::AcceptOpen );
                dlg.setFileMode(QFileDialog::AnyFile);
                if(dlg.exec()) {
                    std::string fsPath = dlg.selectedFiles().at(0).toStdString();
                    f = boost::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                                    teaPath, fsPath);
                    QTreeWidgetItem *item = new QTreeWidgetItem(*it);
                    item->setText(0, QString(boost::filesystem::path(fsPath).filename().c_str()));
                }
            }
        }
        if(f) {
            m_workThread.addWorkFunction(f);
        }
    }
}

void MainWindow::cipherCallback(teasafe::EventType eventType, long const amount)
{
    static long value(0);
    if (eventType == teasafe::EventType::BigCipherBuildBegin) {
        qDebug() << "Got event";
        emit setMaximumProgressSignal(amount);
    }
    if (eventType == teasafe::EventType::CipherBuildUpdate) {
        emit updateProgressSignal(value++);
    }
    if (eventType == teasafe::EventType::BigCipherBuildEnd) {
        value = 0;
        emit cipherGeneratedSignal();
    }
}

void MainWindow::finishedLoadingSlot()
{
    qDebug() << "Finished loading!";
}
