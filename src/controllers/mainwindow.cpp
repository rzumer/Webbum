#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    connect(ui->inputFileBrowsePushButton,SIGNAL(clicked(bool)),ui->actionOpen,SLOT(trigger()));

    // disable controls
    ui->processingGroupBox->setEnabled(false);
    ui->encodingGroupBox->setEnabled(false);

    // local variables
#ifdef Q_OS_WIN32
    taskBarButton = new QWinTaskbarButton(this);
#endif
    ffmpegController = new FFMPEGController();
    inputFile = new InputFile();
    outputFile = new OutputFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *e)
{
#ifdef Q_OS_WIN32
    taskBarButton->setWindow(windowHandle());
    taskBarProgress = taskBarButton->progress();
    connect(ui->progressBar,SIGNAL(valueChanged(int)),taskBarProgress,SLOT(setValue(int)));
#endif

    e->accept();
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

bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::WindowActivate)
    {
        validateFormFields();
    }

    return QWidget::event(event);
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
        if(ui->rateTargetFileSizeDoubleSpinBox->isEnabled() && ui->rateTargetFileSizeDoubleSpinBox->value() == 0.0)
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
    if(currentTargetMode == tr("Bit Rate"))
    {
        ui->rateTargetBitRateSpinBox->setEnabled(true);
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
    }
    else if(currentTargetMode == tr("File Size"))
    {
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(true);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
    }
}

void MainWindow::processInputFile(QString &inputFilePath)
{
    inputFile = new InputFile(this,inputFilePath);
    outputFile = new OutputFile(this,inputFilePath);

    if(!outputFile->isValid())
    {
        QFileInfo file(outputFile->filePath());
        QString outputFilePath = file.absolutePath() + "/" + file.completeBaseName() + "_out.webm";
        if(OutputFile::isValid(outputFilePath))
            outputFile = new OutputFile(this,outputFilePath);
    }

    // initialize crf in case the value is never modified
    outputFile->setCrf(ui->rateCRFSpinBox->value());
    outputFile->setVideoCodec(ui->codecVideoComboBox->currentIndex());
    outputFile->setAudioCodec(ui->codecAudioComboBox->currentIndex());

    connectSignalsAndSlots();
    populateStreamComboBoxes();
    initializeFormData();
    textSubtitlesDisabled = false;

    // Due to an issue with the filtergraph string generation in generatePass(),
    // text subtitles are not supported for input files with a path containing an apostrophe.
    if(inputFilePath.contains("'"))
    {
        bool streamRemoved = false;

        for(int i = 1; i < ui->streamSubtitlesComboBox->count(); i++)
        {
            InputStream subtitleStream = getStreamByType(InputStream::SUBTITLE, i - 1);

            if(!subtitleStream.isImageSub())
            {
                ui->streamSubtitlesComboBox->removeItem(i);
                streamRemoved = true;
            }
        }

        if(ui->streamSubtitlesComboBox->count() <= 1)
            ui->streamSubtitlesComboBox->setEnabled(false);

        if(streamRemoved)
        {
            QString errorMessage = QString(tr(
                        "The text subtitle filter does not support input paths containing apostrophes.\n"
                        "Text subtitle selection was disabled for this file."));
            QMessageBox::warning(this,tr("Warning"),errorMessage);
        }

        textSubtitlesDisabled = true;
    }
}

