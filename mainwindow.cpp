#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    connect(ui->inputFileBrowsePushButton,SIGNAL(clicked(bool)),ui->actionOpen,SLOT(trigger()));

    inputFile = new InputFile();
    outputFile = new OutputFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    // use the first item in a drag & drop event as input file
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

void MainWindow::validateFormFields()
{
    ui->encodePushButton->setEnabled(false);

    if(!inputFile->isValid() || !outputFile->isValid())
        return;
    if(ui->streamVideoComboBox->currentIndex() == 0)
        return;
    if(ui->rateTargetModeComboBox->isEnabled())
    {
        if(ui->rateTargetBitRateSpinBox->isEnabled() && ui->rateTargetBitRateSpinBox->value() == 0)
            return;
        if(ui->rateTargetFileSizeDoubleSpinBox->isEnabled() && ui->rateTargetFileSizeDoubleSpinBox->value() == 0)
            return;
    }
    if(ui->trimStartEndRadioButton->isChecked() && ui->trimStartEndStartTimeEdit->time()
            .msecsTo(ui->trimStartEndEndTimeEdit->time()) == 0)
        return;
    if(ui->trimDurationRadioButton->isChecked() && QTime(0,0).msecsTo(ui->trimDurationDurationTimeEdit->time()) == 0)
        return;

    ui->encodePushButton->setEnabled(true);
}

