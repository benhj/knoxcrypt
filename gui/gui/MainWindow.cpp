#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/OpenDisposition.hpp"
#include "teasafe/TeaSafe.hpp"

#include <boost/make_shared.hpp>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>

#include <fstream>
#include <vector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(ui->loadButton, SIGNAL(clicked()),
                     this, SLOT(loadFileButtonHandler()));

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

    // Obtain the initialization vector from the first 8 bytes
    // and the number of xtea rounds from the ninth byte
    {
        std::ifstream in(io->path.c_str(), std::ios::in | std::ios::binary);
        std::vector<uint8_t> ivBuffer;
        ivBuffer.resize(8);
        (void)in.read((char*)&ivBuffer.front(), teasafe::detail::IV_BYTES);
        char i;
        (void)in.read((char*)&i, 1);
        // note, i should always > 0 <= 255
        io->rounds = (unsigned int)i;
        in.close();
        io->iv = teasafe::detail::convertInt8ArrayToInt64(&ivBuffer.front());
    }

    // Obtain the number of blocks in the image by reading the image's block count
    teasafe::TeaSafeImageStream stream(io, std::ios::in | std::ios::binary);
    io->blocks = teasafe::detail::getBlockCount(stream);
    io->freeBlocks = io->blocks - teasafe::detail::getNumberOfAllocatedBlocks(stream);
    io->blockBuilder = boost::make_shared<teasafe::FileBlockBuilder>(io);
    stream.close();

    // Create the basic file system
    m_teaSafe = boost::make_shared<teasafe::TeaSafe>(io);
}

void MainWindow::testItems()
{
    QTreeWidget *treeWidget = ui->fileTree;
    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 10; ++i) {
        items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
    }
    treeWidget->insertTopLevelItems(0, items);
}
