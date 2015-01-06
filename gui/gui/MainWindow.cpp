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
#include "teasafe/CompoundFolder.hpp"
#include "utility/CopyFromPhysical.hpp"
#include "utility/ExtractToPhysical.hpp"
#include "utility/RecursiveFolderExtractor.hpp"
#include "utility/RemoveEntry.hpp"
#include "utility/RandomNumberGenerator.hpp"

#include <QTreeWidgetItem>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>

#include <string>
#include <fstream>
#include <functional>
#include <vector>
#include <memory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_teaSafe(),
    m_loaderThread(this),
    m_builderThread(this),
    m_workThread(this),
    m_cipherCallback(this),
    m_populatedSet(),
    m_itemAdder(std::make_shared<ItemAdder>()),
    m_spinner()
{
    //this->setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);
    QObject::connect(ui->loadButton, SIGNAL(clicked()),
                     this, SLOT(loadFileButtonHandler()));
    QObject::connect(ui->newButton, SIGNAL(clicked()),
                     this, SLOT(newButtonHandler()));
    QObject::connect(&m_loaderThread, SIGNAL(finishedLoadingSignal()), this,
                     SLOT(getTeaSafeFromLoader()));
    QObject::connect(&m_builderThread, SIGNAL(finishedBuildingSignal()), this,
                     SLOT(getTeaSafeFromBuilder()));
    QObject::connect(&m_cipherCallback, SIGNAL(updateProgressSignal(long)), this,
                     SLOT(updateProgressSlot(long)));
    QObject::connect(&m_cipherCallback, SIGNAL(setMaximumProgressSignal(long)), this,
                     SLOT(setMaximumProgressSlot(long)));

    QObject::connect(&m_builderThread, SIGNAL(blockCountSignal(long)), this,
                     SLOT(setMaximumProgressSlot(long)));
    QObject::connect(&m_builderThread, SIGNAL(blockWrittenSignal(long)), this,
                     SLOT(updateProgressSlot(long)));

    ui->fileTree->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QObject::connect(ui->fileTree, SIGNAL(itemExpanded(QTreeWidgetItem*)),
                     this, SLOT(itemExpanded(QTreeWidgetItem*)));

    QObject::connect(&m_cipherCallback, SIGNAL(setProgressLabelSignal(QString)),
                     this, SLOT(setProgressLabel(QString)));

    QObject::connect(&m_builderThread, SIGNAL(setProgressLabelSignal(QString)),
                     this, SLOT(setProgressLabel(QString)));

    QObject::connect(&m_builderThread, SIGNAL(closeProgressSignal()),
                     this, SLOT(closeProgressSlot()));

    QObject::connect(&m_cipherCallback, SIGNAL(closeProgressSignal()),
                     this, SLOT(closeProgressSlot()));

    QObject::connect(&m_workThread, SIGNAL(startedSignal()),
                     this, SLOT(setBusyIndicator()));

    QObject::connect(&m_workThread, SIGNAL(finishedSignal()),
                     this, SLOT(setReadyIndicator()));

    QObject::connect(this, SIGNAL(updateStatusTextSignal(QString)),
                     this, SLOT(updateStatusTextSlot(QString)));

    m_contextMenu = std::make_shared<QMenu>(ui->fileTree);
    ui->fileTree->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_extractAction = std::make_shared<QAction>("Extract", m_contextMenu.get());
    m_removeAction = std::make_shared<QAction>("Remove", m_contextMenu.get());
    m_newFolderAction = std::make_shared<QAction>("Create folder", m_contextMenu.get());
    m_addFileAction = std::make_shared<QAction>("Add file", m_contextMenu.get());
    m_addFolderAction = std::make_shared<QAction>("Add folder", m_contextMenu.get());
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
    QObject::connect(ui->fileTree, SIGNAL(fileDroppedSignal(QTreeWidgetItem*,std::string)),
                     this, SLOT(fileDroppedSlot(QTreeWidgetItem*,std::string)));

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
        teasafe::SharedCoreIO io(std::make_shared<teasafe::CoreTeaSafeIO>());
        io->path = dlg.selectedFiles().at(0).toStdString();

        bool ok;
        io->encProps.password = QInputDialog::getText(this, tr("Password dialog"),
                                             tr("Password:"), QLineEdit::NoEcho, "", &ok).toStdString();

        if((!io->password.empty() && ok)) {

            io->rootBlock = 0;

            // give the cipher generation process a gui callback
            long const amount = teasafe::detail::CIPHER_BUFFER_SIZE / 100000;
            std::function<void(teasafe::EventType)> f(std::bind(&GUICipherCallback::cipherCallback,
                                                                &m_cipherCallback,
                                                                std::placeholders::_1, amount));
            io->ccb = f;

            // create a progress dialog to display progress of cipher generation
            m_sd = std::make_shared<QProgressDialog>("Generating key...", "Cancel", 0, 0, this);
            m_sd->setWindowModality(Qt::WindowModal);

            // start loading of TeaSafe image
            m_loaderThread.setSharedIO(io);
            m_loaderThread.start();
            m_sd->exec();
            ui->nameLabel->setText(io->path.c_str());
        }
    }
}