void MainWindow::populateStreamComboBoxes()
{
    for(int i = 0; i < inputFile->streamCount(); i++)
    {
        InputStream currentStream = inputFile->stream(i);

        QString streamString = currentStream.getShortString();

        if(currentStream.type() == static_cast<int>(InputStream::VIDEO))
            ui->streamVideoComboBox->addItem(streamString);
        else if(currentStream.type() == static_cast<int>(InputStream::AUDIO))
            ui->streamAudioComboBox->addItem(streamString);
        else if(currentStream.type() == static_cast<int>(InputStream::SUBTITLE))
            ui->streamSubtitlesComboBox->addItem(streamString);
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

    // clear audio bitrate
    ui->codecAudioBitRateSpinBox->setValue(128);

    // clear generated form fields
    ui->resizeWidthSpinBox->setMinimum(0);
    ui->resizeHeightSpinBox->setMinimum(0);
    ui->resizeWidthSpinBox->setValue(0);
    ui->resizeHeightSpinBox->setValue(0);
    ui->resizeWidthSpinBox->setMaximum(9998);
    ui->resizeHeightSpinBox->setMaximum(9998);
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
    ui->trimStartEndStartChapterComboBox->insertItem(0,tr("No Chapter"));
    ui->trimStartEndEndChapterComboBox->insertItem(0,tr("No Chapter"));

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

    // disable controls
    ui->processingGroupBox->setEnabled(false);
    ui->encodingGroupBox->setEnabled(false);
    ui->encodePushButton->setEnabled(false);
    ui->outputFileLineEdit->setEnabled(false);
    ui->outputFileBrowsePushButton->setEnabled(false);
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

    // enable controls
    ui->trimmingGroupBox->setEnabled(true);
    ui->processingGroupBox->setEnabled(true);
    ui->encodingGroupBox->setEnabled(true);

    // set default end time, duration and maximum time edit values based on the container's duration
    QTime duration = inputFile->duration();
    ui->trimDurationDurationTimeEdit->setTime(duration);
    ui->trimStartEndEndTimeEdit->setTime(duration);
    ui->trimStartEndStartTimeEdit->setMaximumTime(duration.addMSecs(-1));
    ui->trimStartEndEndTimeEdit->setMaximumTime(duration);
    ui->trimDurationStartTimeEdit->setMaximumTime(duration.addMSecs(-1));
    ui->trimDurationDurationTimeEdit->setMaximumTime(duration);

    // Set the minimum end time and duration higher than 0, unless the duration was not retrieved correctly.
    // If the duration was not retrieved correctly, disable trimming.
    if(duration > QTime(0,0))
    {
        ui->trimStartEndEndTimeEdit->setMinimumTime(QTime(0,0).addMSecs(1));
        ui->trimDurationDurationTimeEdit->setMinimumTime(QTime(0,0).addMSecs(1));
    }
    else
    {
        ui->trimmingGroupBox->setEnabled(false);
    }

    // set default width and height based on the largest video stream's
    int width = inputFile->width();
    int height = inputFile->height();
    ui->resizeWidthSpinBox->setValue(width);
    ui->resizeHeightSpinBox->setValue(height);
    ui->resizeWidthSpinBox->setMinimum(2);
    ui->resizeHeightSpinBox->setMinimum(2);
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

QTime MainWindow::getOutputDuration() const
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
    // UI
    connect(ui->cropLeftSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropLeft(int)), Qt::UniqueConnection);
    connect(ui->cropRightSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropRight(int)), Qt::UniqueConnection);
    connect(ui->cropTopSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropTop(int)), Qt::UniqueConnection);
    connect(ui->cropBottomSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCropBottom(int)), Qt::UniqueConnection);
    connect(ui->resizeWidthSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setWidth(int)), Qt::UniqueConnection);
    connect(ui->resizeHeightSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setHeight(int)), Qt::UniqueConnection);
    connect(ui->trimStartEndStartTimeEdit,SIGNAL(timeChanged(QTime)),outputFile,SLOT(setStartTime(QTime)), Qt::UniqueConnection);
    connect(ui->trimDurationStartTimeEdit,SIGNAL(timeChanged(QTime)),outputFile,SLOT(setStartTime(QTime)), Qt::UniqueConnection);
    connect(ui->trimStartEndEndTimeEdit,SIGNAL(timeChanged(QTime)),outputFile,SLOT(setEndTime(QTime)), Qt::UniqueConnection);
    connect(ui->codecVideoComboBox,SIGNAL(currentIndexChanged(int)),outputFile,SLOT(setVideoCodec(int)), Qt::UniqueConnection);
    connect(ui->codecAudioComboBox,SIGNAL(currentIndexChanged(int)),outputFile,SLOT(setAudioCodec(int)), Qt::UniqueConnection);
    connect(ui->rateTargetBitRateSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setBitRateInKilobits(int)), Qt::UniqueConnection);
    connect(ui->rateTargetFileSizeDoubleSpinBox,SIGNAL(valueChanged(double)),outputFile,SLOT(setBitRateForMegabytes(double)), Qt::UniqueConnection);
    connect(ui->rateCRFSpinBox,SIGNAL(valueChanged(int)),outputFile,SLOT(setCrf(int)), Qt::UniqueConnection);
    connect(ui->customFiltersLineEdit,SIGNAL(textChanged(QString)),outputFile,SLOT(setCustomFilters(QString)), Qt::UniqueConnection);
    connect(ui->customEncodingParametersLineEdit,SIGNAL(textChanged(QString)),outputFile,SLOT(setCustomParameters(QString)), Qt::UniqueConnection);

    // FFMPEG Controller
    connect(ffmpegController,SIGNAL(failed(bool)),this,SLOT(encodeFailed(bool)), Qt::UniqueConnection);
    connect(ffmpegController,SIGNAL(passFinished(int)),this,SLOT(encodePassFinished(int)), Qt::UniqueConnection);
}

