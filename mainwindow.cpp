#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSerialPortInfo>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mRcvFile(NULL),
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
    if (NULL != mRcvFile) {
        if (mRcvFile->isOpen())
            mRcvFile->close();
        delete mRcvFile;
    }

    if (NULL != mTimer) {
        delete mTimer;
        mTimer = NULL;
    }

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
#include <QDateTime>

void MainWindow::readData() {
    mRxDatas = mComPort->readAll();
    QString ret;

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz:");

    QTextCursor cursor = ui->rcvTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->rcvTextEdit->setTextCursor(cursor);

    if (Qt::CheckState::Checked == ui->rcvHexCheckBox->checkState()) {
        ret = mRxDatas.toHex().toUpper();
        int len = ret.length()/2 + 1;
        for(int i=1;i<len;i++)
            ret.insert(2*i+i-1," ");
    } else {
        ret = QString(mRxDatas);
    }

    if (Qt::CheckState::Checked == ui->rcvSysTimeCheckBox->checkState())
        ret = current_date + ret;

    if (NULL != mRcvFile && mRcvFile->isOpen()) {
        mRcvFile->write(ret.toStdString().c_str());
    } else {
        ui->rcvTextEdit->textCursor().insertText(ret);
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
    QString filename = ui->rcvFileNameLineEdit->text();
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        filename = QFileDialog::getSaveFileName(this, "文件另存为", "", tr("Recved Data (*.txt)"));
        ui->rcvFileNameLineEdit->setText(filename);

        file.setFileName(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, tr("Critical Error"), "无法打开：[" + filename + "]");
            return;
        }
    }

    file.write(ui->rcvTextEdit->toPlainText().toStdString().c_str());
    file.close();

    QMessageBox::information(this, tr("保存成功"), "成功写入文件[" + filename + "]");
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
/*
 * on_rcvFileSelectButton_clicked - 选择接收文件
 */
void MainWindow::on_rcvFileSelectButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "选择存储文件", "", tr("Recved Data (*.txt)"));
    QFileInfo info(filename);
    if (info.isDir()) {
        QMessageBox::critical(this, tr("Critical Error"), "非法文件：[" + filename + "]");
        filename.clear();
    }
    ui->rcvFileNameLineEdit->setText(filename);
}
/*
 * on_rcvToFileCheckBox_toggled - 是否输出到文件
 */
void MainWindow::on_rcvToFileCheckBox_toggled(bool checked)
{
    if (checked) {
        QString filename = ui->rcvFileNameLineEdit->text();
        mRcvFile = new QFile(filename);
        if (!mRcvFile->open(QIODevice::WriteOnly)) {
            filename = QFileDialog::getSaveFileName(this, "文件另存为", "", tr("Recved Data (*.txt)"));
            ui->rcvFileNameLineEdit->setText(filename);

            mRcvFile->setFileName(filename);
            if (!mRcvFile->open(QIODevice::WriteOnly)) {
                QMessageBox::critical(this, tr("Critical Error"), "无法打开：[" + filename + "]");
                delete mRcvFile;
                mRcvFile = NULL;
                return;
            }
        }

        ui->rcvFileNameLineEdit->setEnabled(false);
        ui->rcvFileSelectButton->setEnabled(false);
        ui->rcvSaveButton->setEnabled(false);
        ui->rcvTextEdit->setText(QString("输出数据到文件[") + filename + QString("]"));
    } else {
        if (NULL != mRcvFile) {
            if (mRcvFile->isOpen())
                mRcvFile->close();
            delete mRcvFile;
            mRcvFile = NULL;
        }
        ui->rcvFileNameLineEdit->setEnabled(true);
        ui->rcvFileSelectButton->setEnabled(true);
        ui->rcvSaveButton->setEnabled(true);
        ui->rcvTextEdit->clear();
    }
}


void MainWindow::on_sndNewLineCheckBox_2_clicked(bool checked)
{
    if (checked) {
        if (NULL != mTimer) {
            delete mTimer;
            mTimer = NULL;
        }

        int ms = ui->mTimeLineEdit->text().toInt();
        mTimer = new QTimer();
        mTimer->start(ms);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(on_Timer_overflow()));
    } else {
        if (NULL != mTimer) {
            delete mTimer;
            mTimer = NULL;
        }
    }
}

void MainWindow::on_Timer_overflow()
{
    on_sndSendButton_clicked();
}
