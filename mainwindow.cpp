#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include "algo.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(pb_clicked()));
}

void MainWindow::pb_clicked(){
    std::vector<int> vec;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++){
        for (int j = 0; j < ui->tableWidget->columnCount(); j++){
            vec.push_back(ui->tableWidget->item(i,j)->text().toInt());
        }
    }
    vec = run(ui->lineEdit_2->text().toInt(), ui->lineEdit_3->text().toInt(),vec, ui->tableWidget->rowCount(), ui->tableWidget->columnCount(), ui->lineEdit);
    for (int i = 0; i < ui->tableWidget_2->rowCount(); i++){
        for (int j = 0; j < ui->tableWidget_2->columnCount(); j++){
            ui->tableWidget_2->item(i,j)->setText(QString::number(vec[i*ui->tableWidget_2->rowCount()+j]));
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
