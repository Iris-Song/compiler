#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "generator.h"
#include <queue>
#include <iostream>
#include <fstream>
#include <iomanip>

class Quadruple{
public:
    string op;
    string src1;
    string src2;
    string dst;
    Quadruple(string op,string src1,string src2,string dst){
        this->op=op;
        this->src1=src1;
        this->src2=src2;
        this->dst=dst;
    }
    Quadruple(){};
};

class Block
{
public:
    string name;//基本块的名称
    vector<Quadruple> codes;//基本块中的四元式
    int next1;//基本块的下一连接块
    int next2;//基本块的下一连接块
};

class NewLabeler
{
private:
    int index;
public:
    NewLabeler(){index = 1;};
    string newLabel(){return string("Label") + to_string(index++);};
    void reset(){index=1;}
};

#ifndef IR_H
#define IR_H
class IR //intermediate representation
{
private:
    vector<Quadruple> codeVec;
    map<string, vector<Block>>funcBlocks;
    NewLabeler newLabel;

public:
    void emitt(Quadruple q);
    void backPatch(list<int>nextList,int quad);
    int nextQuad();
    void divideBlocks(vector<pair<int, string>> funcEnter);
    void output(MainWindow* mw);
    void clear();
    friend class Generator;
};
#endif // IR_H

#ifndef SYMBOL_H
#define SYMBOL_H
class Symbol{
public:
    bool isTerm;
    string content;
    friend bool operator ==(const Symbol& one, const Symbol& other);
    friend bool operator < (const Symbol& one, const Symbol& other);
    Symbol(const Symbol& sb);

    Symbol(const bool& isTerm, const string& content);
    Symbol(){}
};
#endif // SYMBOL_H

#ifndef SEMATIC_SYM
#define SEMATIC_SYM

//声明类型（变量声明/函数声明）
enum DecType
{
    DEC_VAR, DEC_FUN
};

//数据类型（int/void）
enum DataType { D_VOID, D_INT };

struct Var
{
    string name;
    DataType type;
    int level;
    int dimension;//0:普通变量，1:一维数组，2:二维数组
};

struct Func
{
    string name;
    DataType returnType;
    list<DataType> paramTypes;
    int enterPoint;
};

class Id :public Symbol
{
public:
    string name;
    Id(const Symbol& sym, const string& name);
};

class Num :public Symbol
{
public:
    string number;
    Num(const Symbol& sym, const string& number);
};

class FuncDeclare :public Symbol
{
public:
    list<DataType>funcList;
    FuncDeclare(const Symbol& sym):Symbol(sym) {};
};

class Parameter :public Symbol
{
public:
    list<DataType>plist;
    Parameter(const Symbol& sym):Symbol(sym) {};
};

class ParaList :public Symbol
{
public:
    list<DataType>plist;
    ParaList(const Symbol& sym):Symbol(sym) {};
};

class StcBlock :public Symbol
{
public:
    list<int>nextList;
    StcBlock(const Symbol& sym):Symbol(sym) {};
};

class StcList :public Symbol
{
public:
    list<int>nextList;
    StcList(const Symbol& sym):Symbol(sym) {};
};

class Sentence :public Symbol
{
public:
    list<int>nextList;
    Sentence(const Symbol& sym):Symbol(sym) {};
};

class WhileStc :public Symbol
{
public:
    list<int>nextList;
    WhileStc(const Symbol& sym):Symbol(sym) {};
};

class BoolStc :public Symbol
{
public:
    list<int>falseList;
    BoolStc(const Symbol& sym):Symbol(sym) {};
};

class IfStc :public Symbol
{
public:
    list<int>nextList;
    IfStc(const Symbol& sym):Symbol(sym) {};
};

class BoolAnd :public Symbol
{
public:
    list<int>falseList;
    BoolAnd(const Symbol& sym):Symbol(sym) {};
};

class BoolOr :public Symbol
{
public:
    list<int>falseList;
    BoolOr(const Symbol& sym):Symbol(sym) {};
};

class Exp :public Symbol
{
public:
    string name;
    list<int>falseList;
    Exp(const Symbol& sym):Symbol(sym) {};
};

class M :public Symbol
{
public:
    int quad;
    M(const Symbol& sym):Symbol(sym) {};
    M(){};
};

class N :public Symbol
{
public:
    list<int> nextList;
    N(const Symbol& sym):Symbol(sym) {};
    N(){};
};

class AddExp :public Symbol
{
public:
    string name;
    AddExp(const Symbol& sym):Symbol(sym) {};
};

class ItemSym :public Symbol
{
public:
    string name;
    ItemSym(const Symbol& sym):Symbol(sym) {};
};

class Factor :public Symbol
{
public:
    string name;
    Factor(const Symbol& sym):Symbol(sym) {};
};

class ArguList :public Symbol
{
public:
    list<string> alist;
    ArguList(const Symbol& sym):Symbol(sym) {};
};

class Array :public Symbol
{
public:
    string name;
    string result;
    int dimension;
    string dm1_size,dm2_size;
    Array(string name, int dimension,string dm1_size,string dm2_size);
    Array(const Symbol& sym, const string& name);
    Array(const Symbol& sym):Symbol(sym) {};
};

class NewTemper
{
private:
    int number;
public:
    NewTemper(){ number = 1;};
    string newTemp(){ return string("T") + to_string(number++);};
};
#endif // SEMATIC_SYM