void MainWindow::refreshTargetMode(QString &currentTargetMode)
{
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

void MainWindow::processInputFile(QString &inputFilePath)
{
    inputFile = new InputFile(this,inputFilePath);
    outputFile = new OutputFile(this,inputFilePath);

    connectSignalsAndSlots();
    populateStreamComboBoxes();
    initializeFormData();
}

void MainWindow::populateStreamComboBoxes()
{
    for(int i = 0; i < inputFile->streamCount(); i++)
    {
        InputStream currentStream = inputFile->stream(i);

        QString streamStr = "[" + QString::number(currentStream.id()) + "] ";

        // Title
        if(!currentStream.title().isEmpty())
            streamStr.append("\"" + currentStream.title() + "\" - ");

        // Codec
        streamStr.append(currentStream.codec());

        // Profile
        if(!currentStream.profile().isEmpty())
            streamStr.append("/" + currentStream.profile());

        // Audio information
        if(currentStream.type() == InputStream::AUDIO)
        {
            streamStr.append(" (");

            // Bit Rate
            if(currentStream.bitRate() > 0)
                streamStr.append(QString::number(round((double)currentStream.bitRate() / 1000)) + "kbps");

            // Channel Layout
            if(!currentStream.channelLayout().isEmpty() && currentStream.bitRate() > 0)
                streamStr.append("/");

            streamStr.append(currentStream.channelLayout() + ")");
        }

        // Language
        if(!currentStream.language().isEmpty())
            streamStr.append(" (" + currentStream.language() + ")");

        // Disposition
        /*if(currentStream.isDefault())
            streamStr.append(" [default]");
        if(currentStream.isForced())
            streamStr.append(" [forced]");*/

        if(currentStream.type() == (int)InputStream::VIDEO)
            ui->streamVideoComboBox->addItem(streamStr);
        else if(currentStream.type() == (int)InputStream::AUDIO)
            ui->streamAudioComboBox->addItem(streamStr);
        else if(currentStream.type() == (int)InputStream::SUBTITLE)
            ui->streamSubtitlesComboBox->addItem(streamStr);
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
    for(int i = 0; i < inputFile->chapterCount(); i++)
    {
        InputChapter currentChapter = inputFile->chapter(i);

        QString chapterStr = "[" + QString::number(currentChapter.id()) + "] ";

        // Title
        if(!currentChapter.title().isEmpty())
            chapterStr.append(currentChapter.title());

        ui->trimStartEndStartChapterComboBox->addItem(chapterStr.trimmed());
        ui->trimStartEndEndChapterComboBox->addItem(chapterStr.trimmed());
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
    // clear crop values
    ui->cropLeftSpinBox->setValue(0);
    ui->cropRightSpinBox->setValue(0);
    ui->cropTopSpinBox->setValue(0);
    ui->cropBottomSpinBox->setValue(0);
    ui->cropLeftSpinBox->setMaximum(9998);
    ui->cropRightSpinBox->setMaximum(9998);
    ui->cropTopSpinBox->setMaximum(9998);
    ui->cropBottomSpinBox->setMaximum(9998);

    // clear generated form fields
    ui->resizeWidthSpinBox->setValue(0);
    ui->resizeHeightSpinBox->setValue(0);
    ui->trimStartEndStartTimeEdit->setTime(QTime(0,0));
    ui->trimStartEndEndTimeEdit->setTime(QTime(0,0));
    ui->trimDurationStartTimeEdit->setTime(QTime(0,0));
    ui->trimDurationDurationTimeEdit->setTime(QTime(0,0));
    ui->rateTargetBitRateSpinBox->setValue(0);
    ui->rateTargetFileSizeDoubleSpinBox->setValue(0);

    // restore minimum/maximum time on time edits
    ui->trimStartEndStartTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimStartEndEndTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimDurationStartTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimDurationDurationTimeEdit->setMaximumTime(QTime(23,59,59,999));
    ui->trimStartEndEndTimeEdit->setMinimumTime(QTime(0,0));
    ui->trimDurationDurationTimeEdit->setMinimumTime(QTime(0,0));

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

void MainWindow::initializeFormData()
{
    // refresh rate control values
    ui->rateCRFSpinBox->setValue(10);
    ui->rateTargetBitRateSpinBox->setValue(0);
    ui->rateTargetFileSizeDoubleSpinBox->setValue(0);

    // enable and initialize output file controls
    ui->outputFileLineEdit->setText(QDir::toNativeSeparators(outputFile->filePath()));
    ui->outputFileLineEdit->setEnabled(true);
    ui->outputFileBrowsePushButton->setEnabled(true);

    // set default end time, duration and maximum time edit values based on the container's duration
    QTime duration = inputFile->duration();
    ui->trimDurationDurationTimeEdit->setTime(duration);
    ui->trimStartEndEndTimeEdit->setTime(duration);
    ui->trimStartEndStartTimeEdit->setMaximumTime(duration.addMSecs(-1));
    ui->trimStartEndEndTimeEdit->setMaximumTime(duration);
    ui->trimDurationStartTimeEdit->setMaximumTime(duration.addMSecs(-1));
    ui->trimDurationDurationTimeEdit->setMaximumTime(duration);
    ui->trimStartEndEndTimeEdit->setMinimumTime(QTime(0,0).addMSecs(1));
    ui->trimDurationDurationTimeEdit->setMinimumTime(QTime(0,0).addMSecs(1));

    // set default width and height based on the largest video stream's
    int width = inputFile->width();
    int height = inputFile->height();
    ui->resizeWidthSpinBox->setValue(width);
    ui->resizeHeightSpinBox->setValue(height);
    ui->cropLeftSpinBox->setMaximum(width - 1);
    ui->cropRightSpinBox->setMaximum(width - 1);
    ui->cropTopSpinBox->setMaximum(height - 1);
    ui->cropBottomSpinBox->setMaximum(height - 1);

    // set default target bitrate and file size based on the container's
    int bitRate = inputFile->bitRateInKilobits();
    ui->rateTargetBitRateSpinBox->setValue(bitRate);
    double fileSize = inputFile->fileSizeInMegabytes();
    ui->rateTargetFileSizeDoubleSpinBox->setValue(fileSize);
}

QTime MainWindow::getOutputDuration()
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
            computedDuration = inputFile->duration();
    }
    return computedDuration;
}

void MainWindow::connectSignalsAndSlots()
{
    connect(ui->cropLeftSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropLeft(int)));
    connect(ui->cropRightSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropRight(int)));
    connect(ui->cropTopSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropTop(int)));
    connect(ui->cropBottomSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropBottom(int)));
    connect(ui->resizeWidthSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setWidth(int)));
    connect(ui->resizeHeightSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setHeight(int)));
    connect(ui->trimStartEndStartTimeEdit,SIGNAL(timeChanged(QTime)),outputFile,SLOT(setStartTime(QTime)));
    connect(ui->trimDurationStartTimeEdit,SIGNAL(timeChanged(QTime)),outputFile,SLOT(setStartTime(QTime)));
    connect(ui->trimStartEndEndTimeEdit,SIGNAL(timeChanged(QTime)),outputFile,SLOT(setEndTime(QTime)));
    connect(ui->codecVideoComboBox,SIGNAL(currentIndexChanged(int)),outputFile,SLOT(setVideoCodec(int)));
    connect(ui->codecAudioComboBox,SIGNAL(currentIndexChanged(int)),outputFile,SLOT(setAudioCodec(int)));
    connect(ui->rateTargetBitRateSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setBitRateInKilobits(int)));
    connect(ui->rateTargetFileSizeDoubleSpinBox,SIGNAL(valueChanged(double)),outputFile,SLOT(setBitRateForMegabytes(double)));
    connect(ui->rateCRFSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCrf(int)));
    connect(ui->customFiltersLineEdit,SIGNAL(textChanged(QString)),outputFile,SLOT(setCustomFilters(QString)));
    connect(ui->customEncodingParametersLineEdit,SIGNAL(textChanged(QString)),outputFile,SLOT(setCustomParameters(QString)));
}

