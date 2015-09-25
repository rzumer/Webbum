  #include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // remove vp9 rate control options until it is supported
    ui->rateCRFSpinBox->setMinimum(4);
    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText("Constant Quality"));
    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText("Lossless"));

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
            ui->inputFileLineEdit->setText(QDir::toNativeSeparators(
                    QFileInfo(url.toLocalFile()).canonicalFilePath()));
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->accept();
}

void MainWindow::connectSignalsAndSlots(InputFile &inputFile)
{
    // do stuff
}

bool MainWindow::validateInputFile(QString &inputFilePath)
{
    // Checks whether the file chosen for input exists
    return QFile::exists(inputFilePath);
}

bool MainWindow::validateOutputFile(QString &outputFilePath)
{
    // Checks whether the directory chosen for output exists
    return !QFileInfo(outputFilePath).exists() &&
            QFileInfo(outputFilePath).dir().exists() &&
            !QFileInfo(outputFilePath).baseName().isEmpty();
}

bool MainWindow::validateFormFields()
{
    // Checks whether form input will produce a valid encode
    if(ui->streamVideoComboBox->currentIndex() == 0)
        return false;
    if(ui->rateTargetModeComboBox->isEnabled())
    {
        if(ui->rateTargetBitRateSpinBox->isEnabled() && ui->rateTargetBitRateSpinBox->value() == 0)
            return false;
        if(ui->rateTargetFileSizeDoubleSpinBox->isEnabled() && ui->rateTargetFileSizeDoubleSpinBox->value() == 0)
            return false;
    }
    if(ui->trimStartEndRadioButton->isChecked() && QTime(0,0).msecsTo(ui->trimStartEndEndTimeEdit->time()) == 0)
        return false;
    if(ui->trimDurationRadioButton->isChecked() && QTime(0,0).msecsTo(ui->trimDurationDurationTimeEdit->time()) == 0)
        return false;
    return true;
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
    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();
    //if(!validateOutputFile(outputFilePath))
    {
        // set output file name based on the input's
        QFileInfo inputFile = QFileInfo(inputFilePath);
        outputFilePath = QDir::toNativeSeparators(inputFile.dir().canonicalPath() + "/") +
                inputFile.completeBaseName() + ".webm";
        QFileInfo outputFile = QFileInfo(outputFilePath);

        while(outputFile.exists())
        {
            outputFilePath.replace(".webm","_out.webm");
            outputFile = QFileInfo(outputFilePath);
        }
        ui->outputFileLineEdit->setText(outputFilePath);
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
// this could be rewritten for better usability
double MainWindow::calculateFileSize(int bitRate, QTime duration)
{
    return (double)bitRate / 1000 / 8 * (QTime(0,0).msecsTo(duration)) / 1000;
}

// inverse of calculateFileSize
// file size is in MEGABYTES
// bitRate returned is in KILOBITS per second
// this could be rewritten for better usability
int MainWindow::calculateBitRate(double fileSize, QTime duration)
{
    // not sure about that rounding
    return (fileSize + 0.0005) * 1000 * 8 / QTime(0,0).msecsTo(duration) * 1000;
}

QTime MainWindow::getOutputDuration(int64_t inputDuration)
{
    QTime duration = ui->trimDurationDurationTimeEdit->time();
    QTime startTime;
    if(QTime(0,0).msecsTo(ui->trimStartEndStartTimeEdit->time()) == 0)
        startTime = ui->trimDurationStartTimeEdit->time();
    else
        startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();

    int durationInMSecs = QTime(0,0).msecsTo(duration);
    QTime computedDuration = QTime(0,0).addMSecs(durationInMSecs);

    if(!ui->trimDurationRadioButton->isChecked()) // if trimmed by duration, use the current value
    {
        if(ui->trimStartEndRadioButton->isChecked()) // if trimmed by start/end time, duration is end - start
            computedDuration = QTime(0,0).addMSecs(startTime.msecsTo(endTime));
        else // else, duration is the entire video (using container duration)
            computedDuration = QTime(0,0).addMSecs((double)(inputDuration + 500) / AV_TIME_BASE * 1000);
    }
    return computedDuration;
}

QStringList MainWindow::generatePass(int passNumber,QString &inputFilePath,
                                 QString &outputFilePath,int videoStreamId,
                                 int audioStreamId, int subtitleStreamId,
                                 QTime startTime,QTime endTime,QTime duration,
                                 int cropLeft,int cropRight,int cropTop,
                                 int cropBottom,int width,int height,int crf,
                                 double targetFileSize,double targetBitRate,
                                 bool cbr,QString customParameters,bool twoPass)
{
    QStringList passStringList = QStringList();

    // open file to get some data
    AVFormatContext *formatContext = openInputFile(inputFilePath);

    // get stream information
    AVStream *videoStream, *audioStream, *subtitleStream;
    videoStream = formatContext->streams[videoStreamId];
    if(audioStreamId > -1) audioStream = formatContext->streams[audioStreamId];
    if(subtitleStreamId > -1) subtitleStream = formatContext->streams[subtitleStreamId];

    // calculate target bitrate and cropping if needed
    QTime computedDuration = getOutputDuration(formatContext->duration);
    double bitRate = targetFileSize > -1 ? calculateBitRate(targetFileSize,computedDuration) : targetBitRate;
    if(audioStreamId > -1 && bitRate > 64)
        bitRate -= 64;

    int cropWidth = videoStream->codec->width - cropLeft - cropRight;
    int cropHeight = videoStream->codec->height - cropTop - cropBottom;
    int cropX = cropLeft;
    int cropY = cropTop;

    // lossless shortcut
    bool lossless = bitRate == -1 && crf == -1;

    // input
    passStringList << "-i" << inputFilePath;

    // trimming
    if(startTime.isValid())
    {
        passStringList << "-ss" << startTime.toString("hh:mm:ss.zzz");
    }

    if(endTime.isValid())
    {
        passStringList << "-t" << computedDuration.toString("hh:mm:ss.zzz");
    }
    else if(duration.isValid())
    {
        passStringList << "-t" << duration.toString("hh:mm:ss.zzz");
    }

    // codec (VP9)
    //passStringList << "-c:v:0." + QString::number(videoStreamId) << "libvpx-vp9";

    // codec (VP8)
    passStringList << "-c:v:0." + QString::number(videoStreamId) << "libvpx";

    // pass number
    if(twoPass) passStringList << "-pass" << QString::number(passNumber);

    // lossless
    if(lossless)
    {
        passStringList << "-lossless" << QString::number(1);
    }

    // cbr
    if(cbr && bitRate > 0)
    {
        passStringList << "-minrate" << QString::number(bitRate).append("K");
        passStringList << "-maxrate" << QString::number(bitRate).append("K");
    }

    // crf
    if(crf > -1)
    {
        passStringList << "-crf" << QString::number(crf);
    }

    // target bit rate
    if(!lossless && bitRate > 0)
    {
        passStringList << "-b:v" << QString::number(bitRate).append("K");
    }

    // threads/speed
    passStringList << "-threads" << QString::number(1);

    // tile columns/frame parallel, vp9 only
    //passStringList << "-tile-columns" << QString::number(6);
    //passStringList << "-frame-parallel" << QString::number(1); // vp9 option

    // auto alt ref/lag in frames
    if(!twoPass || passNumber != 1)
    {
        passStringList << "-auto-alt-ref" << QString::number(1);
        passStringList << "-lag-in-frames" << QString::number(25);
    }

    // g/aq mode - vp9 specific
    //passStringList << "-g" << QString::number(9999);
    //passStringList << "-aq-mode" << QString::number(0);


    // cpu-used/quality, vp8 options
    passStringList << "-quality" << "good";
    passStringList << "-cpu-used" << QString::number(0);

    // filters
    QString filterChain;
    if(cropWidth < videoStream->codec->width && cropHeight < videoStream->codec->height)
    {
        filterChain.append("crop=" + QString::number(cropWidth) + ":" + QString::number(cropHeight) +
                           ":" + QString::number(cropX) + ":" + QString::number(cropY));
    }
    if(width > 0 || height > 0)
    {
        if(!filterChain.isEmpty())
            filterChain.append(",");

        filterChain.append("scale=" + QString::number(width) + ":" + QString::number(height));
    }
    if(subtitleStreamId > -1)
    {
        if(!filterChain.isEmpty())
            filterChain.append(",");

        int subtitleStreamNumber = 0;
        for(int i = 0; i < subtitleStreamId; i++)
        {
            AVStream *currentStream = formatContext->streams[i];
            if(currentStream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                subtitleStreamNumber++;
            }
        }
        filterChain.append(QString("subtitles='" + QFileInfo(inputFilePath).canonicalFilePath()
                                   .replace(":","\\:") + "':si=" + QString::number(subtitleStreamNumber))
                           .replace("'","\\'").replace("[","\\[").replace("]","\\]")
                           .replace(",","\\,").replace(";","\\;"));
    }
    QString customFilters = ui->customFiltersLineEdit->text().trimmed();
    if(!customFilters.isEmpty())
    {
        if(!filterChain.isEmpty())
            filterChain.append(",");

        filterChain.append(customFilters);
    }

    if(!filterChain.isEmpty())
    {
        passStringList << "-vf" << filterChain;
    }

    // audio
    if(audioStreamId == -1 || (twoPass && passNumber == 1))
    {
        // no audio on first pass or if audio is disabled
        passStringList << "-an";
    }
    // webm supports only opus/vorbis, so use same settings across the board for now
    /*else if(lossless)
    {
        // compress lossless pcm audio to flac
        if(QString::fromStdString(audioStream->codec->codec_descriptor->name).startsWith("pcm"))
        {
            passStringList << "-c:a:0." + QString::number(audioStreamId) << "flac";
            passStringList << "-compression_level" << QString::number(8);
        }
        // otherwise copy the audio stream
        else passStringList << "-c:a:0." + QString::number(audioStreamId) << "copy";
    }*/
    else
    {
        // convert audio to 64kbps opus
        passStringList << "-c:a:0." + QString::number(audioStreamId) << "libopus";
        passStringList << "-b:a" << "64k";
    }

    // ignore subtitle streams
    passStringList << "-sn";

    // extra parameters
    if(!customParameters.trimmed().isEmpty())
        passStringList << customParameters.trimmed().split(' ');

    // make webm
    passStringList << "-f" << "webm";

    // output file
    if(twoPass && passNumber == 1)
    {
        passStringList << QDir::toNativeSeparators(QFileInfo(outputFilePath).dir().absolutePath() + "/temp/null");
    }
    else
    {
        passStringList << outputFilePath;
    }

    closeInputFile(formatContext);

    qDebug() << passStringList;
    //QStringList dummy;
    //return dummy;
    return passStringList;
}

void MainWindow::encodePass(QStringList &encodingParameters)
{
    QProcess ffmpegProcess;

    ffmpegProcess.start("ffmpeg",encodingParameters);

    if(ffmpegProcess.waitForStarted())
    {
        /*QString inputFileName = encodingParameters[encodingParameters.indexOf("-i") + 1];
        QString codecString = encodingParameters[encodingParameters.indexOf("libvpx-vp9") - 1];
        int videoStreamId = codecString.right(1).toInt();
        double frameRate = getFrameRate(inputFileName,videoStreamId);
        QTime duration = getDuration(inputFileName);*/

        while(ffmpegProcess.waitForReadyRead(120000))
        {
            qDebug() << ffmpegProcess.readAllStandardError();
            //updateProgressBar(ffmpegProcess.readAllStandardError(),frameRate,duration);
        }
    }

    if(ffmpegProcess.exitCode() != 0)
    {
        QMessageBox::warning(this,"Warning","ffmpeg returned an exit code of " +
                             QString::number(ffmpegProcess.exitCode()) +
                             ". Errors may have occured.",QMessageBox::Ok);
    }
}

double MainWindow::getFrameRate(QString &inputFileName, int videoStreamId)
{
    AVFormatContext *formatContext = openInputFile(inputFileName);
    double frameRate = av_q2d(formatContext->streams[videoStreamId]->codec->framerate);
    closeInputFile(formatContext);
    return frameRate;
}

QTime MainWindow::getDuration(QString &inputFileName)
{
    AVFormatContext *formatContext = openInputFile(inputFileName);
    QTime duration = QTime(0,0).addMSecs((double)(formatContext->duration + 500) / AV_TIME_BASE * 1000);
    closeInputFile(formatContext);
    return duration;
}

void MainWindow::updateProgressBar(QByteArray &standardError,double frameRate,QTime duration)
{
    // do stuff
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

    if(validateInputFile(inputFilePath))
    {
        processInputFile(inputFilePath);
        QString outputFilePath = ui->outputFileLineEdit->text().trimmed();
        if(validateOutputFile(outputFilePath) && validateFormFields())
        {
            ui->encodePushButton->setEnabled(true);
        }
    }
}

void MainWindow::on_outputFileLineEdit_textChanged(const QString &arg1)
{
    ui->encodePushButton->setEnabled(false);

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = arg1.trimmed();

    if(validateInputFile(inputFilePath) && validateOutputFile(outputFilePath) && validateFormFields())
    {
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
    QComboBox *targetModeComboBox = ui->rateTargetModeComboBox;
    QString currentModeText = targetModeComboBox->currentText();

    if(currentMode == "Variable Bit Rate" || currentMode == "Constant Bit Rate")
    {
        // Disable CRF selection, enable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(false);
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(currentModeText);
    }
    else if(currentMode == "Constant Quality")
    {
        // Enable CRF selection, disable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(true);
        ui->rateTargetModeComboBox->setEnabled(false);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
    }
    else if(currentMode == "Constrained Quality")
    {
        // Enable all rate control selection
        ui->rateCRFSpinBox->setEnabled(true);
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(currentModeText);
    }
    else if(currentMode == "Lossless")
    {
        // Disable all rate control selection
        ui->rateCRFSpinBox->setEnabled(false);
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

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();
    if(validateInputFile(inputFilePath) && validateOutputFile(outputFilePath) && validateFormFields())
        ui->encodePushButton->setEnabled(true);
    else
        ui->encodePushButton->setEnabled(false);
}

void MainWindow::on_rateTargetModeComboBox_currentIndexChanged(const QString &arg1)
{
    QString currentTargetMode = arg1;

    refreshTargetMode(currentTargetMode);

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();
    if(validateInputFile(inputFilePath) && validateOutputFile(outputFilePath) && validateFormFields())
        ui->encodePushButton->setEnabled(true);
    else
        ui->encodePushButton->setEnabled(false);
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
        QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
        AVFormatContext *formatContext = openInputFile(inputFilePath);

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
        QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
        AVFormatContext *formatContext = openInputFile(inputFilePath);
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
        QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
        AVFormatContext *formatContext = openInputFile(inputFilePath);
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

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && validateInputFile(inputFilePath))
    {
        AVFormatContext *formatContext = openInputFile(inputFilePath);
        ui->rateTargetFileSizeDoubleSpinBox->setValue(
                    calculateFileSize(ui->rateTargetBitRateSpinBox->value(),getOutputDuration(formatContext->duration)));
        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimStartEndEndTimeEdit_editingFinished()
{
    ui->trimStartEndEndChapterComboBox->setCurrentIndex(0);
    QTime startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();
    if(endTime < startTime)
        ui->trimStartEndStartTimeEdit->setTime(endTime);

    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && validateInputFile(inputFilePath))
    {
        AVFormatContext *formatContext = openInputFile(inputFilePath);
        ui->rateTargetFileSizeDoubleSpinBox->setValue(
                    calculateFileSize(ui->rateTargetBitRateSpinBox->value(),getOutputDuration(formatContext->duration)));
        closeInputFile(formatContext);
    }
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
    int videoStreamId = -1;
    int audioStreamId = -1;
    int subtitleStreamId = -1;
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

    int cropLeft = 0;
    int cropRight = 0;
    int cropTop = 0;
    int cropBottom = 0;
    int width = -1;
    int height = -1;
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

    int crf = -1;
    double targetFileSize = -1;
    int targetBitRate = -1;
    bool cbr = false;
    QString rateMode = ui->rateModeComboBox->currentText();
    if(rateMode == "Constant Bit Rate")
    {
        cbr = true;
    }
    if(rateMode == "Constant Bit Rate" || rateMode == "Variable Bit Rate" || rateMode == "Constrained Quality")
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
    if(rateMode == "Constant Quality" || rateMode == "Constrained Quality")
    {
        crf = ui->rateCRFSpinBox->value();
    }

    QString customParameters = ui->customEncodingParametersLineEdit->text().trimmed();

    // two pass encode
    bool twoPass = true;
    QStringList firstPass,secondPass;

    firstPass = generatePass(1,inputFilePath,outputFilePath,videoStreamId,
                                    audioStreamId,subtitleStreamId,startTime,
                                    endTime,duration,cropLeft,cropRight,cropTop,
                                    cropBottom,width,height,crf,targetFileSize,
                                    targetBitRate,cbr,customParameters,twoPass);

    secondPass = generatePass(2,inputFilePath,outputFilePath,videoStreamId,
                                    audioStreamId,subtitleStreamId,startTime,
                                    endTime,duration,cropLeft,cropRight,cropTop,
                                    cropBottom,width,height,crf,targetFileSize,
                                    targetBitRate,cbr,customParameters,twoPass);

    QDir outputDirectory = QFileInfo(outputFilePath).dir();
    QDir tempDirectory = outputDirectory.canonicalPath() + "/temp";
    QFile logFile("ffmpeg2pass-0.log");

    if(!tempDirectory.exists())
        QDir().mkdir(tempDirectory.absolutePath());

    encodePass(firstPass);
    ui->progressBar->setValue(50);
    encodePass(secondPass);

    if(tempDirectory.exists())
        tempDirectory.removeRecursively();
    if(logFile.exists())
        logFile.remove();

    ui->progressBar->setValue(100);
    QMessageBox::information(this,"Success","Encode successful.",
                             QMessageBox::Ok);

    ui->progressBar->setValue(0);
    ui->encodePushButton->setEnabled(false); // output file name conflict

    this->setEnabled(true);
}

void MainWindow::on_cancelPushButton_clicked()
{
    //ffmpegProcess->close();
    QMessageBox::information(this,"Information","The encoding process has been cancelled.",QMessageBox::Ok);
}

void MainWindow::on_rateTargetFileSizeDoubleSpinBox_editingFinished()
{
    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();
    if(validateInputFile(inputFilePath) && validateOutputFile(outputFilePath) && validateFormFields())
        ui->encodePushButton->setEnabled(true);
    else
        ui->encodePushButton->setEnabled(false);
}

void MainWindow::on_rateTargetBitRateSpinBox_editingFinished()
{   
    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    QString outputFilePath = ui->outputFileLineEdit->text().trimmed();
    if(validateInputFile(inputFilePath) && validateOutputFile(outputFilePath) && validateFormFields())
        ui->encodePushButton->setEnabled(true);
    else
        ui->encodePushButton->setEnabled(false);

    if(validateInputFile(inputFilePath))
    {
        AVFormatContext *formatContext = openInputFile(inputFilePath);
        ui->rateTargetFileSizeDoubleSpinBox->setValue(
                    calculateFileSize(ui->rateTargetBitRateSpinBox->value(),getOutputDuration(formatContext->duration)));
        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimDurationStartTimeEdit_editingFinished()
{
    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && validateInputFile(inputFilePath))
    {
        AVFormatContext *formatContext = openInputFile(inputFilePath);
        ui->rateTargetFileSizeDoubleSpinBox->setValue(
                    calculateFileSize(ui->rateTargetBitRateSpinBox->value(),getOutputDuration(formatContext->duration)));
        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimDurationDurationTimeEdit_editingFinished()
{
    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && validateInputFile(inputFilePath))
    {
        AVFormatContext *formatContext = openInputFile(inputFilePath);
        ui->rateTargetFileSizeDoubleSpinBox->setValue(
                    calculateFileSize(ui->rateTargetBitRateSpinBox->value(),getOutputDuration(formatContext->duration)));
        closeInputFile(formatContext);
    }
}

void MainWindow::on_trimNoneRadioButton_clicked()
{
    QString inputFilePath = ui->inputFileLineEdit->text().trimmed();
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && validateInputFile(inputFilePath))
    {
        AVFormatContext *formatContext = openInputFile(inputFilePath);
        ui->rateTargetFileSizeDoubleSpinBox->setValue(
                    calculateFileSize(ui->rateTargetBitRateSpinBox->value(),getOutputDuration(formatContext->duration)));
        closeInputFile(formatContext);
    }
}