void MainWindow::newButtonHandler()
{
    qDebug() << "New button placeholder!";

    QFileDialog dlg( NULL, tr("Container save name"));
    dlg.setAcceptMode( QFileDialog::AcceptSave );
    if (dlg.exec()) {

        // reset state
        ui->fileTree->clear();
        teasafe::cipher::IByteTransformer::m_init = false;
        m_teaSafe.reset();
        std::set<std::string>().swap(m_populatedSet);

        // build new state
        teasafe::SharedCoreIO io(std::make_shared<teasafe::CoreTeaSafeIO>());
        io->path = dlg.selectedFiles().at(0).toStdString();

        bool ok;
        QInputDialog input;
        input.setWindowModality(Qt::WindowModal);
        io->password = input.getText(this, tr("Password dialog"),
                                     tr("Password:"), QLineEdit::NoEcho, "", &ok).toStdString();

        if((!io->password.empty() && ok)) {

            QStringList items;
            items.append("aes");
            items.append("twofish");
            items.append("rc6");
            items.append("cast256");
            items.append("mars");
            items.append("serpent");
            QString cipher = QInputDialog::getItem(this, tr("Algorithm choice"),
                                                    tr("Cipher:"), items, 0, false, &ok);
            if(cipher == "aes") {
                io->cipher = 1;
            } else if(cipher == "twofish") {
                io->cipher = 2;
            } else if(cipher == "serpent") {
                io->cipher = 3;
            } else if(cipher == "rc6") {
                io->cipher = 4;
            } else if(cipher == "mars") {
                io->cipher = 5;
            } else if(cipher == "cast256") {
                io->cipher = 6;
            }

            io->rootBlock = 0;
            io->rounds = 64;
            io->encProps.iv = teasafe::utility::random();
            io->encProps.iv2 = teasafe::utility::random();
            io->encProps.iv3 = teasafe::utility::random();
            io->encProps.iv4 = teasafe::utility::random();
            io->encProps.cipher = 1; // AES for now; TODO: add ability to chose

            // note, getInt arguably too constraining
            io->blocks = input.getInt(this, tr("#4096 byte blocks"),
                                      tr("Blocks:"), QLineEdit::Normal, 1280);
            io->freeBlocks = io->blocks;

            // give the cipher generation process a gui callback
            long const amount = teasafe::detail::CIPHER_BUFFER_SIZE / 100000;
            std::function<void(teasafe::EventType)> f(std::bind(&GUICipherCallback::cipherCallback,
                                                                &m_cipherCallback,
                                                                std::placeholders::_1, amount));
            io->ccb = f;

            // create a progress dialog to display progress of cipher generation
            m_sd = std::make_shared<QProgressDialog>("Generating key...", "Cancel", 0, 0, this);
            m_sd->setWindowModality(Qt::WindowModal);

            m_builderThread.setSharedIO(io);

            m_builderThread.start();
            m_sd->exec();
            ui->nameLabel->setText(io->path.c_str());

        }
    }
}

void MainWindow::updateProgressSlot(long const value)
{
    m_sd->setValue(value);
}

void MainWindow::setMaximumProgressSlot(long value)
{
    m_sd->setMaximum(value);
}

void MainWindow::setProgressLabel(QString const &str)
{
    m_sd->show(); // in case is closed
    m_sd->setLabelText(str);
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
        std::function<void()> f(std::bind(&ItemAdder::populate, m_itemAdder, parent,
                                              m_teaSafe, pathOfExpanded));
        m_workThread.addWorkFunction(f);
        m_populatedSet.insert(pathOfExpanded);
    }
}

void MainWindow::itemFinishedExpanding()
{
    ui->fileTree->repaint();
}

void MainWindow::closeProgressSlot()
{
    m_sd->close();
}

void MainWindow::openProgressSlot()
{
    m_sd->show();
}

void MainWindow::getTeaSafeFromLoader()
{
    m_teaSafe = m_loaderThread.getTeaSafe();
    this->createRootFolderInTree();
    //
}

void MainWindow::getTeaSafeFromBuilder()
{
    m_teaSafe = m_builderThread.getTeaSafe();
    this->createRootFolderInTree();
}

void MainWindow::setBusyIndicator()
{
    if(!m_spinner) {

        m_spinner = std::make_shared<QMovie>(":/new/prefix1/graphix/spinner.gif");
        m_spinner->setScaledSize(QSize(16,16));
        ui->blinkerLabel->setMovie(m_spinner.get());
        m_spinner->start();
    }
}

void MainWindow::setReadyIndicator()
{
    m_spinner->stop();
    m_spinner.reset();
    ui->blinkerLabel->clear();
}

void MainWindow::loggerCallback(std::string const &str)
{
    emit updateStatusTextSignal(QString(str.c_str()));
}

void MainWindow::updateStatusTextSlot(QString const &str)
{
    ui->statusText->appendPlainText(str);
}

