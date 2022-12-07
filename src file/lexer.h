#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "terminal.h"
#include "qdebug.h"
using namespace std;

#ifndef LEXER
#define LEXER
class Lexer{
public:
    Terminal term;
    Lexer(){
        i_of_src=0;
    }
    void lexer(MainWindow *mw);
private:
    int i_of_src;
    bool IsLetter(char ch);
    bool IsDigit(char ch);
    char getNextChar(string src);
    bool scan(string src,MainWindow *mw);
    void writeout(int type,int num,MainWindow *mw);
};
#endif // LEXER
