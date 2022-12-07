#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "terminal.h"
#include "generator.h"
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <utility>
#include <stack>
#include <QStack>


class Item
{
public:
    int pdctId;//production id
    int pointPos;//. position
    friend bool operator ==(const Item& one, const Item& other);
    friend bool operator <(const Item& one, const Item& other);
};

typedef set<Item> ItemSet;
typedef pair<int, Symbol> GOTO;

struct DFA
{
    list<ItemSet> stas;
    map<GOTO, int> GTMap;
};

enum Behave { reduct, shift, accept, error };
struct Behavior
{
    Behave behavior;
    int nextStat;
};

#ifndef PORDUCTION_H
#define PORDUCTION_H
class Production{
public:
    int id;
    Symbol left;
    vector<Symbol>right;
};
#endif // PORDUCTION_H

#ifndef PARSER_H
#define PARSER_H

//语法分析和语义分析
class Parser{
public:
    Parser(){};
    void Analysis(Terminal tm,MainWindow *mw);//初始化，语法分析，语义分析
    void clear();
    friend bool operator ==(const set<pair<string,string>>&one, const set<pair<string,string>>& other);
    IR iterRps;


private:
    //syntax anlysis
    vector<Production>productions;
    map<Symbol, set<Symbol>> first;//由Parse.conf构造出的first集
    map<Symbol, set<Symbol>> follow;
    DFA dfa;
    map<GOTO, Behavior> SLR1subTable;
    map<int,set<pair<string,string>>> SLR1_table;
    bool readProduction(Terminal tm,MainWindow *mw);
    void getFirst();
    void getFollow();
    bool createDFA(MainWindow *mw);
    void convert2Table();
    ItemSet derive(Item item);
    void showFirst(MainWindow *mw);
    void showFollow(Terminal tm,MainWindow *mw);
    void showSLR1Table(MainWindow *mw);
    void showProductions();

    //symatic anlysis
    stack<Symbol*> symbolStack;
    stack<int> statusStack;
    vector<Var> varTable;//include array
    vector<Array> arrTable;//include array's detail
    vector<Func> funcTable;
    NewTemper newTemper;
    Symbol* popSymbol();
    void pushSymbol(Symbol* sym,MainWindow *mw);
    Var* CheckVar(string ID);
    Array* CheckArray(string ID);
    Func* CheckFunc(string ID);
    bool SematicAnalyze(Terminal tm,MainWindow *mw);
    bool paraSizeMatch(list<string>&argument_list, list<DataType>&parameter_list);
    vector<pair<int, string> > getFuncEnter();
    list<int> merge(list<int>&l1, list<int>&l2);

};
#endif // PARSER_H
