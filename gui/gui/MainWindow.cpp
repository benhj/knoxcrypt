#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "TeaSafeQTreeVisitor.h"

#include "teasafe/EntryInfo.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/OpenDisposition.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeFolder.hpp"
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
    QObject::connect(this, SIGNAL(closeProgressSignal()), this,
                         SLOT(closeProgressSlot()));
    QObject::connect(this, SIGNAL(setMaximumProgressSignal(long)), this,
                         SLOT(setMaximumProgressSlot(long)));
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
    m_sd = boost::make_shared<QProgressDialog>("Building cipher...", "Cancel", 0, 0, this);
    m_sd->setWindowModality(Qt::WindowModal);

    // start loading of TeaSafe image
    m_loaderThread.setSharedIO(io);
    m_loaderThread.start();

    // execute the progress dialog
    m_sd->setMinimum(0);
    m_sd->setMaximum(0);
    m_sd->exec();
}

void MainWindow::updateProgressSlot()
{
    static long value(0);
    m_sd->setValue(value++);
}

void MainWindow::closeProgressSlot()
{
    m_sd->close();
    m_teaSafe = m_loaderThread.getTeaSafe();
    this->testItems();
}

void MainWindow::setMaximumProgressSlot(long value)
{
    m_sd->setMaximum(value);
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
        emit closeProgressSignal();
    }
}

void MainWindow::finishedLoadingSlot()
{
    qDebug() << "Finished loading!";
}


void MainWindow::testItems()
{

    std::string path("/");
    QTreeWidget *treeWidget = ui->fileTree;
    treeWidget->setColumnCount(1);
    TeaSafeQTreeVisitor visitor(treeWidget, path);
    teasafe::utility::recursiveExtract(visitor, *m_teaSafe, path);

/*
    teasafe::TeaSafeFolder currentFolder(m_teaSafe->getTeaSafeFolder("/"));
    typedef std::vector<teasafe::EntryInfo> EntryInfos;
    EntryInfos entryInfos(currentFolder.listAllEntries());
    EntryInfos::iterator it = entryInfos.begin();
    QTreeWidgetItem *parent = new QTreeWidgetItem(treeWidget);
    parent->setText(0, tr("/"));
    for (; it != entryInfos.end(); ++it) {
        QTreeWidgetItem *child = new QTreeWidgetItem(parent);
        child->setText(0, QString(it->filename().c_str()));
    }
    */

}
