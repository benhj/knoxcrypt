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

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>

#include <fstream>
#include <vector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_loaderThread(this)
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

    m_contextMenu = boost::make_shared<QMenu>(ui->fileTree);
    ui->fileTree->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_extractAction = boost::make_shared<QAction>("Extract", m_contextMenu.get());
    ui->fileTree->addAction(m_extractAction.get());
    QObject::connect(m_extractAction.get(), SIGNAL(triggered()), this, SLOT(extractClickedSlot()));

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
    qDebug() << "extract clicked!";
    QList<QTreeWidgetItem*> selectedItems = ui->fileTree->selectedItems();
    QList<QTreeWidgetItem*>::iterator it = selectedItems.begin();
    for(; it != selectedItems.end(); ++it) {
        std::string teaPath(detail::getPathFromCurrentItem(*it));
        std::string fsPath = QFileDialog::getExistingDirectory().toStdString();
        m_extractorThread = boost::make_shared<ExtractorThread>(m_teaSafe, teaPath, fsPath);
        QObject::connect(m_extractorThread.get(), SIGNAL(startedSignal()), this, SLOT(extractBegin()));
        QObject::connect(m_extractorThread.get(), SIGNAL(finishedSignal()), this, SLOT(extractEnd()));
        m_extractorThread->start();
    }
}

void MainWindow::extractBegin()
{
    m_sd = boost::make_shared<QProgressDialog>("Reading image...", "Cancel", 0, 0, this);
    m_sd->setWindowModality(Qt::WindowModal);
    m_sd->exec();
}

void MainWindow::extractEnd()
{
    m_sd->close();
}

void MainWindow::cipherCallback(teasafe::EventType eventType, long const amount)
{
    if(eventType == teasafe::EventType::BigCipherBuildBegin) {
        qDebug() << "Got event";
        emit setMaximumProgressSignal(amount);
    }
    if(eventType == teasafe::EventType::CipherBuildUpdate) {
        //m_sd->setValue(value++);
        emit updateProgressSignal();
    }
    if(eventType == teasafe::EventType::BigCipherBuildEnd) {
        emit cipherGeneratedSIgnal();
    }
}

void MainWindow::finishedLoadingSlot()
{
    qDebug() << "Finished loading!";
}

