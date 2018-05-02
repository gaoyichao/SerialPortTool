#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QSerialPortInfo>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 扫描串口
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.first(), list);
    }
    // 添加波特率
    ui->baudRateBox->addItem(QStringLiteral("460800"), 460800);
    ui->baudRateBox->addItem(QStringLiteral("2008000"), 2008000);
    ui->baudRateBox->addItem(QStringLiteral("250000"), 250000);
    ui->baudRateBox->addItem(QStringLiteral("128000"), 128000);
    ui->baudRateBox->addItem(QStringLiteral("115200"), 115200);
    ui->baudRateBox->addItem(QStringLiteral("76800"), 76800);
    ui->baudRateBox->addItem(QStringLiteral("57600"), 57600);
    ui->baudRateBox->addItem(QStringLiteral("38400"), 38400);
    ui->baudRateBox->addItem(tr("custom"));
    ui->baudRateBox->setCurrentIndex(0);
    //
    connect(ui->baudRateBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, CheckCustomBaudRatePolicy);
    //
    isPortOpen = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::OpenSerialPort()
{
    // 串口配置
    mComPort = new QSerialPort(this);
    connect(mComPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MainWindow::HandleError);
    connect(mComPort, &QSerialPort::readyRead, this, &MainWindow::readData);

    mComPort->setPortName(ui->serialPortInfoListBox->currentText());
    mComPort->setBaudRate(ui->baudRateBox->currentText().toInt());
    mComPort->setDataBits(QSerialPort::Data8);
    mComPort->setParity(QSerialPort::NoParity);
    mComPort->setStopBits(QSerialPort::OneStop);
    mComPort->setFlowControl(QSerialPort::NoFlowControl);
    if (!mComPort->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, tr("Error"), mComPort->errorString());
    }
    isPortOpen = true;
    ui->openButton->setText("关闭串口");
}
void MainWindow::CloseSerialPort()
{
    if (mComPort->isOpen())
        mComPort->close();
    isPortOpen = false;
    ui->openButton->setText("打开串口");
}

void MainWindow::CheckCustomBaudRatePolicy(int idx) {
    bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
    ui->baudRateBox->setEditable(isCustomBaudRate);

    if (isCustomBaudRate) {
        ui->baudRateBox->clearEditText();
    }
}

void MainWindow::HandleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), mComPort->errorString());
        CloseSerialPort();
    }
}


void MainWindow::on_openButton_clicked()
{
    if (false == isPortOpen) {
        OpenSerialPort();

    } else {
        CloseSerialPort();
    }
}

void MainWindow::readData() {
    mRxDatas = mComPort->readAll();

    if (Qt::CheckState::Checked == ui->rcvHexCheckBox->checkState()) {
        QString ret(mRxDatas.toHex().toUpper());
        int len = ret.length()/2 + 1;
        for(int i=1;i<len;i++)
            ret.insert(2*i+i-1," ");
        ui->rcvTextEdit->textCursor().insertText(ret);
    } else {
        ui->rcvTextEdit->textCursor().insertText(QString(mRxDatas));
    }
}

void MainWindow::on_rcvCleanButton_clicked()
{
    ui->rcvTextEdit->clear();
}

void MainWindow::on_rcvHexCheckBox_toggled(bool checked)
{
    ui->rcvTextEdit->clear();
}


void MainWindow::on_rcvSaveButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "文件另存为", "", tr("Recved Data (*.txt)"));
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Critical Error"), "无法打开：[" + filename + "]");
        return;
    }

    file.write(ui->rcvTextEdit->toPlainText().toStdString().c_str());
    file.close();

    QMessageBox::information(this, tr("保存成功"), "成功写入文件[" + filename + "]");
    ui->rcvTextEdit->append(filename);
}

void MainWindow::on_sndSendButton_clicked()
{
    QString str = ui->sndTextEdit->toPlainText();
    if (Qt::CheckState::Checked == ui->sndCRCheckBox->checkState())
        str = str + "\r";
    if (Qt::CheckState::Checked == ui->sndNewLineCheckBox->checkState())
        str = str + "\n";
    mComPort->write(str.toStdString().c_str());
}
