#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtWidgets>
#include "inputfile.h"
#include "outputfile.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() { delete ui; }

protected:
    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev) { ev->accept(); }

private slots:
    void on_actionAbout_triggered();
    void on_inputFileLineEdit_textChanged(const QString &arg1);
    void on_outputFileLineEdit_textChanged(const QString &arg1);
    void on_outputFileBrowsePushButton_clicked();
    void on_rateModeComboBox_currentIndexChanged(const QString &arg1);
    void on_rateTargetModeComboBox_currentIndexChanged(const QString &arg1);
    void on_streamVideoComboBox_currentIndexChanged(int index);
    void on_trimStartEndRadioButton_toggled(bool checked);
    void on_trimStartEndStartChapterComboBox_activated(int index);
    void on_trimStartEndEndChapterComboBox_activated(int index);
    void on_trimStartEndStartTimeEdit_editingFinished();
    void on_trimStartEndEndTimeEdit_editingFinished();
    void on_cropLeftSpinBox_editingFinished();
    void on_cropRightSpinBox_editingFinished();
    void on_cropTopSpinBox_editingFinished();
    void on_cropBottomSpinBox_editingFinished();
    void on_resizeWidthSpinBox_editingFinished();
    void on_resizeHeightSpinBox_editingFinished();
    void on_actionOpen_triggered();
    void on_encodePushButton_clicked();
    void on_cancelPushButton_clicked();
    void on_rateTargetFileSizeDoubleSpinBox_editingFinished();
    void on_rateTargetBitRateSpinBox_editingFinished();
    void on_trimDurationStartTimeEdit_editingFinished();
    void on_trimDurationDurationTimeEdit_editingFinished();
    void on_trimNoneRadioButton_clicked();

private:
    Ui::MainWindow *ui;
    InputFile *inputFile;
    OutputFile *outputFile;
    void refreshTargetMode(QString &currentTargetMode);
    void processInputFile(QString &inputFilePath);
    void clearInputFileFormData();
    void populateStreamComboBoxes();
    void initializeFormData();
    QStringList generatePass(int passNumber, QString &inputFilePath, QString &outputFilePath, int videoStreamId, int audioStreamId, int subtitleStreamId, QTime startTime, QTime endTime, QTime duration, int cropLeft, int cropRight, int cropTop, int cropBottom, int width, int height, int crf, double targetFileSize, double targetBitRate, bool cbr, QString customParameters, bool twoPass);
    void encodePass(QStringList &encodingParameters);
    bool validateFormFields();
    void updateProgressBar();
    QTime getOutputDuration(int64_t inputDuration);
    void connectSignalsAndSlots();
};

#endif // MAINWINDOW_H
