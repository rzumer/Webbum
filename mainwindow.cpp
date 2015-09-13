#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QtWidgets>
#include <QDebug>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    av_register_all();
}

MainWindow::~MainWindow()
{
   delete ui;
}

bool MainWindow::validateInputFile(QString &inputFilePath)
{
    // Checks whether the file chosen for input exists
    return QFile::exists(inputFilePath);
}

bool MainWindow::validateOutputFile(QString &outputFilePath)
{
    // Checks whether the directory chosen for output exists
    return QDir(QFileInfo(outputFilePath).dir()).exists();
}

void MainWindow::refreshTargetMode(QString &currentTargetMode)
{
    // Checks current target mode and enables/disables associated controls accordingly
    if(currentTargetMode == "Bit Rate")
    {
        ui->rateTargetBitRateSpinBox->setEnabled(true);
        ui->rateTargetFileSizeSpinBox->setEnabled(false);
    }
    else if(currentTargetMode == "File Size")
    {
        ui->rateTargetFileSizeSpinBox->setEnabled(true);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
    }
}

void MainWindow::initializeStreamComboBoxes(QString &inputFilePath)
{
    if(inputFilePath.trimmed().isEmpty()) return;
    AVFormatContext *pFormatCtx = NULL;
    if(avformat_open_input(&pFormatCtx,inputFilePath.toStdString().c_str(),NULL,NULL) == 0)
    {
        if(avformat_find_stream_info(pFormatCtx,NULL) >= 0)
        {
            for(int i = 0; (unsigned)i < pFormatCtx->nb_streams; i++)
            {
                AVStream * currentStream = pFormatCtx->streams[i];
                if(currentStream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
                {
                    ui->streamVideoComboBox->addItem(QString("[" + QString::number(i) + "] " +
                                                             QString::fromStdString(currentStream->codec->codec_descriptor->name) + " (" +
                                                             QString::number(av_q2d(currentStream->r_frame_rate)) + "fps)"));
                    //qDebug() << av_q2d(currentStream->r_frame_rate);
                    //qDebug() << av_q2d(currentStream->time_base);
                    //qDebug() << (unsigned long) (currentStream->duration * (av_q2d(currentStream->r_frame_rate) * av_q2d(currentStream->time_base)));
                }
                else if(currentStream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
                {
                    ui->streamAudioComboBox->addItem(QString("[" + QString::number(i) + "] " + QString::fromStdString(currentStream->codec->codec_descriptor->name)));
                }
                else if(currentStream->codec->codec_type==AVMEDIA_TYPE_SUBTITLE)
                {
                    ui->streamSubtitlesComboBox->addItem(QString("[" + QString::number(i) + "] " + QString::fromStdString(currentStream->codec->codec_descriptor->name)));
                }
            }
        }
    }
    if(ui->streamVideoComboBox->count() > 1)
    {
        ui->streamVideoComboBox->setEnabled(true); // enable the video stream combo box if video streams are found
        if(ui->streamVideoComboBox->currentIndex() == 0)
            ui->streamVideoComboBox->setCurrentIndex(1); // select the first video stream instead of leaving it disabled
    }
    if(ui->streamAudioComboBox->count() > 1)
    {
        ui->streamAudioComboBox->setEnabled(true);
        if(ui->streamAudioComboBox->currentIndex() == 0)
            ui->streamAudioComboBox->setCurrentIndex(1);
    }
    if(ui->streamSubtitlesComboBox->count() > 1)
    {
        ui->streamSubtitlesComboBox->setEnabled(true);
        if(ui->streamSubtitlesComboBox->currentIndex() == 0)
            ui->streamSubtitlesComboBox->setCurrentIndex(1);
    }
}

void MainWindow::clearStreamComboBoxes()
{
    // set selected streams to Disabled
    ui->streamVideoComboBox->setCurrentIndex(0);
    ui->streamAudioComboBox->setCurrentIndex(0);
    ui->streamSubtitlesComboBox->setCurrentIndex(0);

    // disable combo boxes
    ui->streamVideoComboBox->setEnabled(false);
    ui->streamAudioComboBox->setEnabled(false);
    ui->streamSubtitlesComboBox->setEnabled(false);

    // clear combo box items
    for(int i = ui->streamVideoComboBox->count() - 1; i > 0; i--)
    {
        ui->streamVideoComboBox->removeItem(i);
    }
    for(int i = ui->streamAudioComboBox->count() - 1; i > 0; i--)
    {
        ui->streamAudioComboBox->removeItem(i);
    }
    for(int i = ui->streamSubtitlesComboBox->count() - 1; i > 0; i--)
    {
        ui->streamSubtitlesComboBox->removeItem(i);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    // About dialog box
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

void MainWindow::on_inputFileLineEdit_textChanged(const QString &arg1)
{
    clearStreamComboBoxes();
    ui->encodePushButton->setEnabled(false);

    QString inputFilePath = arg1.trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();

    if(validateInputFile(inputFilePath))
    {
        initializeStreamComboBoxes(inputFilePath);
        if(validateOutputFile(outputFilePath))
        {
            if(ui->streamVideoComboBox->currentIndex() > 0)
                ui->encodePushButton->setEnabled(true);
        }
    }

}

void MainWindow::on_outputFileLineEdit_textChanged(const QString &arg1)
{
    ui->encodePushButton->setEnabled(false);

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = arg1.trimmed();

    if(validateInputFile(inputFilePath) && validateOutputFile(outputFilePath))
    {
        if(ui->streamVideoComboBox->currentIndex() > 0)
            ui->encodePushButton->setEnabled(true);
    }
}

void MainWindow::on_inputFileBrowsePushButton_clicked()
{
    QString inputFilePath = QFileDialog::getOpenFileName(this,"Select Input File",
                                 QFileInfo(ui->inputFileLineEdit->text().trimmed()).dir().canonicalPath(),
                                 "All Files (*.*)");
    if(!inputFilePath.isEmpty())
        ui->inputFileLineEdit->setText(QDir::toNativeSeparators(inputFilePath));
}

void MainWindow::on_outputFileBrowsePushButton_clicked()
{
    QString outputFilePath = QFileDialog::getSaveFileName(this,"Select Output File",
                                QFileInfo(ui->outputFileLineEdit->text().trimmed()).dir().canonicalPath(),
                                "WebM (*.webm)");
    if(!outputFilePath.isEmpty())
        ui->outputFileLineEdit->setText(QDir::toNativeSeparators(outputFilePath));
}

void MainWindow::on_rateModeComboBox_currentIndexChanged(const QString &arg1)
{
    QString currentMode = arg1;

    if(currentMode == "VBR")
    {
        // Disable CRF selection, enable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(false);

        QComboBox * targetModeComboBox = ui->rateTargetModeComboBox;
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(targetModeComboBox->currentText());
    }
    else if(currentMode == "CBR")
    {
        // Disable CRF selection, enable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(false);

        QComboBox * targetModeComboBox = ui->rateTargetModeComboBox;
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(targetModeComboBox->currentText());
    }
    else if(currentMode == "CRF")
    {
        // Enable CRF selection, disable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(true);
        ui->rateTargetModeComboBox->setEnabled(false);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
        ui->rateTargetFileSizeSpinBox->setEnabled(false);
    }
    else
    {
        // Invalid mode selected, disable all controls
        ui->rateCRFSpinBox->setEnabled(false);
        ui->rateTargetModeComboBox->setEnabled(false);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
        ui->rateTargetFileSizeSpinBox->setEnabled(false);
    }
}

void MainWindow::on_rateTargetModeComboBox_currentIndexChanged(const QString &arg1)
{
    QString currentTargetMode = arg1;

    refreshTargetMode(currentTargetMode);
}

void MainWindow::on_streamVideoComboBox_currentIndexChanged(int index)
{
    // disable encode button if no video stream is selected
    if(index == 0)
    {
        ui->encodePushButton->setEnabled(false);
    }
}
