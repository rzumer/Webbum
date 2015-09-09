#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
   delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    //QMessageBox::about(this,"About Webbum","This is Webbum.");
    QMessageBox::aboutQt(this,"About Qt");
}
