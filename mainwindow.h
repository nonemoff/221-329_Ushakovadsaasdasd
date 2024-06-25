#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QDateTime>
#include <QCryptographicHash>
#include <fstream>
#include <string>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Move {
    int row;
    int column;
    QString dateTime;
    QString hash;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleCellClick(int row, int column);
    void handleResetClick();
    void handleLoadClick();

private:
    Ui::MainWindow *ui;
    int clickCounter;
    QString previousHash;
    std::vector<Move> moves;

    std::string calculateHash(int row, int column, const QString &dateTime, const QString &prevHash);
    void logMove(int row, int column);
    void initializeLog();
};

#endif // MAINWINDOW_H
