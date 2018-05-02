#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDockWidget>
#include <QTextEdit>

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

private:
    Ui::MainWindow *ui;

    QSerialPort *mComPort;

    QByteArray mRxDatas;
    bool isPortOpen;
};

#endif // MAINWINDOW_H
