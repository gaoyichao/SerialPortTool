#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDockWidget>
#include <QTextEdit>
#include <QFileDialog>
#include <QTimer>

namespace Ui {
class MainWindow;
}

enum ComStatus {
    expectHead,
    expectTid,
    expectLen,
    expectData,
    expectCheck
};

enum ToolMode {
    rcvMode,
    hexMode
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void readData();
    void HandleError(QSerialPort::SerialPortError error);
    void OpenSerialPort();
    void CloseSerialPort();

    void CheckCustomBaudRatePolicy(int idx);


    void on_openButton_clicked();

    void on_rcvCleanButton_clicked();

    void on_rcvHexCheckBox_toggled(bool checked);

    void on_rcvSaveButton_clicked();

    void on_sndSendButton_clicked();

    void on_rcvFileSelectButton_clicked();

    void on_rcvToFileCheckBox_toggled(bool checked);

    void on_sndNewLineCheckBox_2_clicked(bool checked);

    void on_Timer_overflow();

    void on_hexFileSelectButton_clicked();
    void on_hexProgramButton_clicked();

private:
    void rcvModeRcvData();
    void hexModeRcvData();
private:
    Ui::MainWindow *ui;

    QSerialPort *mComPort;
    QFile *mRcvFile;
    QFile *mHexFile;
    QTimer *mTimer = NULL;

    QByteArray mRxDatas;
    bool isPortOpen;

    enum ToolMode mMode = rcvMode;
};

#endif // MAINWINDOW_H