void MainWindow::fileDroppedSlot(QTreeWidgetItem *dropItem, std::string const &fsPath)
{
    auto itemPath(detail::getPathFromCurrentItem(dropItem));
    auto theFSPath(fsPath);
    auto newItemText(QString(boost::filesystem::path(fsPath).filename().c_str()));
    if(boost::filesystem::is_directory(fsPath)) {
        theFSPath = boost::filesystem::path(fsPath).parent_path().string();
        newItemText = QString(boost::filesystem::path(theFSPath).filename().c_str());
    }

    // if item is a folder, add as a child; if item is a file
    // add to parent
    // ------------------------------------------------------
    bool isFolder = false;
    // Note root is a special case, as it is always a folder
    if(itemPath == "/") {
        isFolder = true;
    } else {
        // Not the root folder so check entry info instead for type
        auto info(m_teaSafe->getInfo(itemPath));
        if(info.type() == teasafe::EntryType::FolderType) {
            isFolder = true;
        }
    }
    if(isFolder) {
        std::function<void(std::string)> cb(std::bind(&MainWindow::loggerCallback,
                                                      this, std::placeholders::_1));
        auto f(std::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                         itemPath, theFSPath, cb));
        QTreeWidgetItem *item = new QTreeWidgetItem(dropItem);
        if(boost::filesystem::is_directory(theFSPath)) {
            item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
        }
        item->setText(0, newItemText);
        m_workThread.addWorkFunction(f);
    } else {

        std::function<void(std::string)> cb(std::bind(&MainWindow::loggerCallback,
                                                      this, std::placeholders::_1));

        boost::filesystem::path parentTeaPath(itemPath);
        parentTeaPath = parentTeaPath.parent_path();

        auto f(std::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                         parentTeaPath.string(), theFSPath, cb));
        QTreeWidgetItem *item = new QTreeWidgetItem(dropItem->parent());
        if(boost::filesystem::is_directory(theFSPath)) {
            item->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
        }
        item->setText(0, newItemText);
        m_workThread.addWorkFunction(f);
    }
}

void MainWindow::doWork(WorkType workType)
{
    QList<QTreeWidgetItem*> selectedItems = ui->fileTree->selectedItems();
    for (auto const &it : selectedItems) {
        std::string teaPath(detail::getPathFromCurrentItem(it));
        qDebug() << teaPath.c_str();
        std::function<void()> f;
        if(workType == WorkType::RemoveItem) {
            qDebug() << "RemoveItem";
            f = std::bind(teasafe::utility::removeEntry, boost::ref(*m_teaSafe),
                          teaPath);

            delete it;
        } else if(workType == WorkType::ExtractItem) {
            QFileDialog dlg( NULL, tr("Filesystem folder to extract to.."));
            dlg.setAcceptMode( QFileDialog::AcceptOpen );
            dlg.setFileMode(QFileDialog::DirectoryOnly);
            if(dlg.exec()) {
                std::string fsPath = dlg.selectedFiles().at(0).toStdString();

                qDebug() << "ExtractItem";
                std::function<void(std::string)> cb(std::bind(&MainWindow::loggerCallback,
                                                              this, std::placeholders::_1));
                f = std::bind(teasafe::utility::extractToPhysical, boost::ref(*m_teaSafe),
                                teaPath, fsPath, cb);
            }

        } else if(workType == WorkType::CreateFolder) {
            qDebug() << "CreateFolder";
            if(m_teaSafe->folderExists(teaPath)) {
                bool ok;
                std::string folderName = QInputDialog::getText(this, tr("New folder name dialog"),
                                                               tr("Folder name:"), QLineEdit::Normal,"",&ok).toStdString();

                if((!folderName.empty()) && ok) {
                    std::string path((it)->text(0).toStdString() == "/" ?
                                     teaPath.append(folderName) :
                                     teaPath.append("/").append(folderName));

                    f = std::bind(&teasafe::TeaSafe::addFolder, m_teaSafe, path);
                    QTreeWidgetItem *item = new QTreeWidgetItem(it);
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
                    std::function<void(std::string)> cb(std::bind(&MainWindow::loggerCallback,
                                                                  this, std::placeholders::_1));
                    f = std::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                                    teaPath, fsPath, cb);
                    QTreeWidgetItem *item = new QTreeWidgetItem(it);
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
                    std::function<void(std::string)> cb(std::bind(&MainWindow::loggerCallback,
                                                                  this, std::placeholders::_1));
                    f = std::bind(&teasafe::utility::copyFromPhysical, boost::ref(*m_teaSafe),
                                    teaPath, fsPath, cb);
                    QTreeWidgetItem *item = new QTreeWidgetItem(it);
                    item->setText(0, QString(boost::filesystem::path(fsPath).filename().c_str()));
                }
            }
        }
        if(f) {
            m_workThread.addWorkFunction(f);
        }
    }
}

void MainWindow::createRootFolderInTree()
{
    // set root '/' tree item
    QTreeWidgetItem *parent = new QTreeWidgetItem(ui->fileTree);
    parent->setChildIndicatorPolicy (QTreeWidgetItem::ShowIndicator);
    parent->setText(0, QString("/"));
}