QStringList MainWindow::generatePass(int passNumber, bool twoPass)
{
    QString inputFilePath = inputFile->filePath();
    QString outputFilePath = outputFile->filePath();

    // get streams
    InputStream videoStream = InputStream();
    InputStream audioStream = InputStream();
    InputStream subtitleStream = InputStream();
    int videoStreamCounter = ui->streamVideoComboBox->currentIndex() - 1;
    int audioStreamCounter = ui->streamAudioComboBox->currentIndex() - 1;
    int subtitleStreamCounter = ui->streamSubtitlesComboBox->currentIndex() - 1;
    for(int i = 0; i < inputFile->streamCount(); i++)
    {
        InputStream stream = inputFile->stream(i);

        if(stream.type() == InputStream::VIDEO)
        {
            if(videoStreamCounter == 0)
                videoStream = stream;
            else
                videoStreamCounter--;
        }
        else if(stream.type() == InputStream::AUDIO)
        {
            if(audioStreamCounter == 0)
                audioStream = stream;
            else
                audioStreamCounter--;
        }
        else if(stream.type() == InputStream::SUBTITLE)
        {
            if(subtitleStreamCounter == 0)
                subtitleStream = stream;
            else
                subtitleStreamCounter--;
        }
    }
    bool vp9 = outputFile->videoCodec() == OutputFile::VP9;
    bool vorbis = outputFile->audioCodec() == OutputFile::VORBIS;

    QTime startTime, endTime;
    if(ui->trimStartEndRadioButton->isChecked())
        outputFile->setStartTime(ui->trimStartEndStartTimeEdit->time());
    else if(ui->trimDurationRadioButton->isChecked())
        outputFile->setStartTime(ui->trimDurationStartTimeEdit->time());
    if(ui->trimStartEndRadioButton->isChecked() || ui->trimDurationRadioButton->isChecked())
    {
        startTime = outputFile->startTime();
        endTime = outputFile->endTime();
    }

    int cropLeft = 0;
    int cropRight = 0;
    int cropTop = 0;
    int cropBottom = 0;
    int width = -1;
    int height = -1;
    if(ui->cropCheckBox->isChecked())
    {
        cropLeft = outputFile->cropLeft();
        cropRight = outputFile->cropRight();
        cropTop = outputFile->cropTop();
        cropBottom = outputFile->cropBottom();
    }
    if(ui->resizeCheckBox->isChecked())
    {
        width = outputFile->width();
        height = outputFile->height();
        if(width == 0) width = -1;
        if(height == 0) height = -1;
    }

    int crf = -1;
    int bitRate = -1;
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
            outputFile->setBitRateForMegabytes(ui->rateTargetFileSizeDoubleSpinBox->value());
        }
        else if(ui->rateTargetModeComboBox->currentText() == "Bit Rate")
        {
            outputFile->setBitRateInKilobits(ui->rateTargetBitRateSpinBox->value());
        }
        bitRate = outputFile->bitRateInKilobits();
    }
    if(rateMode == "Constant Quality" || rateMode == "Constrained Quality")
    {
        crf = outputFile->crf();
    }

    QString customFilters = outputFile->customFilters().trimmed();
    QString customParameters = outputFile->customParameters().trimmed();

    // Build the string list
    QStringList passStringList = QStringList();

    // calculate target bitrate and cropping if needed
    QTime computedDuration = getOutputDuration();
    if(audioStream.isValid() && bitRate > 64)
        bitRate -= 64;

    int cropWidth = videoStream.width() - cropLeft - cropRight;
    int cropHeight = videoStream.height() - cropTop - cropBottom;
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

    // video codec
    if(vp9)
        passStringList << "-c:v:0." + QString::number(videoStream.id()) << "libvpx-vp9";
    else
        passStringList << "-c:v:0." + QString::number(videoStream.id()) << "libvpx";

    // pass number
    if(twoPass) passStringList << "-pass" << QString::number(passNumber);

    // lossless
    if(lossless)
        passStringList << "-lossless" << QString::number(1);

    // cbr
    if(cbr && bitRate > 0)
    {
        passStringList << "-minrate" << QString::number(bitRate).append("K");
        passStringList << "-maxrate" << QString::number(bitRate).append("K");
    }

    // crf
    if(crf > -1)
        passStringList << "-crf" << QString::number(crf);

    // target bit rate
    if(!lossless && bitRate > 0)
        passStringList << "-b:v" << QString::number(bitRate).append("K");

    // threads/speed
    passStringList << "-threads" << QString::number(1);

    // last pass exclusive parameters
    if(!twoPass || passNumber != 1)
    {
        passStringList << "-auto-alt-ref" << QString::number(1);
        passStringList << "-lag-in-frames" << QString::number(25);
        if(vp9)
            passStringList << "-speed" << QString::number(0);
    }
    else
    {
        if(vp9)
            passStringList << "-speed" << QString::number(4);
    }

    // vp8/vp9 exclusive parameters
    if(vp9)
    {
        passStringList << "-tile-columns" << QString::number(6);
        passStringList << "-frame-parallel" << QString::number(1);
        passStringList << "-g" << QString::number(9999);
        passStringList << "-aq-mode" << QString::number(0);
    }
    else
    {
        passStringList << "-quality" << "good";
        passStringList << "-cpu-used" << QString::number(0);
    }

    // filters
    QString filterChain;
    if(cropWidth < videoStream.width() && cropHeight < videoStream.height())
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
    if(subtitleStream.isValid())
    {
        if(!filterChain.isEmpty())
            filterChain.append(",");

        int subtitleStreamNumber = 0;
        for(int i = 0; i < subtitleStream.id(); i++)
        {
            if(inputFile->stream(i).type() == InputStream::SUBTITLE)
                subtitleStreamNumber++;
        }
        filterChain.append(QString("subtitles='" + inputFilePath.replace(":","\\:") + "':si=" + QString::number(subtitleStreamNumber))
                           .replace("'","\\'").replace("[","\\[").replace("]","\\]")
                           .replace(",","\\,").replace(";","\\;"));
    }
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
    if(!audioStream.isValid() || (twoPass && passNumber == 1))
    {
        passStringList << "-an";
    }
    else
    {
        if(vorbis)
            passStringList << "-c:a:0." + QString::number(audioStream.id()) << "libvorbis";
        else
            passStringList << "-c:a:0." + QString::number(audioStream.id()) << "libopus";

        passStringList << "-b:a" << "64k";
    }

    // ignore subtitle streams
    passStringList << "-sn";

    // extra parameters
    if(!customParameters.isEmpty())
        passStringList << customParameters.split(' ');

    // make webm
    passStringList << "-f" << "webm";

    // output file
    if(twoPass && passNumber == 1)
    {
        passStringList << QDir::toNativeSeparators(QFileInfo(outputFilePath).absolutePath() + "/temp/null");
    }
    else
    {
        passStringList << outputFilePath;
    }

    qDebug() << passStringList;
    /*QStringList dummy = QStringList();
    return dummy;*/
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

        while(ffmpegProcess.waitForReadyRead(240000))
        {
            qDebug() << ffmpegProcess.readAllStandardError();
            //updateProgressBar(ffmpegProcess.readAllStandardError(),frameRate,duration);
        }
    }

    if(ffmpegProcess.exitCode() != 0)
    {
        /*QMessageBox::warning(this,"Warning","ffmpeg returned an exit code of " +
                             QString::number(ffmpegProcess.exitCode()) +
                             ". Errors may have occured.",QMessageBox::Ok);*/
    }
}

