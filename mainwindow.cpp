#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QtWidgets>
#include <QDebug>

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
    return QFileInfo(outputFilePath).dir().exists() &&
            !QFileInfo(outputFilePath).baseName().isEmpty();
}

void MainWindow::refreshTargetMode(QString &currentTargetMode)
{
    // Checks current target mode and enables/disables associated controls accordingly
    if(currentTargetMode == "Bit Rate")
    {
        ui->rateTargetBitRateSpinBox->setEnabled(true);
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
    }
    else if(currentTargetMode == "File Size")
    {
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(true);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
    }
}

AVFormatContext *MainWindow::openInputFile(QString &inputFilePath)
{
    AVFormatContext *formatContext = NULL;
    if(avformat_open_input(&formatContext,inputFilePath.toStdString().c_str(),NULL,NULL) == 0)
    {
        if(avformat_find_stream_info(formatContext,NULL) >= 0)
        {
            return formatContext;
        }
    }
    // close the file and return NULL if the input file has no streams to be found
    avformat_close_input(&formatContext);
    return NULL;
}

void MainWindow::closeInputFile(AVFormatContext *formatContext)
{
    avformat_close_input(&formatContext);
}

void MainWindow::processInputFile(QString &inputFilePath)
{
    AVFormatContext *formatContext = openInputFile(inputFilePath);

    if(formatContext != NULL)
    {
        populateStreamComboBoxes(formatContext);
        initializeFormData(formatContext);
        closeInputFile(formatContext);
    }
}

void MainWindow::populateStreamComboBoxes(AVFormatContext *formatContext)
{
    // add audio, video and subtitle streams to their respective combo boxes
    for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
    {
        AVStream *currentStream = formatContext->streams[i];
        if(currentStream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            ui->streamVideoComboBox->addItem(QString("[" + QString::number(i) + "] " +
                QString::fromStdString(currentStream->codec->codec_descriptor->name) + " (" +
                QString::number(av_q2d(currentStream->r_frame_rate)) + "fps)"));
            //qDebug() << av_q2d(currentStream->r_frame_rate);
            //qDebug() << av_q2d(currentStream->time_base);
            //duration of the container
            //qDebug() << formatContext->duration / AV_TIME_BASE;
            //qDebug() << currentStream->codec->bits_per_raw_sample;
            //qDebug() << currentStream->codec->sample_fmt;
        }
        else if(currentStream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            ui->streamAudioComboBox->addItem(QString("[" + QString::number(i) + "] " +
                QString::fromStdString(currentStream->codec->codec_descriptor->name)));
        }
        else if(currentStream->codec->codec_type==AVMEDIA_TYPE_SUBTITLE)
        {
            ui->streamSubtitlesComboBox->addItem(QString("[" + QString::number(i) + "] " +
                QString::fromStdString(currentStream->codec->codec_descriptor->name)));
        }
    }

    // enable combo boxes if at least one stream of that type is found,
    // then automatically select the first one
    if(ui->streamVideoComboBox->count() > 1)
    {
        ui->streamVideoComboBox->setEnabled(true);
        if(ui->streamVideoComboBox->currentIndex() == 0)
            ui->streamVideoComboBox->setCurrentIndex(1);
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

void MainWindow::clearInputFileFormData()
{
    // clear generated form fields
    ui->resizeWidthSpinBox->setValue(0);
    ui->resizeHeightSpinBox->setValue(0);
    ui->trimDurationDurationTimeEdit->setTime(QTime(0,0));
    ui->trimStartEndEndTimeEdit->setTime(QTime(0,0));

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

void MainWindow::initializeFormData(AVFormatContext *formatContext)
{
    if(!validateOutputFile(ui->outputFileLineEdit->text().trimmed()))
    {
        // set output file name based on the input's
        QFileInfo inputFile = QFileInfo(ui->inputFileLineEdit->text().trimmed());
        ui->outputFileLineEdit->setText(QDir::toNativeSeparators(inputFile.dir().canonicalPath() + "/") +
                                    inputFile.completeBaseName() + ".webm");
    }

    // set default end time and duration based on the container's
    QTime duration = QTime(0,0).addMSecs((double)formatContext->duration / AV_TIME_BASE * 1000);
    ui->trimDurationDurationTimeEdit->setTime(duration);
    ui->trimStartEndEndTimeEdit->setTime(duration);

    // set default width and height based on the first video stream's
    for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
    {
        AVStream *currentStream = formatContext->streams[i];
        if(currentStream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            ui->resizeWidthSpinBox->setValue(currentStream->codec->width);
            ui->resizeHeightSpinBox->setValue(currentStream->codec->height);
            break;
        }
    }
}

void MainWindow::on_actionAbout_triggered()
{
    // About dialog box
    QMessageBox::about(this,"About","Webbum makes WebMs.");
}

void MainWindow::on_inputFileLineEdit_textChanged(const QString &arg1)
{
    clearInputFileFormData();
    ui->encodePushButton->setEnabled(false);

    QString inputFilePath = arg1.trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();

    if(validateInputFile(inputFilePath))
    {
        processInputFile(inputFilePath);
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
                                ui->outputFileLineEdit->text().trimmed(),
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
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
    }
    else
    {
        // Invalid mode selected, disable all controls
        ui->rateCRFSpinBox->setEnabled(false);
        ui->rateTargetModeComboBox->setEnabled(false);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
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
