#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    digitBTNs = {{Qt::Key_0, ui->btnNum0},
        {Qt::Key_1, ui->btnNum1},
        {Qt::Key_2, ui->btnNum2},
        {Qt::Key_3, ui->btnNum3},
        {Qt::Key_4, ui->btnNum4},
        {Qt::Key_5, ui->btnNum5},
        {Qt::Key_6, ui->btnNum6},
        {Qt::Key_7, ui->btnNum7},
        {Qt::Key_8, ui->btnNum8},
        {Qt::Key_9, ui->btnNum9},
    };

    foreach (auto btn, digitBTNs) {
        connect(btn, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    }

    bOperatorKeys = {{Qt::Key_Plus, ui->btnPlus},
        {Qt::Key_Minus, ui->btnMinus},
        {Qt::Key_Asterisk, ui->btnMultiple},
        {Qt::Key_Slash, ui->btnDivide},
    };

    foreach (auto bok, bOperatorKeys) {
        connect(bok, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    }

    // uOperatorKeys = {{Qt::Key_Percent,ui->btnPercentage},
    //                 {Qt::Key_,ui->btnInverse},
    //                 {Qt::Key_Percent,ui->btnSquare},
    //                 {Qt::Key_Percent,ui->btnSqrt},}

    // connect(ui->btnNum0, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum1, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum2, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum3, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum4, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum5, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum6, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum7, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum8, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    // connect(ui->btnNum9, SIGNAL(clicked()), this, SLOT(btnNumClicked()));

    // connect(ui->btnPlus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    // connect(ui->btnMinus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    // connect(ui->btnMultiple, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    // connect(ui->btnDivide, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));

    connect(ui->btnPercentage, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnInverse, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnSquare, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
    connect(ui->btnSqrt, SIGNAL(clicked()), this, SLOT(btnUnaryOperatorClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{
    double result = 0;
    if (operands.size() == 2 && opcodes.size() > 0) {
        //取操作数
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operands.front().toDouble();
        operands.pop_front();

        //取操作符
        QString op = opcodes.front();
        opcodes.pop_front();

        if (op == "+") {
            result = operand1 + operand2;
        } else if (op == "-") {
            result = operand1 - operand2;
        } else if (op == "×") {
            result = operand1 * operand2;
        } else if (op == "➗") {
            result = operand1 / operand2;
        }
        operands.push_back(QString::number(result));

        ui->statusbar->showMessage(QString("calcution is in progress: operands is %1,opcodes is %2").arg(operands.size()).arg(
                                       opcodes.size()));
    } else
        ui->statusbar->showMessage(QString("operands is %1,opcodes is %2").arg(operands.size()).arg(opcodes.size()));

    return QString::number(result);
}

void MainWindow::btnNumClicked()
{
    QString digit = qobject_cast<QPushButton *>(sender())->text();

    if (digit == "0" && operand == "0")
        digit = "";

    if (operand == "0" && digit != "0")
        operand = "";

    operand += digit;
    ui->display->setText(operand);
    //ui->statusbar->showMessage(qobject_cast<QPushButton *>(sender())->text() + "btn clicked");

}


void MainWindow::on_btnPeriod_clicked()
{
    if (!operand.contains(".")) {
        operand += qobject_cast<QPushButton *>(sender())->text();
    }
    ui->display->setText(operand);
}


void MainWindow::on_btnDel_clicked()
{
    operand = operand.left(operand.length() - 1);
    ui->display->setText(operand);
}


void MainWindow::on_btnClear_clicked()
{
    operands.clear();
    opcodes.clear();
    operand.clear();
    ui->display->setText(operand);
}

void MainWindow::btnBinaryOperatorClicked()
{
    QString opcode = qobject_cast<QPushButton *>(sender())->text();

    qDebug() << operands.size();
    qDebug() << opcodes.size();
    qDebug() << operand;
    if (operand != "") {
        qDebug() << "有添加符号";
        operands.push_back(operand);
        operand = "";

        opcodes.push_back(opcode);
        qDebug() << operands.size();
        qDebug() << opcodes.size();
        qDebug() << operand;
    } else if (operand == "" && operands.size() > 0) {
        opcodes.push_back(opcode);
    }

    QString result = calculation(NULL);
    ui->display->setText(result);

}

void MainWindow::btnUnaryOperatorClicked()
{
    QString text = ui->display->text();
    if (text != "") operand = text;
    if (operand != "") {
        double result = operand.toDouble();
        operand = "";

        QString op = qobject_cast<QPushButton *>(sender())->text();

        if (op == "%")
            result /= 100.0;
        else if (op == "1/x")
            result = 1 / result;
        else if (op == "x^2")
            result *= result;
        else if (op == "√")
            result = sqrt(result);

        ui->display->setText(QString::number(result));
    }
    qDebug() << operands.size();
    qDebug() << opcodes.size();
    qDebug() << operand;
}


void MainWindow::on_btnEqual_clicked()
{
    if (operand == "" and operands.size() == 0) {
        ui->display->setText("0");
    }
    qDebug() << operands.size();
    qDebug() << opcodes.size();
    qDebug() << operand;
    if (operand != "" and operands.size() == 1 and opcodes.size() == 1) {
        double result = 0;
        QString op = opcodes.front();
        opcodes.pop_front();
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operand.toDouble();
        operand = "";
        if (op == "+") {
            result = operand1 + operand2;
        } else if (op == "-") {
            result = operand1 - operand2;
        } else if (op == "×") {
            result = operand1 * operand2;
        } else if (op == "➗") {
            result = operand1 / operand2;
        }
        // operand = QString::number(result);
        operands.push_back(QString::number(result));
        ui->display->setText(QString::number(result));
    }
    qDebug() << operands.size();
    qDebug() << opcodes.size();
    qDebug() << operand;
}


void MainWindow::on_btnSign_clicked()
{
    QString text = ui->display->text();
    if (text != "" && operand == "") {
        operand = text;
        operands.pop_front();
    }
    if (operand != "") {
        double result = operand.toDouble();

        result = -result;
        operand = QString::number(result);
        ui->display->setText(QString::number(result));
    }
    qDebug() << operands.size();
    qDebug() << opcodes.size();
    qDebug() << operand;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    foreach (auto btnKey, digitBTNs.keys()) {
        if (event->key() == btnKey)
            digitBTNs[btnKey]->animateClick();
    }
    foreach (auto bOKey, bOperatorKeys.keys()) {
        if (event->key() == bOKey)
            bOperatorKeys[bOKey]->animateClick();
    }
    if (event->key() == Qt::Key_Equal) {
        ui->btnEqual->animateClick();
    }
    if (event->key() == Qt::Key_Backspace) {
        ui->btnDel->animateClick(); // 调用退格功能
    }
    if (event->key() == Qt::Key_Percent) {
        ui->btnPercentage->animateClick();
    }
    if (event->key() == Qt::Key_Period) {
        ui->btnPeriod->animateClick();
    }
}