InputStream MainWindow::getSelectedStream(InputStream::StreamType streamType) const
{
    if(streamType == InputStream::VIDEO)
        return getStreamByType(streamType, ui->streamVideoComboBox->currentIndex() - 1);
    else if(streamType == InputStream::AUDIO)
        return getStreamByType(streamType, ui->streamAudioComboBox->currentIndex() - 1);
    else if(streamType == InputStream::SUBTITLE)
        return getStreamByType(streamType, ui->streamSubtitlesComboBox->currentIndex() - 1);

    throw std::invalid_argument("streamType");
}

InputStream MainWindow::getStreamByType(InputStream::StreamType streamType, int index) const
{
    if(index < 0)
    {
        return InputStream();
    }

    int streamCounter = index;

    for(int i = 0; i < inputFile->streamCount(); i++)
    {
        const InputStream stream = inputFile->stream(i);

        if(stream.type() == streamType)
        {
            // If text subtitles are disabled, do not count them.
            if(streamType == InputStream::SUBTITLE &&
                    textSubtitlesDisabled &&
                    !stream.isImageSub())
                continue;

            if(streamCounter == 0)
                return stream;
            --streamCounter;
        }
    }

    return InputStream();
}

QStringList MainWindow::generatePass(int passNumber) const
{
    QString inputFilePath = inputFile->filePath();
    QString outputFilePath = outputFile->filePath();

    // get streams
    InputStream videoStream = getSelectedStream(InputStream::VIDEO);
    InputStream audioStream = getSelectedStream(InputStream::AUDIO);
    InputStream subtitleStream = getSelectedStream(InputStream::SUBTITLE);

    OutputFile::OutputVideoCodec videoCodec = static_cast<OutputFile::OutputVideoCodec>(outputFile->videoCodec());
    OutputFile::OutputAudioCodec audioCodec = static_cast<OutputFile::OutputAudioCodec>(outputFile->audioCodec());

    QTime startTime, endTime;
    outputFile->setStartTime(QTime(0,0));
    outputFile->setEndTime(getOutputDuration());

    if(ui->trimStartEndRadioButton->isChecked())
    {
        outputFile->setStartTime(ui->trimStartEndStartTimeEdit->time());
        outputFile->setEndTime(ui->trimStartEndEndTimeEdit->time());
    }
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
    int width = -2; // mod2 if automatic
    int height = -2; // mod2 if automatic

    if(ui->cropCheckBox->isChecked())
    {
        cropLeft = outputFile->cropLeft();
        cropRight = outputFile->cropRight();
        cropTop = outputFile->cropTop();
        cropBottom = outputFile->cropBottom();
    }
    if(ui->resizeCheckBox->isChecked())
    {
        if(!ui->resizeWidthAutomaticCheckBox->isChecked())
            width = outputFile->width();
        if(!ui->resizeHeightAutomaticCheckBox->isChecked())
            height = outputFile->height();
    }

    int crf = -1;
    int bitRate = -1;
    QString rateMode = ui->rateModeComboBox->currentText();

    bool twoPass = rateMode == tr("Variable Bit Rate");
    bool cbr = rateMode == tr("Constant Bit Rate");

    if(rateMode == tr("Constant Bit Rate") || rateMode == tr("Variable Bit Rate") || rateMode == tr("Constrained Quality"))
    {
        if(ui->rateTargetModeComboBox->currentText() == tr("File Size"))
        {
            outputFile->setBitRateForMegabytes(ui->rateTargetFileSizeDoubleSpinBox->value());
        }
        else if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate"))
        {
            outputFile->setBitRateInKilobits(ui->rateTargetBitRateSpinBox->value());
        }

        bitRate = static_cast<int>(outputFile->bitRateInKilobits());
    }

    if(rateMode == tr("Constant Quality") || rateMode == tr("Constrained Quality"))
    {
        crf = outputFile->crf();
    }

    int audioBitRate = ui->codecAudioBitRateSpinBox->value();

    QString customFilters = outputFile->customFilters().trimmed();
    QString customParameters = outputFile->customParameters().trimmed();

    // build the string list
    QStringList passStringList = QStringList();

    // adjust target bitrate and cropping if needed
    QTime computedDuration = getOutputDuration();
    if(audioStream.isValid() && bitRate > audioBitRate)
        bitRate -= audioBitRate;

    int cropWidth = videoStream.width() - cropLeft - cropRight;
    int cropHeight = videoStream.height() - cropTop - cropBottom;
    int cropX = cropLeft;
    int cropY = cropTop;

    // lossless shortcut
    bool lossless = bitRate == -1 && crf == -1;

    // fast seeking compatible
    bool fastSeek = subtitleStream.isImageSub() || !subtitleStream.isValid();

    // input - non-seekable subtitles
    if(!fastSeek)
        passStringList << "-i" << inputFilePath;

    // seeking/trimming
    if(startTime.isValid())
    {
        passStringList << "-ss" << startTime.toString("hh:mm:ss.zzz");
    }
    if(endTime.isValid())
    {
        passStringList << "-t" << computedDuration.toString("hh:mm:ss.zzz");
    }

    // input - seekable subtitles
    if(fastSeek)
        passStringList << "-i" << inputFilePath;

    // video codec
    if(videoCodec == OutputFile::AV1)
    {
        passStringList << "-c:v" << "libaom-av1";
        passStringList << "-strict" << "experimental";
        passStringList << "-cpu-used" << QString::number(4);
    }
    else if(videoCodec == OutputFile::VP9)
        passStringList << "-c:v" << "libvpx-vp9";
    else
        passStringList << "-c:v" << "libvpx";

    // video stream
    passStringList << "-map" << "0:v:" + QString::number(videoStream.index());

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
    {
        passStringList << "-crf" << QString::number(crf);
        if(videoCodec == OutputFile::VP8)
        {
            passStringList << "-qmin" << QString::number(0);
            passStringList << "-qmax" << QString::number(50);
        }
    }

    // target bit rate
    if(!lossless && bitRate > 0)
        passStringList << "-b:v" << QString::number(bitRate).append("K");

    // threads/speed
    //passStringList << "-threads" << QString::number(1);

    // last pass exclusive parameters
    if(!twoPass || passNumber != 1)
    {
        passStringList << "-auto-alt-ref" << QString::number(1);
        passStringList << "-lag-in-frames" << QString::number(25);
        if(videoCodec == OutputFile::VP9)
            passStringList << "-speed" << QString::number(0);
    }
    else
    {
        if(videoCodec == OutputFile::VP9)
            passStringList << "-speed" << QString::number(4);
    }

    // vp8/vp9 exclusive parameters
    if(videoCodec == OutputFile::VP9)
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
        // TODO: calculate width and height of the output (based on aspect ratio)
        int pixelCount = 0;
        if(width < 0 && height < 0)
            pixelCount = videoStream.width() * videoStream.height();
        else if(width < 0)
            pixelCount = (videoStream.width() * height / videoStream.height()) * height;
        else if(height < 0)
            pixelCount = width * (videoStream.height() * width / videoStream.width());
        else
            pixelCount = width * height;

        // 4 slices for anything larger than PAL DVD, else 1
        int slices = (pixelCount > 720 * 576 ? 4 : 1);
        passStringList << "-slices" << QString::number(slices);
    }

    // filters
    QString filterChain;
    QString complexFilterChain;

    if(cropWidth < videoStream.width() || cropHeight < videoStream.height())
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
        if(subtitleStream.isImageSub())
        {
            QString subtitleID = "[0:s:" + QString::number(subtitleStream.index()) + "]";
            QString videoID = "[0:v:" + QString::number(videoStream.index()) + "]";

            complexFilterChain.append(videoID);
            complexFilterChain.append(subtitleID);
            complexFilterChain.append("overlay");
            if(!filterChain.isEmpty())
            {
                complexFilterChain.append("[vid];[vid]");
                complexFilterChain.append(filterChain);
            }
        }
        else
        {
            if(!filterChain.isEmpty())
                filterChain.append(",");

            // Bug: filenames with single quotation marks cannot be used as input files with subs enabled
            filterChain.append(QString("subtitles=" + getFilterString("'" + inputFilePath + "'") + ":si=" + QString::number(subtitleStream.index())));
        }
    }

    if(!customFilters.isEmpty())
    {
        if(!complexFilterChain.isEmpty())
        {
            if(!filterChain.isEmpty())
                complexFilterChain.append(",");
            else
                complexFilterChain.append("[vid];[vid]");
            complexFilterChain.append(customFilters);
        }
        else if(!filterChain.isEmpty())
            filterChain.append(",");

        filterChain.append(customFilters);
    }

    if(!complexFilterChain.isEmpty())
    {
        passStringList << "-filter_complex" << complexFilterChain;
    }
    else if(!filterChain.isEmpty())
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
        if(audioCodec == OutputFile::VORBIS)
            passStringList << "-c:a" << "libvorbis";
        else
            passStringList << "-c:a" << "libopus";

        passStringList << "-map" << "0:a:" + QString::number(audioStream.index());
        //passStringList << "-ac" << QString::number(2);
        passStringList << "-b:a" << QString::number(audioBitRate) + "k";
    }

    // ignore subtitle streams
    passStringList << "-sn";

    // scaling algorithm
    passStringList << "-sws_flags" << "lanczos";

    // extra parameters
    if(!customParameters.isEmpty())
        passStringList << customParameters.split(' ');

    // make webm
    passStringList << "-f" << "webm";

    // output file
    if(twoPass && passNumber == 1)
        passStringList << QDir::cleanPath(QFileInfo(outputFilePath).absolutePath() + "/temp/null");
    else
        passStringList << outputFilePath;

    return passStringList;
}

