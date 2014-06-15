#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QTreeWidgetItem>
#include <QDebug>

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
    qDebug() << "hello!";
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
