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
    QMessageBox::about(this,"About","Webbum makes WebMs.");
}

void MainWindow::on_resizeCheckBox_toggled(bool checked)
{
    if(checked)
    {
        ui->resizeWidthSpinBox->setEnabled(true);
        ui->resizeHeightSpinBox->setEnabled(true);
    }
    else
    {
        ui->resizeWidthSpinBox->setEnabled(false);
        ui->resizeHeightSpinBox->setEnabled(false);
    }
}

void MainWindow::on_cropCheckBox_toggled(bool checked)
{
    if(checked)
    {
        ui->cropLeftSpinBox->setEnabled(true);
        ui->cropRightSpinBox->setEnabled(true);
        ui->cropTopSpinBox->setEnabled(true);
        ui->cropBottomSpinBox->setEnabled(true);
    }
    else
    {
        ui->cropLeftSpinBox->setEnabled(false);
        ui->cropRightSpinBox->setEnabled(false);
        ui->cropTopSpinBox->setEnabled(false);
        ui->cropBottomSpinBox->setEnabled(false);
    }
}

void MainWindow::on_trimStartEndRadioButton_toggled(bool checked)
{
    if(checked)
    {
        ui->trimStartEndStartTimeEdit->setEnabled(true);
        ui->trimStartEndEndTimeEdit->setEnabled(true);
    }
    else
    {
        ui->trimStartEndStartTimeEdit->setEnabled(false);
        ui->trimStartEndEndTimeEdit->setEnabled(false);
    }
}

void MainWindow::on_trimDurationRadioButton_toggled(bool checked)
{
    if(checked)
    {
        ui->trimDurationStartTimeEdit->setEnabled(true);
        ui->trimDurationDurationTimeEdit->setEnabled(true);
    }
    else
    {
        ui->trimDurationStartTimeEdit->setEnabled(false);
        ui->trimDurationDurationTimeEdit->setEnabled(false);
    }
}
