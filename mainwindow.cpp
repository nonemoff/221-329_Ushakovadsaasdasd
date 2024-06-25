#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , clickCounter(0)
    , previousHash("")
{
    ui->setupUi(this);

    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::handleCellClick);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::handleResetClick);
    connect(ui->loadButton, &QPushButton::clicked, this, &MainWindow::handleLoadClick);

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
            QTableWidgetItem* item = ui->tableWidget->item(row, column);
            if (item) {
                item->setBackground(Qt::white);
            } else {
                item = new QTableWidgetItem();
                item->setBackground(Qt::white);
                ui->tableWidget->setItem(row, column, item);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleCellClick(int row, int column)
{
    QTableWidgetItem* item = ui->tableWidget->item(row, column);
    if (!item) {
        item = new QTableWidgetItem();
        ui->tableWidget->setItem(row, column, item);
    }

    if (item->background() != Qt::white)
        return;

    if (clickCounter == 0){
        initializeLog();
    }

    clickCounter++;

    if (clickCounter % 2 == 0) {
        item->setBackground(Qt::green);
    } else {
        item->setBackground(Qt::red);
    }

    if (clickCounter == 16) {
        handleResetClick();
    }

    logMove(row, column);
}

void MainWindow::handleResetClick()
{
    clickCounter = 0;
    previousHash = "";
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
            QTableWidgetItem* item = ui->tableWidget->item(row, column);
            if (item) {
                item->setBackground(Qt::white);
            } else {
                item = new QTableWidgetItem();
                item->setBackground(Qt::white);
                ui->tableWidget->setItem(row, column, item);
            }
        }
    }
}

std::string MainWindow::calculateHash(int row, int column, const QString &dateTime, const QString &prevHash)
{
    QString data = QString::number(row) + QString::number(column) + dateTime + prevHash;
    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex().toStdString();
}

void MainWindow::logMove(int row, int column)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeStr = dateTime.toString("yyyy.MM.dd_HH:mm:ss");

    std::string hash = calculateHash(row, column, dateTimeStr, previousHash);
    previousHash = QString::fromStdString(hash);

    std::ofstream logFile("game_log.csv", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << row << "," << column << "," << dateTimeStr.toStdString() << "," << hash << "\n";
        logFile.close();
    }
}

void MainWindow::initializeLog()
{
    QFile file("game_log.csv");
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.close();
    }
    std::ofstream logFile("game_log.csv", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "Row,Column,DateTime,Hash\n";
        logFile.close();
    }

}

void MainWindow::handleLoadClick()
{
    QFile file("game_log.csv");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Не могу открыть файл"));
        return;
    }

    QTextStream in(&file);
    QStringList headers = in.readLine().split(',');

    QByteArray previousHash;
    moves.clear();
    int lineNumber = 1;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() != headers.size()) {
            QMessageBox::warning(this, tr("Error"), tr("Ошибка чтения файла, не совпадает количество атрибутов в ходе №%1").arg(lineNumber));
            return;
        }

        QString row = fields[0];
        QString column = fields[1];
        QString dateTime = fields[2];
        QString hash = fields[3];

        QByteArray data = row.toUtf8() + column.toUtf8() + dateTime.toUtf8() + previousHash;
        QByteArray calculatedHash = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();

        if (calculatedHash != hash.toUtf8()) {
            QMessageBox::warning(this, tr("Error"), tr("Ошибка контрольной суммы %1").arg(lineNumber));
            return;
        }
        previousHash = calculatedHash;


        QTableWidgetItem* item = ui->tableWidget->item(row.toInt(), column.toInt());
        if (lineNumber % 2 == 1) {
            item->setBackground(Qt::red);
        } else {
            item->setBackground(Qt::green);
        }

        Move move = { row.toInt(), column.toInt(), dateTime, hash };
        moves.push_back(move);

        lineNumber++;
    }
    file.close();
    QMessageBox::information(this, tr("Success"), tr("Игра успешно загружена"));
}
