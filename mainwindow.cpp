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
    connect(ui->inputFileBrowsePushButton,SIGNAL(clicked(bool)),ui->actionOpen,SLOT(trigger()));
    setAcceptDrops(true);
    av_register_all();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    // use the first item in a drag&drop event as input file
    if(!ev->mimeData()->urls().isEmpty())
    {
        QUrl url = ev->mimeData()->urls().first();
        if(url.isLocalFile())
        {
            ui->inputFileLineEdit->setText(
                QDir::toNativeSeparators(
                    QFileInfo(url.toLocalFile()).canonicalFilePath()));
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->accept();
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
    // uncomment below to dump stream information to stderr
    //const char *inputFileName = ui->inputFileLineEdit->text().trimmed().toStdString().c_str();
    //av_dump_format(formatContext,0,inputFileName,false);

    // add audio, video and subtitle streams to their respective combo boxes
    for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
    {
        AVStream *currentStream = formatContext->streams[i];
        AVDictionaryEntry *lang = av_dict_get(currentStream->metadata,"language",NULL,0);

        if(currentStream->codec->codec_descriptor != NULL)
        {
            QString streamStr = "[" + QString::number(i) + "] ";

            // add stream title if available
            AVDictionaryEntry *title = av_dict_get(currentStream->metadata,"title",NULL,0);
            if(title)
                streamStr.append("\"" + QString::fromStdString(title->value) + "\" - ");

            // add codec name
            streamStr.append(QString::fromStdString(avcodec_get_name(currentStream->codec->codec_id)));

            // add profile name if available
            if(currentStream->codec->profile != FF_PROFILE_UNKNOWN)
            {
                const AVCodec *profile;
                const char *profileName;

                if(currentStream->codec->codec)
                    profile = currentStream->codec->codec;
                else
                    profile = avcodec_find_decoder(currentStream->codec->codec_id);

                if(profile)
                    profileName = av_get_profile_name(profile,currentStream->codec->profile);

                if(profileName)
                    streamStr.append("/" + QString::fromStdString(profileName) + "");
            }

            // add bitrate and channels to audio streams
            if(currentStream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
            {
                streamStr.append(" (");

                int bitsPerSample = av_get_bits_per_sample(currentStream->codec->codec_id);
                int bitRate = bitsPerSample ? currentStream->codec->sample_rate *
                                              currentStream->codec->channels *
                                              bitsPerSample : currentStream->codec->bit_rate;
                if(bitRate != 0)
                    streamStr.append(QString::number((double)bitRate / 1000) + "kbps/");

                char buf[256];
                av_get_channel_layout_string(buf,sizeof(buf),
                    currentStream->codec->channels,currentStream->codec->channel_layout);
                streamStr.append(QString::fromStdString(buf) + ")");
            }

            if(lang)
                streamStr.append(" (" + QString::fromStdString(lang->value) + ")");
            /*if(currentStream->disposition & AV_DISPOSITION_DEFAULT)
                streamStr.append(" [default]");*/

            if(currentStream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                ui->streamVideoComboBox->addItem(streamStr);
            }
            else if(currentStream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
            {
                ui->streamAudioComboBox->addItem(streamStr);
            }
            else if(currentStream->codec->codec_type==AVMEDIA_TYPE_SUBTITLE)
            {
                ui->streamSubtitlesComboBox->addItem(streamStr);
            }
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

    // populate chapter combo boxes
    for(int i = 0; (unsigned)i < formatContext->nb_chapters; i++)
    {
        AVChapter *currentChapter = formatContext->chapters[i];
        AVDictionaryEntry *title = av_dict_get(currentChapter->metadata,"title",NULL,0);

        QString currentChapterText = "[" + QString::number(i) + "] ";
        if(title)
            currentChapterText.append(QString::fromStdString(title->value)); // use chapter title if it exists

        ui->trimStartEndStartChapterComboBox->addItem(currentChapterText.trimmed());
        ui->trimStartEndEndChapterComboBox->addItem(currentChapterText.trimmed());
    }
    int chapterCount = ui->trimStartEndStartChapterComboBox->count() - 1;
    if(chapterCount > 0)
    {
        ui->trimStartEndStartChapterComboBox->setCurrentIndex(1);
        ui->trimStartEndEndChapterComboBox->setCurrentIndex(chapterCount);

        if(ui->trimStartEndRadioButton->isChecked())
        {
            ui->trimStartEndStartChapterComboBox->setEnabled(true);
            ui->trimStartEndEndChapterComboBox->setEnabled(true);
        }
    }
}

void MainWindow::clearInputFileFormData()
{
    // clear generated form fields
    ui->resizeWidthSpinBox->setValue(0);
    ui->resizeHeightSpinBox->setValue(0);
    ui->trimStartEndStartTimeEdit->setTime(QTime(0,0));
    ui->trimStartEndEndTimeEdit->setTime(QTime(0,0));
    ui->trimDurationStartTimeEdit->setTime(QTime(0,0));
    ui->trimDurationDurationTimeEdit->setTime(QTime(0,0));
    ui->rateTargetBitRateSpinBox->setValue(0);
    ui->rateTargetFileSizeDoubleSpinBox->setValue(0);

    // restore maximum time on time edits
    ui->trimStartEndStartTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimStartEndEndTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimDurationStartTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimDurationDurationTimeEdit->setMaximumTime(QTime(23,59,59,999));

    // add "No Chapter" item to chapter combo boxes in case it was removed
    ui->trimStartEndStartChapterComboBox->insertItem(0,"No Chapter");
    ui->trimStartEndEndChapterComboBox->insertItem(0,"No Chapter");

    // set selected streams/chapters to Disabled/No Chapter
    ui->streamVideoComboBox->setCurrentIndex(0);
    ui->streamAudioComboBox->setCurrentIndex(0);
    ui->streamSubtitlesComboBox->setCurrentIndex(0);
    ui->trimStartEndStartChapterComboBox->setCurrentIndex(0);
    ui->trimStartEndEndChapterComboBox->setCurrentIndex(0);

    // disable combo boxes
    ui->streamVideoComboBox->setEnabled(false);
    ui->streamAudioComboBox->setEnabled(false);
    ui->streamSubtitlesComboBox->setEnabled(false);
    ui->trimStartEndStartChapterComboBox->setEnabled(false);
    ui->trimStartEndEndChapterComboBox->setEnabled(false);

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
    for(int i = ui->trimStartEndStartChapterComboBox->count() - 1; i > 0; i--)
    {
        ui->trimStartEndStartChapterComboBox->removeItem(i);
    }
    for(int i = ui->trimStartEndEndChapterComboBox->count() - 1; i > 0; i--)
    {
        ui->trimStartEndEndChapterComboBox->removeItem(i);
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
    // duration is rounded for ms accuracy
    QTime duration = QTime(0,0).addMSecs((double)(formatContext->duration + 500) / AV_TIME_BASE * 1000);
    ui->trimDurationDurationTimeEdit->setTime(duration);
    ui->trimStartEndEndTimeEdit->setTime(duration);

    // set maximum time to the video duration
    ui->trimStartEndStartTimeEdit->setMaximumTime(duration);
    ui->trimStartEndEndTimeEdit->setMaximumTime(duration);
    ui->trimDurationStartTimeEdit->setMaximumTime(duration);
    ui->trimDurationDurationTimeEdit->setMaximumTime(duration);

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

    // set default target bitrate and file size based on the container's
    int bitRate = (formatContext->bit_rate + 500) / 1000; // in kilobits = 1000 bits
    ui->rateTargetBitRateSpinBox->setValue(bitRate);
    double fileSize = calculateFileSize(bitRate, duration); // in megabytes = 1024 kilobytes
    ui->rateTargetFileSizeDoubleSpinBox->setValue(fileSize);
}

// bitRate is in KILOBITS per second
// file size returned is in MEGABYTES
// this could be remade for better usability
double MainWindow::calculateFileSize(int bitRate, QTime duration)
{
    return bitRate * (((double)QTime(0,0).msecsTo(duration)) / 1024) / 1024 / 8;
}

QStringList &MainWindow::generatePass(int passNumber,QString &inputFilePath,
                                 QString &outputFilePath,int videoStreamId,
                                 int audioStreamId, int subtitleStreamId,
                                 QTime startTime,QTime endTime,QTime duration,
                                 int cropLeft,int cropRight,int cropTop,
                                 int cropBottom,int width,int height,double crf,
                                 double targetFileSize,double targetBitRate,
                                 bool cbr,QString customParameters)
{
    //QString passString = "-i " + inputFilePath +
    return QStringList();
}

void MainWindow::encodePass(QStringList &encodingParameters)
{
    //Code crashes currently.
    /*QProcess ffmpegProcess(this);
    QString processName = "ffmpeg";

    ffmpegProcess.start(processName);

    ffmpegProcess.waitForFinished();

    if(ffmpegProcess.exitCode() != 0)
    {
        QMessageBox::warning(this,"Warning","ffmpeg returned an exit code of " +
                             QString::number(ffmpegProcess.exitCode()) +
                             ". There may have been errors.",QMessageBox::Ok);
    }
    qDebug() << "ffmpeg output: " << ffmpegProcess.readAll();*/
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
        ui->rateCRFDoubleSpinBox->setEnabled(false);

        QComboBox * targetModeComboBox = ui->rateTargetModeComboBox;
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(targetModeComboBox->currentText());
    }
    else if(currentMode == "CBR")
    {
        // Disable CRF selection, enable target mode/bit rate/file size selection
        ui->rateCRFDoubleSpinBox->setEnabled(false);

        QComboBox * targetModeComboBox = ui->rateTargetModeComboBox;
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(targetModeComboBox->currentText());
    }
    else if(currentMode == "CRF")
    {
        // Enable CRF selection, disable target mode/bit rate/file size selection
        ui->rateCRFDoubleSpinBox->setEnabled(true);
        ui->rateTargetModeComboBox->setEnabled(false);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
    }
    else
    {
        // Invalid mode selected, disable all controls
        ui->rateCRFDoubleSpinBox->setEnabled(false);
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
    // disable encode button and reset resolution fields if no video stream is selected
    if(index == 0)
    {
        ui->encodePushButton->setEnabled(false);
        ui->resizeWidthSpinBox->setValue(0);
        ui->resizeHeightSpinBox->setValue(0);
    }
    // change resolution fields based on the selected stream
    else
    {
        AVFormatContext *formatContext = openInputFile(ui->inputFileLineEdit->text().trimmed());

        int videoStreamIndex = 0;
        for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
        {
            AVStream *currentStream = formatContext->streams[i];
            if(currentStream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                videoStreamIndex++;
                if(videoStreamIndex == index)
                {
                    ui->resizeWidthSpinBox->setValue(currentStream->codec->width);
                    ui->resizeHeightSpinBox->setValue(currentStream->codec->height);
                }
            }
        }

        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimStartEndRadioButton_toggled(bool checked)
{
    if(checked && ui->trimStartEndStartChapterComboBox->count() > 1)
    {
        ui->trimStartEndStartChapterComboBox->setEnabled(true);
        ui->trimStartEndEndChapterComboBox->setEnabled(true);
    }
    else
    {
        ui->trimStartEndStartChapterComboBox->setEnabled(false);
        ui->trimStartEndEndChapterComboBox->setEnabled(false);
    }
}

void MainWindow::on_trimStartEndStartChapterComboBox_activated(int index)
{
    if(index > 0)
    {
        int endChapterIndex = ui->trimStartEndEndChapterComboBox->currentIndex();
        AVFormatContext *formatContext = openInputFile(ui->inputFileLineEdit->text().trimmed());
        AVChapter *currentChapter = formatContext->chapters[index - 1];
        ui->trimStartEndStartTimeEdit->setTime(
            QTime(0,0).addMSecs(((double)currentChapter->start + 500) * av_q2d(currentChapter->time_base) * 1000));
        if(index > endChapterIndex && endChapterIndex > 0)
        {
            ui->trimStartEndEndChapterComboBox->setCurrentIndex(index);
            ui->trimStartEndEndTimeEdit->setTime(
                QTime(0,0).addMSecs(((double)currentChapter->end + 500) * av_q2d(currentChapter->time_base) * 1000));
        }
        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimStartEndEndChapterComboBox_activated(int index)
{
    if(index > 0)
    {
        int startChapterIndex = ui->trimStartEndStartChapterComboBox->currentIndex();
        AVFormatContext *formatContext = openInputFile(ui->inputFileLineEdit->text().trimmed());
        AVChapter *currentChapter = formatContext->chapters[index - 1];
        ui->trimStartEndEndTimeEdit->setTime(
            QTime(0,0).addMSecs(((double)currentChapter->end + 500) * av_q2d(currentChapter->time_base) * 1000));
        if(index < startChapterIndex)
        {
            ui->trimStartEndStartChapterComboBox->setCurrentIndex(index);
            ui->trimStartEndStartTimeEdit->setTime(
                QTime(0,0).addMSecs(((double)currentChapter->start + 500) * av_q2d(currentChapter->time_base) * 1000));
        }
        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimStartEndStartTimeEdit_editingFinished()
{
    ui->trimStartEndStartChapterComboBox->setCurrentIndex(0);
    QTime startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();
    if(startTime > endTime)
        ui->trimStartEndEndTimeEdit->setTime(startTime);
}

void MainWindow::on_trimStartEndEndTimeEdit_editingFinished()
{
    ui->trimStartEndEndChapterComboBox->setCurrentIndex(0);
    QTime startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();
    if(endTime < startTime)
        ui->trimStartEndStartTimeEdit->setTime(endTime);
}

void MainWindow::on_cropLeftSpinBox_editingFinished()
{
    int value = ui->cropLeftSpinBox->value();
    if(value % 2 != 0)
        ui->cropLeftSpinBox->setValue(value - 1);
}

void MainWindow::on_cropRightSpinBox_editingFinished()
{
    int value = ui->cropRightSpinBox->value();
    if(value % 2 != 0)
        ui->cropRightSpinBox->setValue(value - 1);
}

void MainWindow::on_cropTopSpinBox_editingFinished()
{
    int value = ui->cropTopSpinBox->value();
    if(value % 2 != 0)
        ui->cropTopSpinBox->setValue(value - 1);
}

void MainWindow::on_cropBottomSpinBox_editingFinished()
{
    int value = ui->cropBottomSpinBox->value();
    if(value % 2 != 0)
        ui->cropBottomSpinBox->setValue(value - 1);
}

void MainWindow::on_resizeWidthSpinBox_editingFinished()
{
    int value = ui->resizeWidthSpinBox->value();
    if(value % 2 != 0)
        ui->resizeWidthSpinBox->setValue(value - 1);
}

void MainWindow::on_resizeHeightSpinBox_editingFinished()
{
    int value = ui->resizeHeightSpinBox->value();
    if(value % 2 != 0)
        ui->resizeHeightSpinBox->setValue(value - 1);
}

void MainWindow::on_actionOpen_triggered()
{
    QString inputFilePath = QFileDialog::getOpenFileName(this,"Select Input File",
                                 QFileInfo(ui->inputFileLineEdit->text().trimmed()).dir().canonicalPath(),
                                 "All Files (*.*)");
    if(!inputFilePath.isEmpty())
        ui->inputFileLineEdit->setText(QDir::toNativeSeparators(inputFilePath));
}

void MainWindow::on_encodePushButton_clicked()
{
    this->setEnabled(false);

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();

    // get stream IDs
    int videoStreamId, audioStreamId, subtitleStreamId;
    AVFormatContext *formatContext = openInputFile(inputFilePath);
    int videoStreamIndex = 0;
    int audioStreamIndex = 0;
    int subtitleStreamIndex = 0;
    for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
    {
        AVStream *currentStream = formatContext->streams[i];
        if(currentStream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex++;
            if(videoStreamIndex == ui->streamVideoComboBox->currentIndex())
                videoStreamId = i;
        }
        else if(currentStream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex++;
            if(audioStreamIndex == ui->streamAudioComboBox->currentIndex())
                audioStreamId = i;
        }
        else if(currentStream->codec->codec_type==AVMEDIA_TYPE_SUBTITLE)
        {
            subtitleStreamIndex++;
            if(subtitleStreamIndex == ui->streamSubtitlesComboBox->currentIndex())
                subtitleStreamId = i;
        }
    }
    closeInputFile(formatContext);

    QTime startTime, endTime, duration;
    if(ui->trimStartEndRadioButton->isChecked())
    {
        startTime = ui->trimStartEndStartTimeEdit->time();
        endTime = ui->trimStartEndEndTimeEdit->time();
    }
    else if(ui->trimDurationRadioButton->isChecked())
    {
        startTime = ui->trimDurationStartTimeEdit->time();
        duration = ui->trimDurationDurationTimeEdit->time();
    }

    int cropLeft, cropRight, cropTop, cropBottom, width, height;
    if(ui->cropCheckBox->isChecked())
    {
        cropLeft = ui->cropLeftSpinBox->value();
        cropRight = ui->cropRightSpinBox->value();
        cropTop = ui->cropTopSpinBox->value();
        cropBottom = ui->cropBottomSpinBox->value();
    }
    if(ui->resizeCheckBox->isChecked())
    {
        width = ui->resizeWidthSpinBox->value();
        height = ui->resizeHeightSpinBox->value();
        if(width == 0) width = -1;
        if(height == 0) height = -1;
    }

    double crf = -1;
    double targetFileSize;
    int targetBitRate;
    bool cbr;
    QString rateMode = ui->rateModeComboBox->currentText();
    if(rateMode == "CBR")
    {
        cbr = true;
    }
    if(rateMode == "CBR" || rateMode == "VBR")
    {
        if(ui->rateTargetModeComboBox->currentText() == "File Size")
        {
            targetFileSize = ui->rateTargetFileSizeDoubleSpinBox->value();
        }
        else if(ui->rateTargetModeComboBox->currentText() == "Bit Rate")
        {
            targetBitRate = ui->rateTargetBitRateSpinBox->value();
        }
    }
    else if(rateMode == "CRF")
    {
        crf = ui->rateCRFDoubleSpinBox->value();
    }

    QString customParameters = ui->customEncoderParametersLineEdit->text().trimmed();

    QStringList firstPass,secondPass;

    firstPass = generatePass(1,inputFilePath,outputFilePath,videoStreamId,
                                    audioStreamId,subtitleStreamId,startTime,
                                    endTime,duration,cropLeft,cropRight,cropTop,
                                    cropBottom,width,height,crf,targetFileSize,
                                    targetBitRate,cbr,customParameters);

    if(crf == -1)
    {
        secondPass = generatePass(2,inputFilePath,outputFilePath,videoStreamId,
                                        audioStreamId,subtitleStreamId,startTime,
                                        endTime,duration,cropLeft,cropRight,cropTop,
                                        cropBottom,width,height,crf,targetFileSize,
                                        targetBitRate,cbr,customParameters);
    }

    ui->progressBar->setValue(10);

    if(secondPass.isEmpty())
    {
        encodePass(firstPass);
    }
    else
    {
        QDir outputDirectory = QFileInfo(outputFilePath).dir();
        QDir tempDirectory = outputDirectory.canonicalPath() + "/temp";

        encodePass(firstPass);
        ui->progressBar->setValue(50);
        encodePass(secondPass);

        if(tempDirectory.exists())
            tempDirectory.removeRecursively();
    }
    ui->progressBar->setValue(100);
    QMessageBox::information(this,"Success","Encode successful.",
                             QMessageBox::Ok);

    ui->progressBar->setValue(0);
    this->setEnabled(true);
}