void MainWindow::updateProgressBar()
{
    // do stuff
}

void MainWindow::on_inputFileLineEdit_textChanged(const QString &arg1)
{
    QString inputFilePath = arg1;

    clearInputFileFormData();

    if(InputFile::isValid(inputFilePath))
    {
        processInputFile(inputFilePath);

        validateFormFields();
    }
}

void MainWindow::on_outputFileLineEdit_textChanged(const QString &arg1)
{
    QString outputFilePath = arg1;

    outputFile->setFilePath(outputFilePath);

    validateFormFields();
}

void MainWindow::on_outputFileBrowsePushButton_clicked()
{
    QString outputFilePath = QFileDialog::getSaveFileName(this,tr("Select Output File"),
                                ui->outputFileLineEdit->text().trimmed(),
                                tr("WebM (*.webm)"));
    if(!outputFilePath.isEmpty())
        ui->outputFileLineEdit->setText(QDir::toNativeSeparators(outputFilePath));
}

void MainWindow::on_rateModeComboBox_currentIndexChanged(const QString &arg1)
{
    QString currentMode = arg1;
    QComboBox *targetModeComboBox = ui->rateTargetModeComboBox;
    QString currentModeText = targetModeComboBox->currentText();

    if(currentMode == tr("Variable Bit Rate") || currentMode == tr("Constant Bit Rate"))
    {
        // Disable CRF selection, enable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(false);
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(currentModeText);
    }
    else if(currentMode == tr("Constant Quality"))
    {
        // Enable CRF selection, disable target mode/bit rate/file size selection
        ui->rateCRFSpinBox->setEnabled(true);
        ui->rateTargetModeComboBox->setEnabled(false);
        ui->rateTargetBitRateSpinBox->setEnabled(false);
        ui->rateTargetFileSizeDoubleSpinBox->setEnabled(false);
    }
    else if(currentMode == tr("Constrained Quality"))
    {
        // Enable all rate control selection
        ui->rateCRFSpinBox->setEnabled(true);
        targetModeComboBox->setEnabled(true);
        refreshTargetMode(currentModeText);
    }
    else if(currentMode == tr("Lossless"))
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
        ui->resizingGroupBox->setEnabled(false);
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

        ui->encodePushButton->setEnabled(true);
        ui->resizingGroupBox->setEnabled(true);
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

    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());
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

    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());
}

