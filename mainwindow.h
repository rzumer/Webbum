#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_resizeCheckBox_toggled(bool checked);
    void on_cropCheckBox_toggled(bool checked);
    void on_trimStartEndRadioButton_toggled(bool checked);
    void on_trimDurationRadioButton_toggled(bool checked);
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
    void initializeStreamComboBoxes(QString &inputFilePath);
    void clearStreamComboBoxes();
};

#endif // MAINWINDOW_H
