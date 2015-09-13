#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private slots:
    void on_actionAbout_triggered();
    void on_inputFileLineEdit_textChanged(const QString &arg1);
    void on_outputFileLineEdit_textChanged(const QString &arg1);
    void on_inputFileBrowsePushButton_clicked();
    void on_outputFileBrowsePushButton_clicked();
    void on_rateModeComboBox_currentIndexChanged(const QString &arg1);
    void on_rateTargetModeComboBox_currentIndexChanged(const QString &arg1);

    void on_streamVideoComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    bool validateInputFile(QString &inputFilePath);
    bool validateOutputFile(QString &outputFilePath);
    void refreshTargetMode(QString &currentTargetMode);
    void processInputFile(QString &inputFilePath);
    void clearInputFileFormData();
    AVFormatContext *openInputFile(QString &inputFilePath);
    void closeInputFile(AVFormatContext *MainWindowformatContext);
    void populateStreamComboBoxes(AVFormatContext *formatContext);
    void initializeFormData(AVFormatContext *formatContext);
};

#endif // MAINWINDOW_H
