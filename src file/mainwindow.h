#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <string>
#include <QFont>
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    friend class Lexer;
    friend class Parser;
    friend class IR;
    friend class Generator;

private slots:
   void on_actionOpenFile_triggered();
   void on_actionHow_to_use_triggered();

   void on_lexButton_clicked();
   void on_synButton_clicked();
   void on_codeButton_clicked();

private:
    Ui::MainWindow *ui;
    QString fileName;

};
#endif // MAINWINDOW_H
