#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>
#include <vector>
using namespace std;

enum keyword{
    LT=1 ,LE ,GT ,GE ,VL,EQ ,NEQ,
    ADD ,DEC ,MUL ,DIV ,
    SEM ,AND,OR,
    LB , RB , BLB ,BRB, MLB, MRB
    ,WHILE ,DO ,IF ,ELSE ,FOR , RETURN
    ,ID ,NUMBER
    ,INT8 ,INT10 ,INT16
    ,FLOATNUM
    ,VOID ,INT ,FLOAT
    ,COM,ERRORCHAR,END
};

class Terminal{
public:
    int IDnum;
    vector<pair<keyword,string>> termVec;
    void insert(keyword kd,string x,bool isID=false){
        this->termVec.push_back({kd,x});
        if(isID){
            this->IDnum++;
        }
    }
    Terminal(){
        this->IDnum=0;
    }
    void reset(){
        this->IDnum=0;
        this->termVec.clear();
    }
};
#endif // TERMINAL_H