void MainWindow::on_trimStartEndStartTimeEdit_editingFinished()
{
    ui->trimStartEndStartChapterComboBox->setCurrentIndex(0);
    QTime startTime = ui->trimStartEndStartTimeEdit->time();
    QTime endTime = ui->trimStartEndEndTimeEdit->time();
    if(startTime >= endTime)
        ui->trimStartEndEndTimeEdit->setTime(startTime.addMSecs(1));

    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
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

    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
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
    QString inputFilePath = QFileDialog::getOpenFileName(this,tr("Select Input File"),
                                 QFileInfo(ui->inputFileLineEdit->text().trimmed()).dir().canonicalPath(),
                                 tr("All Files (*.*)"));
    if(!inputFilePath.isEmpty())
        ui->inputFileLineEdit->setText(QDir::toNativeSeparators(inputFilePath));
}

void MainWindow::on_encodePushButton_clicked()
{
    ui->encodePushButton->setEnabled(false);
    ui->scrollArea->setEnabled(false);
    ui->menuBar->setEnabled(false);

    ffmpegController->setOutputFilePath(outputFile->filePath());

    // generate the first pass of an encode
    // the second pass is generated afterwards for two-pass encodes
    QStringList firstPass = generatePass(1);

    ffmpegController->cleanTemporaryFiles();

    QDir outputDirectory = QFileInfo(outputFile->filePath()).dir();
    QDir tempDirectory = outputDirectory.canonicalPath() + "/temp";

    if(!tempDirectory.exists())
        QDir().mkdir(tempDirectory.path());

    ffmpegController->encodePass(firstPass);
#ifdef Q_OS_WIN32
    taskBarProgress->setVisible(true);
#endif
    ui->cancelPushButton->setEnabled(true);
}