void MainWindow::updateProgressBar()
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
    QString inputFilePath = arg1;

    clearInputFileFormData();
    ui->encodePushButton->setEnabled(false);

    if(InputFile::isValid(inputFilePath))
    {
        processInputFile(inputFilePath);

        validateFormFields();
    }
}

void MainWindow::on_outputFileLineEdit_textChanged(const QString &arg1)
{
    QString outputFilePath = arg1;

    ui->encodePushButton->setEnabled(false);

    outputFile->setFilePath(outputFilePath);

    validateFormFields();
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

    validateFormFields();
}

void MainWindow::on_rateTargetModeComboBox_currentIndexChanged(const QString &arg1)
{
    QString currentTargetMode = arg1;

    refreshTargetMode(currentTargetMode);

    validateFormFields();
}

void MainWindow::on_streamVideoComboBox_currentIndexChanged(int index)
{
    // disable encode button and reset resolution fields if no video stream is selected
    if(index == 0)
    {
        ui->encodePushButton->setEnabled(false);
        ui->resizeWidthSpinBox->setValue(0);
        ui->resizeHeightSpinBox->setValue(0);
        ui->codecVideoComboBox->setEnabled(false);
    }
    // change resolution fields based on the selected stream
    else
    {
        int videoStreamIndex = 0;
        for(int i = 0; i < inputFile->streamCount(); i++)
        {
            InputStream stream = inputFile->stream(i);
            if(stream.type() == InputStream::VIDEO)
            {
                videoStreamIndex++;
                if(videoStreamIndex == index)
                {
                    ui->resizeWidthSpinBox->setValue(stream.width());
                    ui->resizeHeightSpinBox->setValue(stream.height());
                }
            }
        }
        ui->codecVideoComboBox->setEnabled(true);
    }
}

