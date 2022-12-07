#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lexer.h"
#include "parser.h"
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <string>
#include <iostream>
#include <stdlib.h>
using namespace std;

Lexer lx;
Parser *p=nullptr;
Generator *pgen=nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenFile_triggered()
{
    fileName = QFileDialog::getOpenFileName(
                   this, tr("open file"),
                   "./", tr("All files (*.*)"));

    if(fileName.isEmpty()||fileName.isNull())
    {
       QMessageBox mesg;
       mesg.warning(this,"warning","fail to open a file");
       return;
    }
    else
    {
       QFile file(fileName);
       QTextStream readStream(&file);
       if(!file.open(QIODevice::ReadOnly | QIODevice::Text))//只读
       {
           QMessageBox mesg;
           mesg.warning(this,"warning","fail to open a file");
       }
       ui->textEdit->setPlainText(readStream.readAll());
    }
}

void MainWindow::on_actionHow_to_use_triggered()
{
    QMessageBox mesg;
    mesg.information(this,"How to use","click <b>file</b> to open the source and you will see the text in <b>input</b>.Typing in input box is also applicable.Then click <b>lexical analysis</b> button ,"
                                       "you will see lexical result below<br><br>"
                     "As for syntax and sematic analysis, the rules of production have already declared in \"production.conf\" file, which you can also change it using SLR(1) rules."
                     "Click <b>syntax and sematic analysis</b> button to select configuration file then see result of synatx and itermediate representation.<br><br>"
                     "After successful sematic analysis, click <b>code generation</b> button to get MIPS instructions.<br><br>"
                     "You can also find intermediate representation in \"InterCode.txt\", object code in \"objectCode.asm\"<br><br>"
                     );

}

void MainWindow::on_lexButton_clicked()
{
    ui->lexerBrowser->clear();
    lx.lexer(this);
    return;
}

void MainWindow::on_synButton_clicked()
{
    ui->ParserBrowser->clear();
    ui->IRBrowser->clear();
    static Parser ps;
    ps.clear();
    ps.Analysis(lx.term,this);
    p = &ps;
}

void MainWindow::on_codeButton_clicked()
{
    ui->CodeBrowser->clear();
    if(!p){
        ui->CodeBrowser->append("<font color='red'><b>have not generator intermediate code!</b></font>");
        return;
    }
    Generator gen(&p->iterRps,this);
}