void MainWindow::activateUserInterface()
{
#ifdef Q_OS_WIN32
    taskBarProgress->setVisible(false);
#endif
    ui->progressBar->setValue(0);
    ui->cancelPushButton->setEnabled(false);
    ui->encodePushButton->setEnabled(false); // output file name conflict
    ui->scrollArea->setEnabled(true);
    ui->menuBar->setEnabled(true);

    validateFormFields();
}

void MainWindow::on_rateTargetFileSizeDoubleSpinBox_editingFinished()
{
    if(inputFile->isValid())
        ui->rateTargetBitRateSpinBox->setValue(static_cast<int>(getTargetBitRate()));

    validateFormFields();
}

void MainWindow::on_rateTargetBitRateSpinBox_editingFinished()
{   
    if(inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());

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

        if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate"))
            ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());
    }

    validateFormFields();
}

void MainWindow::on_trimDurationDurationTimeEdit_editingFinished()
{
    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());

    validateFormFields();
}

void MainWindow::on_trimNoneRadioButton_clicked()
{
    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());
}

void MainWindow::on_trimStartEndRadioButton_clicked()
{
    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());
}

void MainWindow::on_trimDurationRadioButton_clicked()
{
    if(ui->rateTargetModeComboBox->currentText() == tr("Bit Rate") && inputFile->isValid())
        ui->rateTargetFileSizeDoubleSpinBox->setValue(getTargetFileSize());
}

void MainWindow::on_codecVideoComboBox_currentIndexChanged(const QString &arg1)
{
    QString codec = arg1;

    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText(tr("Variable Bit Rate")));
    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText(tr("Constant Quality")));
    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText(tr("Constant Bit Rate")));
    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText(tr("Constrained Quality")));
    ui->rateModeComboBox->removeItem(ui->rateModeComboBox->findText(tr("Lossless")));

    ui->codecAudioComboBox->removeItem(ui->codecAudioComboBox->findText(tr("Vorbis")));

    if(codec == tr("VP8"))
    {
        ui->rateCRFSpinBox->setMinimum(4);
        ui->rateCRFSpinBox->setMaximum(63);
        ui->rateModeComboBox->insertItem(1,tr("Variable Bit Rate"));
        ui->rateModeComboBox->insertItem(2,tr("Constant Bit Rate"));
        ui->rateModeComboBox->insertItem(3,tr("Constrained Quality"));

        ui->codecAudioComboBox->insertItem(2,tr("Vorbis"));
    }
    else if(codec == tr("VP9"))
    {
        ui->rateCRFSpinBox->setMinimum(0);
        ui->rateCRFSpinBox->setMaximum(63);
        ui->rateModeComboBox->insertItem(1,tr("Variable Bit Rate"));
        ui->rateModeComboBox->insertItem(2,tr("Constant Quality"));
        ui->rateModeComboBox->insertItem(3,tr("Constant Bit Rate"));
        ui->rateModeComboBox->insertItem(4,tr("Constrained Quality"));
        ui->rateModeComboBox->insertItem(5,tr("Lossless"));

        ui->codecAudioComboBox->insertItem(2,tr("Vorbis"));
    }
    else if(codec == tr("AV1"))
    {
        ui->rateCRFSpinBox->setMinimum(0);
        ui->rateCRFSpinBox->setMaximum(63);
        ui->rateModeComboBox->insertItem(1,tr("Variable Bit Rate"));
        ui->rateModeComboBox->insertItem(2,tr("Constant Quality"));
    }
}