void MainWindow::on_trimStartEndRadioButton_toggled(bool checked)
{
    if(checked)
    {
        if(ui->trimStartEndStartChapterComboBox->count() > 1)
        {
            ui->trimStartEndStartChapterComboBox->setEnabled(true);
            ui->trimStartEndEndChapterComboBox->setEnabled(true);
        }
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

        InputChapter chapter = inputFile->chapter(index - 1);
        ui->trimStartEndStartTimeEdit->setTime(chapter.startTime());
        if(index > endChapterIndex && endChapterIndex > 0)
        {
            ui->trimStartEndEndChapterComboBox->setCurrentIndex(index);
            ui->trimStartEndEndTimeEdit->setTime(chapter.endTime());
        }
    }
}

void MainWindow::on_trimStartEndEndChapterComboBox_activated(int index)
{
    if(index > 0)
    {
        int startChapterIndex = ui->trimStartEndStartChapterComboBox->currentIndex();
        InputChapter chapter = inputFile->chapter(index - 1);
        ui->trimStartEndEndTimeEdit->setTime(chapter.endTime());
        if(index < startChapterIndex)
        {
            ui->trimStartEndStartChapterComboBox->setCurrentIndex(index);
            ui->trimStartEndStartTimeEdit->setTime(chapter.startTime());
        }
    }
}

void MainWindow::on_trimStartEndStartTimeEdit_editingFinished()
{
    ui->trimStartEndStartChapterComboBox->setCurrentIndex(0);
    QTime startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();
    if(startTime >= endTime)
        ui->trimStartEndEndTimeEdit->setTime(startTime.addMSecs(1));

    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(inputFile->fileSizeInMegabytes(getOutputDuration()));

    validateFormFields();
}

void MainWindow::on_trimStartEndEndTimeEdit_editingFinished()
{
    ui->trimStartEndEndChapterComboBox->setCurrentIndex(0);
    QTime startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();
    if(endTime <= startTime)
        ui->trimStartEndStartTimeEdit->setTime(endTime.addMSecs(-1));

    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(inputFile->fileSizeInMegabytes(getOutputDuration()));

    validateFormFields();
}

