#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtWidgets>
#include <QtWinExtras>
#include "inputfile.h"
#include "outputfile.h"
#include "inputstream.h"

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
    ~MainWindow();

protected:
    void showEvent(QShowEvent *e);
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
    void on_rateTargetFileSizeDoubleSpinBox_editingFinished();
    void on_rateTargetBitRateSpinBox_editingFinished();
    void on_trimDurationStartTimeEdit_editingFinished();
    void on_trimDurationDurationTimeEdit_editingFinished();
    void on_trimNoneRadioButton_clicked();
    void on_codecVideoComboBox_currentIndexChanged(const QString &arg1);
    void on_streamAudioComboBox_currentIndexChanged(int index);
    void on_resizeWidthAutomaticCheckBox_toggled(bool checked);
    void on_resizeHeightAutomaticCheckBox_toggled(bool checked);
    void on_resizeCheckBox_toggled(bool checked);
    void on_actionExit_triggered();
    void encodePassFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_cancelPushButton_clicked();
    void on_codecAudioComboBox_currentIndexChanged(const QString &arg1);
    void on_trimStartEndRadioButton_clicked();
    void on_trimDurationRadioButton_clicked();

private:
    Ui::MainWindow *ui;
#ifdef Q_OS_WIN32
    QWinTaskbarButton *taskBarButton;
    QWinTaskbarProgress *taskBarProgress;
#endif
    InputFile *inputFile;
    OutputFile *outputFile;
    QProcess *ffmpegProcess;
    void refreshTargetMode(QString &currentTargetMode);
    void processInputFile(QString &inputFilePath);
    void clearInputFileFormData();
    void populateStreamComboBoxes();
    void initializeFormData();
    QStringList generatePass(int passNumber, bool twoPass = true);
    void encodePass(QStringList &encodingParameters);
    void validateFormFields();
    void updateProgressBar();
    QTime getOutputDuration();
    void connectSignalsAndSlots();
    void cleanTemporaryFiles();
    void activateUserInterface();
    bool passFileExists();
    double getTargetFileSize();
    QString getFilterString(QString rawString);
};

#endif // MAINWINDOW_H