void MainWindow::on_streamAudioComboBox_currentIndexChanged(int index)
{
    if(index == 0)
    {
        ui->codecAudioComboBox->setEnabled(false);
        ui->codecAudioBitRateSpinBox->setEnabled(false);
    }
    else
    {
        ui->codecAudioComboBox->setEnabled(true);
        ui->codecAudioBitRateSpinBox->setEnabled(true);
    }
}

void MainWindow::on_resizeWidthAutomaticCheckBox_toggled(bool checked)
{
    if(checked)
        ui->resizeWidthSpinBox->setEnabled(false);
    else
        ui->resizeWidthSpinBox->setEnabled(true);
}

void MainWindow::on_resizeHeightAutomaticCheckBox_toggled(bool checked)
{
    if(checked)
        ui->resizeHeightSpinBox->setEnabled(false);
    else
        ui->resizeHeightSpinBox->setEnabled(true);
}

void MainWindow::on_resizeCheckBox_toggled(bool checked)
{
    if(checked)
    {
        if(ui->resizeWidthAutomaticCheckBox->isChecked())
            ui->resizeWidthSpinBox->setEnabled(false);
        if(ui->resizeHeightAutomaticCheckBox->isChecked())
            ui->resizeHeightSpinBox->setEnabled(false);
    }
}

void MainWindow::on_actionExit_triggered()
{
    ffmpegController->killProcess();

    this->close();
}

void MainWindow::on_cancelPushButton_clicked()
{
    ffmpegController->killProcess();
    ui->cancelPushButton->setEnabled(false);
}

void MainWindow::on_codecAudioComboBox_currentIndexChanged(const QString &arg1)
{
    QString selectedCodec = arg1;
    if(selectedCodec == tr("Opus"))
    {
        ui->codecAudioBitRateSpinBox->setMinimum(6);
        ui->codecAudioBitRateSpinBox->setMaximum(510);
    }
    else if(selectedCodec == tr("Vorbis"))
    {
        ui->codecAudioBitRateSpinBox->setMinimum(45);
        ui->codecAudioBitRateSpinBox->setMaximum(500);
    }
}

double MainWindow::getTargetFileSize() const
{
    if(inputFile->bitRateInKilobits() == 0 || ui->rateTargetBitRateSpinBox->value() == 0)
        return 0;

    return inputFile->fileSizeInMegabytes(getOutputDuration())
            / inputFile->bitRateInKilobits() * ui->rateTargetBitRateSpinBox->value();
}

double MainWindow::getTargetBitRate() const
{
    if(inputFile->bitRateInKilobits() == 0 || ui->rateTargetFileSizeDoubleSpinBox->value() == 0.0)
        return 0;

    return ui->rateTargetFileSizeDoubleSpinBox->value() * 1024
            / QTime(0,0).msecsTo(getOutputDuration()) * 1000 * 8;
}

QString MainWindow::getFilterString(QString rawString) const
{
    return rawString.replace(":","\\:").replace("'","\\'").replace("[","\\[").replace("]","\\]").replace(",","\\,")
    .replace(";","\\;");//.replace("\\","\\\\").replace("\\\\'","\\\\\\'"));
}

void MainWindow::encodeFailed(bool crashed)
{
    if(crashed)
        QMessageBox::critical(this,tr("Failure"),tr("The encode has failed unexpectedly."), QMessageBox::Ok);

    activateUserInterface();
}

void MainWindow::encodePassFinished(int passNumber)
{
    if(passNumber == 1)
    {
        QStringList secondPass = generatePass(2);
        ui->progressBar->setValue(50);
        ffmpegController->encodePass(secondPass);
    }
    else if(passNumber == 2)
    {
        encodeFinished();
    }
}

void MainWindow::encodeFinished()
{
    ui->progressBar->setValue(100);
    QMessageBox::information(this,tr("Success"),tr("Encode successful."),
                             QMessageBox::Ok);

    activateUserInterface();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