void MainWindow::on_cropLeftSpinBox_editingFinished()
{
    int cropLeftValue = ui->cropLeftSpinBox->value();
    int cropRightValue = ui->cropRightSpinBox->value();
    int width = inputFile->width();
    if(cropLeftValue % 2 != 0)
    {
        cropLeftValue--;
        ui->cropLeftSpinBox->setValue(cropLeftValue);
    }

    if(cropLeftValue + cropRightValue >= width)
        ui->cropRightSpinBox->setValue(width - cropLeftValue - 2);
}

void MainWindow::on_cropRightSpinBox_editingFinished()
{
    int cropLeftValue = ui->cropLeftSpinBox->value();
    int cropRightValue = ui->cropRightSpinBox->value();
    int width = inputFile->width();
    if(cropRightValue % 2 != 0)
    {
        cropRightValue--;
        ui->cropRightSpinBox->setValue(cropRightValue);
    }

    if(cropLeftValue + cropRightValue >= width)
        ui->cropLeftSpinBox->setValue(width - cropRightValue - 2);
}

void MainWindow::on_cropTopSpinBox_editingFinished()
{
    int cropTopValue = ui->cropTopSpinBox->value();
    int cropBottomValue = ui->cropBottomSpinBox->value();
    int height = inputFile->height();
    if(cropTopValue % 2 != 0)
    {
        cropTopValue--;
        ui->cropTopSpinBox->setValue(cropTopValue);
    }

    if(cropTopValue + cropBottomValue >= height)
        ui->cropBottomSpinBox->setValue(height - cropTopValue - 2);
}

void MainWindow::on_cropBottomSpinBox_editingFinished()
{
    int cropTopValue = ui->cropTopSpinBox->value();
    int cropBottomValue = ui->cropBottomSpinBox->value();
    int height = inputFile->height();
    if(cropBottomValue % 2 != 0)
    {
        cropBottomValue--;
        ui->cropBottomSpinBox->setValue(cropBottomValue);
    }

    if(cropTopValue + cropBottomValue >= height)
        ui->cropTopSpinBox->setValue(height - cropBottomValue - 2);
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

    // two pass encode
    QStringList firstPass, secondPass;

    firstPass = generatePass(1);
    secondPass = generatePass(2);

    QDir outputDirectory = QFileInfo(outputFile->filePath()).dir();
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
    validateFormFields();
}

void MainWindow::on_rateTargetBitRateSpinBox_editingFinished()
{   
    if(inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(inputFile->fileSizeInMegabytes(getOutputDuration()));

    validateFormFields();
}

void MainWindow::on_trimDurationStartTimeEdit_editingFinished()
{
    if(inputFile->isValid())
    {
        QTime maxDuration = QTime(0,0).addMSecs(ui->trimDurationStartTimeEdit->time().msecsTo(inputFile->duration()));
        if(ui->trimDurationDurationTimeEdit->time() > maxDuration)
            ui->trimDurationDurationTimeEdit->setTime(maxDuration);
        ui->trimDurationDurationTimeEdit->setMaximumTime(maxDuration);

        if(ui->rateTargetModeComboBox->currentText() == "Bit Rate")
            ui->rateTargetFileSizeDoubleSpinBox->setValue(inputFile->fileSizeInMegabytes(getOutputDuration()));
    }

    validateFormFields();
}

void MainWindow::on_trimDurationDurationTimeEdit_editingFinished()
{
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(inputFile->fileSizeInMegabytes(getOutputDuration()));

    validateFormFields();
}

void MainWindow::on_trimNoneRadioButton_clicked()
{
    if(ui->rateTargetModeComboBox->currentText() == "Bit Rate" && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(inputFile->fileSizeInMegabytes(getOutputDuration()));
}

void MainWindow::on_codecVideoComboBox_currentIndexChanged(const QString &arg1)
{
    QString codec = arg1;
    if(codec == "VP8")
    {
        ui->rateCRFSpinBox->setMinimum(4);
        ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText("Constant Quality"));
        ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText("Lossless"));
    }
    else if(codec == "VP9")
    {
        ui->rateCRFSpinBox->setMinimum(0);
        ui->rateModeComboBox->insertItem(2,"Constant Quality");
        ui->rateModeComboBox->insertItem(4,"Lossless");
    }
}

void MainWindow::on_streamAudioComboBox_currentIndexChanged(int index)
{
    if(index == 0)
        ui->codecAudioComboBox->setEnabled(false);
    else
        ui->codecAudioComboBox->setEnabled(true);
}
