#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatserver.h"
#include "historydialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startStopButton_clicked();
    void on_historyButton_clicked();

public slots:
    void logMessage(const QString &msg);

    void updateHistory();
private:
    Ui::MainWindow *ui;

    ChatServer *m_chatServer;

    HistoryDialog *dialog; // 保存对话框指针
};
#endif // MAINWINDOW_H
