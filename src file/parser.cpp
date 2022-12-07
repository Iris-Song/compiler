#include "parser.h"
#include <QDebug>
#include <QStandardPaths>
#include <QFileDialog>
#include <QCoreApplication>

bool operator ==(const Symbol&one, const Symbol&other)
{
    return one.content == other.content;
}

bool operator <(const Symbol&one, const Symbol&other)
{
    return one.content < other.content;
}

bool operator < (const Item&one, const Item& other)
{
    return pair<int, int>(one.pdctId, one.pointPos) < pair<int, int>(other.pdctId, other.pointPos);
}

bool operator ==(const Item&one, const Item& other)
{
    return one.pdctId == other.pdctId&&one.pointPos == other.pointPos;
}

bool operator ==(const set<pair<string,string>>&one, const set<pair<string,string>>& other){
    if(one.size()!=other.size()){
        return false;
    }
    else{
        set<pair<string,string>>::iterator iter1=one.begin();
        set<pair<string,string>>::iterator iter2=other.begin();
        for(;iter1!=one.end();iter1++,iter2++){
            if(iter1->first!=iter2->first||iter1->second!=iter2->second)
                return false;
        }
        return true;
    }

}

Symbol::Symbol(const Symbol& sb)
{
    this->isTerm = sb.isTerm;
    this->content = sb.content;
}

Symbol::Symbol(const bool &isTerm, const string& content)
{
    this->isTerm = isTerm;
    this->content = content;
}


//judge terminal
bool isTerm(Terminal tm,string s)
{
    //why add this？ src maynot contain every alpha
    if (s == "int" || s == "void" || s == "if" || s == "while" || s == "else" || s == "return") {
        return true;
    }
    if (s == "+" || s == "-" || s == "*" || s == "/" || s == "=" || s == "==" || s == ">" || s == "<" || s == "!=" || s == ">=" || s == "<=") {
        return true;
    }
    if (s == ";" || s == "," || s == "(" || s == ")" || s == "{" || s == "}" ||  s == "[" || s == "]" ||s == "ID" ||s == "AID" || s == "NUM"|| s=="&&"||s=="||") {
        return true;
    }
    for(int i=0;i<tm.termVec.size();i++){
        if(s==tm.termVec[i].second)
            return true;
    }

    return false;
}


void Parser::Analysis(Terminal term, MainWindow *mw){
    if(!readProduction(term,mw))return;
    getFirst();
    getFollow();
    if(createDFA(mw)){
        showFirst(mw);
        showFollow(term,mw);
//        showProductions();
        convert2Table();
        showSLR1Table(mw);
        mw->ui->ParserBrowser->append("<font color='red'><b>SUCCESSFUL!</b></font>");
        if(SematicAnalyze(term,mw)){
            iterRps.divideBlocks(getFuncEnter());
            iterRps.output(mw);
        }
    }
    else{
        showFirst(mw);
        showFollow(term,mw);
        mw->ui->ParserBrowser->append("<font color='red'><b>FAIL</b></font>");
    }
}

void Parser::clear(){
    productions.clear();
    first.clear();
    follow.clear();
    dfa.stas.clear();
    dfa.GTMap.clear();
    SLR1subTable.clear();
    SLR1_table.clear();
    while(!statusStack.empty()){
        statusStack.pop();
    }
    while(!symbolStack.empty()){
        symbolStack.pop();
    }
    varTable.clear();
    arrTable.clear();
    funcTable.clear();
    iterRps.clear();
}

bool Parser::readProduction(Terminal tm,MainWindow*mw){

    QString filePath;
    QFileInfo info(filePath);
    QFileDialog* f=new QFileDialog(mw);
    f->setWindowTitle("parser file open");
    if(f->exec()==QDialog::Accepted)
        filePath = f->selectedFiles()[0];

    ifstream fin;
    fin.open(filePath.toLatin1(), ios::in);
    //qDebug()<< filePath;
    if (!fin.is_open()){
        QMessageBox mesg;
        mesg.warning(mw,"warning","can not open "+QString(filePath)+" "
                                                                    "<br>请注意中文路径是不被允许的");
        return false;
    }
    int Pindex = 0;
    char buf[1024];
    while (fin >> buf)
    {
        Production p;
        p.id = Pindex++;
        p.left = Symbol(false,string(buf));
        //middle ::=
        fin >> buf;
        assert(strcmp(buf, "::=") == 0);
        //right production
        fin.getline(buf, 1024);
        stringstream sstream(buf);
        string temp;
        while (sstream >> temp)
            p.right.push_back(Symbol(isTerm(tm,temp),string(temp)));
        productions.push_back(p);
    }
    //qDebug()<<"psize"<<productions.size();
    return true;
}

void Parser::getFirst(){
    bool changeFlag = true;//first set change flag
    while (changeFlag)
    {
        changeFlag = false;
        //traverse each production
        for (vector<Production>::iterator iter = productions.begin(); iter != productions.end(); iter++)
        {
            vector<Symbol>::iterator rtIter;
            //traverse each right symbol
            for (rtIter = iter->right.begin(); rtIter != iter->right.end(); rtIter++)
            {
                //rtIter is terminal
                if (rtIter->isTerm)
                {
                    //if left first is empty
                    if (first.count(iter->left) == 0)
                        first[iter->left] = set<Symbol>();
                    //left first not contain right Symbol
                    if (first[iter->left].insert(*rtIter).second == true)
                        changeFlag = true;
                    break;
                }
                //rtIter is not terminal
                else
                {
                    bool continueFlag = false;//continue read next rtIter's first
                    set<Symbol>::iterator firstIter;
                    //traverse each rtIter's first
                    for (firstIter = first[*rtIter].begin(); firstIter != first[*rtIter].end(); firstIter++)
                    {
                        //if rtIter's first element EPSILON
                        if (firstIter->content == "EPSILON")
                            continueFlag = true;
                        //rtIter's first element not in left first
                        else if (first[iter->left].find(*firstIter) == first[iter->left].end())
                        {
                            if (first.count(iter->left) == 0)
                                first[iter->left] = set<Symbol>();
                            first[iter->left].insert(*firstIter);
                            changeFlag = true;
                        }
                    }
                    if (!continueFlag)
                        break;
                }
            }
            //traverse right to the end,so EPSILON in
            if (rtIter == iter->right.end())
            {
                if (first.count(iter->left) == 0)
                    first[iter->left] = set<Symbol>();
                if (first[iter->left].insert(Symbol{ true,"EPSILON" }).second == true)
                    changeFlag = true;
            }
        }
    }
}

void Parser::getFollow()
{
    //put # in start production's follow
    follow[productions[0].left] = set<Symbol>();
    follow[productions[0].left].insert(Symbol{ true,"#" });
    bool changeFlag = true;
    while (changeFlag)
    {
        changeFlag = false;
        //for each production
        for (vector<Production>::iterator proIter = productions.begin(); proIter != productions.end(); proIter++)
        {
            //for each production's right
            for (vector<Symbol>::iterator sbIter = proIter->right.begin(); sbIter != proIter->right.end(); sbIter++)
            {
                //for each symbol next
                vector<Symbol>::iterator nextSbIter;
                for (nextSbIter = sbIter + 1; nextSbIter != proIter->right.end(); nextSbIter++)
                {
                    Symbol nextSym = *nextSbIter;
                    bool nextFlag = false;
                    if (nextSym.isTerm) {
                        if (follow.count(*sbIter) == 0)
                            follow[*sbIter] = set<Symbol>();
                        if (follow[*sbIter].insert(nextSym).second == true)
                            changeFlag = true;
                    }
                    else
                    {
                        //for each symbol next's first
                        for (set<Symbol>::iterator fIter = first[nextSym].begin(); fIter != first[nextSym].end(); fIter++)
                        {
                            if (fIter->content == "EPSILON")
                                nextFlag = true;
                            else
                            {
                                if (follow.count(*sbIter) == 0)
                                    follow[*sbIter] = set<Symbol>();
                                if (follow[*sbIter].insert(*fIter).second == true)
                                    changeFlag = true;
                            }
                        }
                    }
                    if (!nextFlag)
                        break;
                }
                //if traverse to end, add left symbol's follow to right symbol's follow
                if (nextSbIter == proIter->right.end())
                {
                    //for each element in left symbol's first
                    for (set<Symbol>::iterator followIter = follow[proIter->left].begin(); followIter != follow[proIter->left].end(); followIter++)
                    {
                        if (follow.count(*sbIter) == 0)
                            follow[*sbIter] = set<Symbol>();
                        if (follow[*sbIter].insert(*followIter).second == true)
                            changeFlag = true;
                    }
                }
            }
        }
    }
}

bool Parser::createDFA(MainWindow* mw)
{

    int nowState = 0;//the number of now state
    dfa.stas.push_back(derive(Item{ 0,0 }));
    //for each state 计算每个状态的项目集
    for (list<ItemSet>::iterator iter = dfa.stas.begin(); iter != dfa.stas.end(); iter++, nowState++)
    {
        //for each item in state
        for (set<Item>::iterator itIter = iter->begin(); itIter != iter->end(); itIter++)
        {
            // . at rightest ,reduct
            if (productions[itIter->pdctId].right.size() == itIter->pointPos)
            {
                set<Symbol>FOLLOW = follow[productions[itIter->pdctId].left];
                for (set<Symbol>::iterator followIter = FOLLOW.begin(); followIter != FOLLOW.end(); followIter++)
                {
                    if (SLR1subTable.count(GOTO(nowState, *followIter)) != 0) {
                        QString err = "Grammar is not SLR(1)，reduce conflict exists.";
                        mw->ui->ParserBrowser->append(err);
                        return false;
                    }
                    if (itIter->pdctId == 0)
                        SLR1subTable[GOTO(nowState, *followIter)] = Behavior{ accept,itIter->pdctId };
                    else
                        SLR1subTable[GOTO(nowState, *followIter)] = Behavior{ reduct,itIter->pdctId };
                }
                continue;
            }

            Symbol nextSymbol = productions[itIter->pdctId].right[itIter->pointPos];//Symbol after .
            //in DFA, GOTO(nowState,nextSymbol) already exist
            if (dfa.GTMap.count(GOTO(nowState, nextSymbol)) != 0)
                continue;
            //if in DFA, GOTO(nowState,nextSymbol) not exist
            ItemSet newI = derive(Item{ itIter->pdctId,itIter->pointPos + 1 });//新产生的状态
            //in this state, find other GOTO[nowI,nextSymbol]
            set<Item>::iterator shiftIter = itIter;
            shiftIter++;
            for (; shiftIter != iter->end(); shiftIter++)
            {
                //if reduct
                if (productions[shiftIter->pdctId].right.size() == shiftIter->pointPos)
                    continue;
                //if shift，and shift to nextSymbol
                else if (productions[shiftIter->pdctId].right[shiftIter->pointPos] == nextSymbol)
                {
                    ItemSet tempI = derive(Item{ shiftIter->pdctId,shiftIter->pointPos + 1 });
                    newI.insert(tempI.begin(), tempI.end());
                }
            }
            //search if this state is contain in dfa
            bool searchFlag = false;
            int index = 0;
            for (list<ItemSet>::iterator iterHave = dfa.stas.begin(); iterHave != dfa.stas.end(); iterHave++, index++)
                if (*iterHave == newI)
                {
                    dfa.GTMap[GOTO(nowState, nextSymbol)] = index;
                    if (SLR1subTable.count(GOTO(nowState, nextSymbol)) != 0){
                        QString err = "Grammar is not SLR(1)，conflict exists.";
                        mw->ui->ParserBrowser->append(err);
                        return false;
                    }
                    SLR1subTable[GOTO(nowState, nextSymbol)] = Behavior{ shift,index };
                    searchFlag = true;
                    break;
                }
            //not find this state
            if (!searchFlag)
            {
                if (SLR1subTable.count(GOTO(nowState, nextSymbol)) != 0){
                    QString err = "Grammar is not SLR(1)，conflict exists.";
                    mw->ui->ParserBrowser->append(err);
                    return false;
                }
                dfa.stas.push_back(newI);
                dfa.GTMap[GOTO(nowState, nextSymbol)] = dfa.stas.size() - 1;
                SLR1subTable[GOTO(nowState, nextSymbol)] = Behavior{ shift,int(dfa.stas.size() - 1) };
            }
            else
                continue;
        }
    }
    return true;
}

ItemSet Parser::derive(Item item)
{
    ItemSet i;
    // when.is at the rightest of production, it should be Reduced
    if (productions[item.pdctId].right.size() == item.pointPos)
        i.insert(item);
    //right of . is Terminal
    else if (productions[item.pdctId].right[item.pointPos].isTerm)
        i.insert(item);
    //right of . is not terminal
    else
    {
        i.insert(item);
        vector<Production>::iterator iter;
        for (iter = productions.begin(); iter < productions.end(); iter++)
            //left symbol of production == right of. non-Terminal
            if (iter->left == productions[item.pdctId].right[item.pointPos])
            {
                //add derive
                ItemSet temp = derive(Item{int(iter - productions.begin()),0 });
                set<Item>::iterator siter;
                for (siter = temp.begin(); siter != temp.end(); siter++)
                    i.insert(*siter);
            }
    }
    return i;
}

void Parser::showFirst(MainWindow *mw){

    mw->ui->ParserBrowser->append("<font color='orange'><b>FIRST:</b></font>");
    map<Symbol, set<Symbol>>::iterator iter = first.begin();
    while(iter!=first.end()){
        mw->ui->ParserBrowser->append("FIRST[<font color=blue>"+QString::fromStdString(iter->first.content)+"</font>] = { ");
        QString result;
        set<Symbol>::iterator setIter= iter->second.begin();
        while(setIter!=iter->second.end()){
            result+=QString::fromStdString(setIter->content);
            if(setIter!=--iter->second.end())
                result+=",";
            setIter++;
        }
        result+=" }";
        mw->ui->ParserBrowser->insertPlainText(result);
        iter++;
    }
    mw->ui->ParserBrowser->append("");

}

void Parser::showFollow(Terminal tm,MainWindow *mw){
    mw->ui->ParserBrowser->append("<font color='orange'><b>FOLLOW:</b></font>");
    map<Symbol, set<Symbol>>::iterator iter = follow.begin();
    while(iter!=follow.end()){
        //if empty, not show
        if(isTerm(tm,iter->first.content)){
            iter++;
            continue;
        }
        mw->ui->ParserBrowser->append("FOLLOW[<font color=blue>"+QString::fromStdString(iter->first.content)+"</font>] = { ");
        QString result;
        set<Symbol>::iterator setIter= iter->second.begin();
        while(setIter!=iter->second.end()){
            result+=QString::fromStdString(setIter->content);
            if(setIter!=--iter->second.end())
                result+=",";
            setIter++;
        }
        result+=" }";
        mw->ui->ParserBrowser->insertPlainText(result);
        iter++;
    }
    mw->ui->ParserBrowser->append("");
}

void Parser::showProductions(){
    for(int i=0;i<productions.size();i++){
        qDebug()<<i<<":"<<QString::fromStdString(productions[i].left.content)<<" ::= ";
        for(int j=0;j<productions[i].right.size();j++){
            qDebug()<<QString::fromStdString(productions[i].right[j].content)<<" ";
        }
        qDebug()<<endl;
    }
}

QString int_to_Qs(Behavior bh){
    if(bh.behavior == 0)
        return "r"+ QString::number(bh.nextStat);
    else if(bh.behavior==1)
        return "s"+ QString::number(bh.nextStat);
    else if(bh.behavior==2)
        return "acc";
    else
        return "err";
}

string int_to_s(Behavior bh){
    if(bh.behavior == 0)
        return "r"+ to_string(bh.nextStat);
    else if(bh.behavior==1)
        return "s"+ to_string(bh.nextStat);
    else if(bh.behavior==2)
        return "acc";
    else
        return "err";
}

void Parser::convert2Table(){

    map<GOTO, Behavior>::iterator iter = SLR1subTable.begin();
    int stateNum=iter->first.first;
    while(iter!=SLR1subTable.end()){
        set<pair<string,string>> tmp{};
        //QString result;
        //result+=+"<b>"+QString::number(iter->first.first)+"</b>: ";
        for(;iter!=SLR1subTable.end()&&stateNum==iter->first.first;iter++){
            pair<string,string> tmpPair(iter->first.second.content,int_to_s(iter->second));
            tmp.insert(tmpPair);
            //result+=QString::fromStdString(iter->first.second.content)+"&rarr;";
            //result+=int_to_Qs(iter->second)+' ';
        }
        //检查有无相同value
        //map<int,set<pair<string,string>>>::iterator FinalIter=SLR1_table.end();
        /*for(;FinalIter!=Final.end();FinalIter++){
            if(tmp==FinalIter->second)
                break;
        }*/
        //if(FinalIter==Final.end())
        //if(!SLR1_table.empty()&&tmp==(--FinalIter)->second){}
        //else
        SLR1_table.insert(pair<int,set<pair<string,string>>>(stateNum,tmp));
        //mw->ui->ParserBrowser->append(result);
        stateNum=iter->first.first;
    }
}

void Parser::showSLR1Table(MainWindow *mw){
    mw->ui->ParserBrowser->append("<font color='orange'><b>SLR(1) TABLE:</b></font>");
    map<int,set<pair<string,string>>>::iterator iter=SLR1_table.begin();
    while(iter!=SLR1_table.end()){
        mw->ui->ParserBrowser->append("<b>"+QString::number(iter->first)+"</b>: ");
        QString result;
        for(set<pair<string,string>>::iterator setIter=iter->second.begin();setIter!=iter->second.end();setIter++){
            result+=QString::fromStdString(setIter->first)+"→";
            result+=QString::fromStdString(setIter->second)+"  ";
        }
        mw->ui->ParserBrowser->insertPlainText(result);
        iter++;
    }
    mw->ui->ParserBrowser->append("");
}


list<int> Parser::merge(list<int>&l1, list<int>&l2)
{
    list<int>ret;
    ret.assign(l1.begin(), l1.end());
    ret.splice(ret.end(), l2);
    return ret;
}

bool Parser::SematicAnalyze(Terminal tm,MainWindow *mw){

    bool IsAccept = false;
    int nowBlockLevel=0;
    Symbol sb(true, "#");
    symbolStack.push(&sb);
    statusStack.push(0);

    for (int i=0; i<tm.termVec.size(); )
    {
        keyword kd = tm.termVec[i].first;
        string  word = tm.termVec[i].second;

        Symbol* symbolNow;
        if (kd == ID)
            symbolNow = new Id(Symbol{ true,"ID" }, word);
        else if (kd == INT10)
            symbolNow = new Num(Symbol{ true,"NUM" }, word);
        else
            symbolNow = new Symbol(true, word);

        //qDebug()<<"symbolNow:"<<QString::fromStdString(symbolNow->content);
        //qDebug()<<"statusStack.top"<<statusStack.top();

        //if not in SLR1
        if (SLR1subTable.count(GOTO(statusStack.top(), *symbolNow)) == 0){
            mw->ui->IRBrowser->append(QString::fromStdString(symbolNow->content));
            mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
            mw->ui->IRBrowser->append(QString::fromStdString(symbolNow->content)+" is not expected.");
            return false;
        }

        Behavior bh = SLR1subTable[GOTO(statusStack.top(), *symbolNow)];

        //qDebug()<<"bh.behavior:"<<bh.behavior;

        if (bh.behavior == shift)
        {
            symbolStack.push(symbolNow);
            statusStack.push(bh.nextStat);
            i++;
        }
        else if (bh.behavior == reduct)
        {
            Production rdPdc = productions[bh.nextStat];//rdPdc:reduct Production
            int popSbNum = rdPdc.right.size();//number of Symbol to reduct
            //qDebug()<<"bh.nextstate:"<<bh.nextStat;
            switch (bh.nextStat)
            {
            case 3://declare ::= int ID M A func_declare
            {
                FuncDeclare *_function_declare = (FuncDeclare*)popSymbol();
                Symbol* _A = popSymbol();
                M* _M = (M*)popSymbol();
                Id* _ID = (Id*)popSymbol();
                Symbol* _int = popSymbol();
                pushSymbol(new Symbol(rdPdc.left),mw);
                funcTable.push_back(Func{ _ID->name,D_INT,_function_declare->funcList,_M->quad });
                delete _A;
                delete _M;
                delete _ID;
                delete _int;
                break;
            }
            case 4://declare ::= int ID ;
            {
                Symbol* _com = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Symbol* _int = popSymbol();
                pushSymbol(new Symbol(rdPdc.left),mw);
                varTable.push_back(Var{ _ID->name,D_INT,nowBlockLevel,0 });
                delete _com;
                delete _ID;
                delete _int;
                break;
            }
            case 5://declare ::= void ID M A func_declare
            {
                FuncDeclare* _func_declare = (FuncDeclare*)popSymbol();
                Symbol* _A = popSymbol();
                M* _M = (M*)popSymbol();
                Id* _ID = (Id*)popSymbol();
                Symbol* _void = popSymbol();
                pushSymbol(new Symbol(rdPdc.left),mw);
                funcTable.push_back(Func{ _ID->name, D_VOID, _func_declare->funcList,_M->quad });
                delete _A;
                delete _M;
                delete _ID;
                delete _void;
                delete _func_declare;
                break;
            }
            case 6://A ::=
            {
                nowBlockLevel++;
                pushSymbol(new Symbol(rdPdc.left),mw);
                break;
            }
            case 7://func_declare ::= ( para ) stc_block
            {
                StcBlock* _stc_block = (StcBlock*)popSymbol();
                Symbol* _rb = popSymbol();
                Parameter* _paramter = (Parameter*)popSymbol();
                Symbol* _lb = popSymbol();
                FuncDeclare* _func_declare = new FuncDeclare(rdPdc.left);
                _func_declare->funcList.assign(_paramter->plist.begin(), _paramter->plist.end());
                pushSymbol(_func_declare,mw);
                delete _stc_block;
                delete _rb;
                delete _paramter;
                delete _lb;
                break;
            }
            case 8://para ::= para_list
            {
                ParaList* _para_list = (ParaList*)popSymbol();
                Parameter *_parameter = new Parameter(rdPdc.left);
                _parameter->plist.assign(_para_list->plist.begin(), _para_list->plist.end());
                pushSymbol(_parameter,mw);
                delete _para_list;
                break;
            }
            case 9://para ::= void
            {
                Symbol* _void = popSymbol();
                Parameter* _parameter = new Parameter(rdPdc.left);
                pushSymbol(_parameter,mw);
                delete _void;
                break;
            }
            case 10://para_list ::= param
            {
                Symbol* _param = popSymbol();
                ParaList* parameter_list = new ParaList(rdPdc.left);
                parameter_list->plist.push_back(D_INT);
                pushSymbol(parameter_list,mw);
                delete _param;
                break;
            }
            case 11://para_list ::= param , para_list
            {
                ParaList* _para_list2 = (ParaList*)popSymbol();
                Symbol* _com = popSymbol();
                Symbol* _para = popSymbol();
                ParaList *_para_list1 = new ParaList(rdPdc.left);
                //when instancing,add int as declare type
                _para_list2->plist.push_front(D_INT);
                _para_list1->plist.assign(_para_list2->plist.begin(), _para_list2->plist.end());
                pushSymbol(_para_list1,mw);
                delete _para_list2;
                delete _com;
                delete _para;
                break;
            }
            case 12://param ::= int ID
            {
                Id* _ID = (Id*)popSymbol();
                Symbol* _int = popSymbol();
                varTable.push_back(Var{ _ID->name,D_INT,nowBlockLevel,0 });
                iterRps.emitt(Quadruple("get", "_", "_", _ID->name));
                Symbol* _param = new Symbol(rdPdc.left);
                pushSymbol(_param,mw);
                delete _ID;
                delete _int;
                break;
            }
            case 13://stc_block ::= { inner_declare stc_list }
            {
                Symbol* _rb = popSymbol();
                StcList* _stc_list = (StcList*)popSymbol();
                Symbol* _inner_declare = popSymbol();
                Symbol* _lb = popSymbol();
                StcBlock* _stc_block = new StcBlock(rdPdc.left);
                _stc_block->nextList = _stc_list->nextList;
                nowBlockLevel--;
                int popNum = 0;
                for (vector<Var>::reverse_iterator riter = varTable.rbegin(); riter != varTable.rend(); riter++)
                    if (riter->level > nowBlockLevel)
                        popNum++;
                    else
                        break;
                for (int i = 0; i < popNum; i++)
                    varTable.pop_back();
                pushSymbol(_stc_block,mw);
                delete _rb;
                delete _stc_list;
                delete _inner_declare;
                delete _lb;
                break;
            }
            case 16://inner_var_declare ::= int ID
            {
                Id* _ID = (Id*)popSymbol();
                Symbol* _int = popSymbol();
                pushSymbol(new Symbol(rdPdc.left),mw);
                varTable.push_back(Var{ _ID->name,D_INT,nowBlockLevel,0});
                delete _ID;
                delete _int;
                break;
            }
            case 17://stc_list ::= stc M stc_list
            {
                StcList* _stc_list2 = (StcList*)popSymbol();
                M* _M = (M*)popSymbol();
                Sentence* _stc = (Sentence*)popSymbol();
                StcList* _stc_list1 = new StcList(rdPdc.left);
                _stc_list1->nextList = _stc_list2->nextList;
                iterRps.backPatch(_stc->nextList, _M->quad);
                pushSymbol(_stc_list1,mw);
                delete _stc_list2;
                delete _M;
                delete _stc;
                break;
            }
            case 18://stc_list ::= stc
            {
                Sentence* _stc = (Sentence*)popSymbol();
                StcList* _stc_list = new StcList(rdPdc.left);
                _stc_list->nextList = _stc->nextList;
                pushSymbol(_stc_list,mw);
                delete _stc;
                break;
            }
            case 19://stc ::= if_stc
            {
                IfStc* _if_stc = (IfStc*)popSymbol();
                Sentence* _sentence = new Sentence(rdPdc.left);
                _sentence->nextList = _if_stc->nextList;
                pushSymbol(_sentence,mw);
                delete _if_stc;
                break;
            }
            case 20://stc ::= while_stc
            {
                WhileStc* _while_stc = (WhileStc*)popSymbol();
                Sentence* _sentence = new Sentence(rdPdc.left);
                _sentence->nextList = _while_stc->nextList;
                pushSymbol(_sentence,mw);
                delete _while_stc;
                break;
            }
            case 21://stc ::= return_stc
            {
                Symbol* _return_stc = popSymbol();
                Sentence* sentence = new Sentence(rdPdc.left);
                pushSymbol(sentence,mw);
                delete _return_stc;
                break;
            }
            case 22://stc ::= assign_stc
            {
                Symbol* _assign_stc = popSymbol();
                Sentence* _sentence = new Sentence(rdPdc.left);
                pushSymbol(_sentence,mw);
                delete _assign_stc;
                break;
            }
            case 23://assign_stc ::= ID = exp ;
            {
                Symbol* _com = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _assign = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Symbol* _assign_stc = new Symbol(rdPdc.left);
                iterRps.emitt(Quadruple("=", _exp->name, "_", _ID->name));
                pushSymbol(_assign_stc,mw);
                delete _com;
                delete _exp;
                delete _assign;
                delete _ID;
                break;
            }
            case 24://return_stc ::= return ;
            {
                Symbol* _com = popSymbol();
                Symbol* _return = popSymbol();
                iterRps.emitt(Quadruple("return", "_", "_", "_"));
                pushSymbol(new Symbol(rdPdc.left),mw);
                delete _com;
                delete _return;
                break;
            }
            case 25://return_stc ::= return exp ;
            {
                Symbol* _com = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _return = popSymbol();
                iterRps.emitt(Quadruple("return", _exp->name, "_", "_"));
                pushSymbol(new Symbol(rdPdc.left),mw);
                delete _com;
                delete _exp;
                delete _return;
                break;
            }
            case 26://while_stc ::= while M ( exp ) A stc_block
            {
                StcBlock* _stc_block = (StcBlock*)popSymbol();
                Symbol* _A = popSymbol();
                Symbol* _rp = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _lp = popSymbol();
                M* _M = (M*)popSymbol();
                Symbol* _while = popSymbol();
                WhileStc* while_sentence = new WhileStc(rdPdc.left);
                iterRps.backPatch(_stc_block->nextList, _M->quad);
                while_sentence->nextList = _exp->falseList;
                iterRps.emitt(Quadruple("j", "_", "_", to_string(_M->quad)));
                pushSymbol(while_sentence,mw);
                delete _stc_block;
                delete _A;
                delete _rp;
                delete _exp;
                delete _lp;
                delete _M;
                delete _while;
                break;
            }
            case 27://if_stc ::= if ( exp ) A stc_block
            {
                StcBlock* _stc_block = (StcBlock*)popSymbol();
                Symbol* _A = popSymbol();
                Symbol* _rp = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _lp = popSymbol();
                Symbol* _if = popSymbol();
                IfStc* _if_stc = new IfStc(rdPdc.left);
                _exp->falseList.splice(_exp->falseList.begin(), _stc_block->nextList);
                _if_stc->nextList = _exp->falseList;
                pushSymbol(_if_stc,mw);
                delete _stc_block;
                delete _A;
                delete _rp;
                delete _exp;
                delete _lp;
                delete _if;
                break;
            }
            case 28://if_stc ::= if ( exp ) A stc_block N else M A stc_block
            {
                StcBlock* _stc_block2 = (StcBlock*)popSymbol();
                Symbol* _A2 = popSymbol();
                M* _M = (M*)popSymbol();
                Symbol* _else = popSymbol();
                N* _N = (N*)popSymbol();
                StcBlock* _stc_block1 = (StcBlock*)popSymbol();
                Symbol* _A1 = popSymbol();
                Symbol* _rp = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _lp = popSymbol();
                Symbol* _if = popSymbol();
                IfStc* _if_sentence = new IfStc(rdPdc.left);
                iterRps.backPatch(_exp->falseList, _M->quad);
                _if_sentence->nextList = merge(_stc_block1->nextList, _stc_block2->nextList);
                _if_sentence->nextList = merge(_if_sentence->nextList, _N->nextList);
                pushSymbol(_if_sentence,mw);
                delete _stc_block2;
                delete _A2;
                delete _M;
                delete _else;
                delete _N;
                delete _stc_block1;
                delete _A1;
                delete _rp;
                delete _exp;
                delete _lp;
                delete _if;
                break;
            }
            case 29://N ::=
            {
                N* _N = new N(rdPdc.left);
                _N->nextList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j", "_", "_", "-1"));
                pushSymbol(_N,mw);
                break;
            }
            case 30://M ::=
            {
                M* _M = new M(rdPdc.left);
                _M->quad = iterRps.nextQuad();
                pushSymbol(_M,mw);
                break;
            }
            case 31://exp ::= add_exp
            {
                AddExp* _add_exp = (AddExp*)popSymbol();
                Exp* _exp = new Exp(rdPdc.left);
                _exp->name = _add_exp->name;
                pushSymbol(_exp,mw);
                delete _add_exp;
                break;
            }
            case 32://exp ::= add_exp > add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _gt = popSymbol();
                AddExp* _add_exp1 = (AddExp*)popSymbol();
                Exp* expression = new Exp(rdPdc.left);
                expression->falseList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j<=", _add_exp1->name, _add_exp2->name, "-1"));
                pushSymbol(expression,mw);
                delete _add_exp2;
                delete _gt;
                delete _add_exp1;
                break;
            }
            case 33://exp ::= add_exp < add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _lt = popSymbol();
                AddExp* _add_exp1 = (AddExp*)popSymbol();
                Exp* _exp = new Exp(rdPdc.left);
                _exp->falseList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j>=", _add_exp1->name, _add_exp2->name, "-1"));
                pushSymbol(_exp,mw);
                delete _add_exp2;
                delete _lt;
                delete _add_exp1;
                break;
            }
            case 34://exp ::= add_exp == add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol *_eq = popSymbol();
                AddExp *_add_exp1 = (AddExp*)popSymbol();
                Exp *_exp = new Exp(rdPdc.left);
                _exp->falseList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j!=", _add_exp1->name, _add_exp2->name, "-1"));
                pushSymbol(_exp,mw);
                delete _add_exp2;
                delete _eq;
                delete _add_exp1;
                break;
            }
            case 35://exp ::= add_exp >= add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _ge = popSymbol();
                AddExp* _add_exp1 = (AddExp*)popSymbol();
                Exp* expression = new Exp(rdPdc.left);
                expression->falseList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j<", _add_exp1->name, _add_exp2->name, "-1"));
                pushSymbol(expression,mw);
                delete _add_exp2;
                delete _ge;
                delete _add_exp1;
                break;
            }
            case 36://exp ::= add_exp <= add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _le = popSymbol();
                AddExp* _add_exp1 = (AddExp*)popSymbol();
                Exp* exp = new Exp(rdPdc.left);
                exp->falseList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j>", _add_exp1->name, _add_exp2->name, "-1"));
                pushSymbol(exp,mw);
                delete _add_exp2;
                delete _le;
                delete _add_exp1;
                break;
            }
            case 37://exp ::= add_exp != add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _neq = popSymbol();
                AddExp* _add_exp1 = (AddExp*)popSymbol();
                Exp* _exp = new Exp(rdPdc.left);
                _exp->falseList.push_back(iterRps.nextQuad());
                iterRps.emitt(Quadruple("j==", _add_exp1->name, _add_exp2->name, "-1"));
                pushSymbol(_exp,mw);
                delete _add_exp2;
                delete _neq;
                delete _add_exp1;
                break;
            }
            case 38://add_exp ::= item
            {
                ItemSym* _item = (ItemSym*)popSymbol();
                AddExp* _add_exp = new AddExp(rdPdc.left);
                _add_exp->name = _item->name;
                pushSymbol(_add_exp,mw);
                delete _item;
                break;
            }
            case 39://add_exp ::= item + add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _add = popSymbol();
                ItemSym* _item = (ItemSym*)popSymbol();
                AddExp* _add_exp1 = new AddExp(rdPdc.left);
                _add_exp1->name = newTemper.newTemp();
                iterRps.emitt(Quadruple("+", _item->name, _add_exp2->name, _add_exp1->name));
                pushSymbol(_add_exp1,mw);
                delete _add_exp2;
                delete _add;
                delete _item;
                break;
            }
            case 40://add_exp ::= item - add_exp
            {
                AddExp* _add_exp2 = (AddExp*)popSymbol();
                Symbol* _sub = popSymbol();
                ItemSym* _item = (ItemSym*)popSymbol();
                AddExp* _add_exp1 = new AddExp(rdPdc.left);
                _add_exp1->name = newTemper.newTemp();
                iterRps.emitt(Quadruple("-", _item->name, _add_exp2->name, _add_exp1->name));
                pushSymbol(_add_exp1,mw);
                delete _add_exp2;
                delete _sub;
                delete _item;
                break;
            }
            case 41://item ::= factor
            {
                Factor* _factor = (Factor*)popSymbol();
                ItemSym* _item = new ItemSym(rdPdc.left);
                _item->name = _factor->name;
                pushSymbol(_item,mw);
                delete _factor;
                break;
            }
            case 42://item1 ::= factor * item2
            {
                ItemSym* _item2 = (ItemSym*)popSymbol();
                Symbol* _mul = popSymbol();
                Factor* _factor = (Factor*)popSymbol();
                ItemSym* _item1 = new ItemSym(rdPdc.left);
                _item1->name = newTemper.newTemp();
                iterRps.emitt(Quadruple("*", _factor->name, _item2->name, _item1->name));
                pushSymbol(_item1,mw);
                delete _item2;
                delete _mul;
                delete _factor;
                break;
            }
            case 43://item1 ::= factor / item2
            {
                ItemSym* _item2 = (ItemSym*)popSymbol();
                Symbol* _div = popSymbol();
                Factor* _factor = (Factor*)popSymbol();
                ItemSym* _item1 = new ItemSym(rdPdc.left);
                _item1->name = newTemper.newTemp();
                iterRps.emitt(Quadruple("/", _factor->name, _item2->name, _item1->name));
                pushSymbol(_item1,mw);
                delete _item2;
                delete _div;
                delete _factor;
                break;
            }
            case 44://factor ::= NUM
            {
                Num* _NUM = (Num*)popSymbol();
                Factor* _factor = new Factor(rdPdc.left);
                _factor->name = _NUM->number;
                pushSymbol(_factor,mw);
                delete _NUM;
                break;
            }
            case 45://factor ::= ( exp )
            {
                Symbol* _rp = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _lp = popSymbol();
                Factor* _factor = new Factor(rdPdc.left);
                _factor->name = _exp->name;
                pushSymbol(_factor,mw);
                delete _rp;
                delete _exp;
                delete _lp;
                break;
            }
            case 46://factor ::= ID ( argu_list )
            {
                Symbol* _rp = popSymbol();
                ArguList* _argu_list = (ArguList*)popSymbol();
                Symbol* _lp = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Factor* _factor = new Factor(rdPdc.left);
                Func* _func = CheckFunc(_ID->name);
                if (!_func){
                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                    mw->ui->IRBrowser->append("Function "+QString::fromStdString(_ID->name)+" is not declared.");
                    return false;
                }
                else if (!paraSizeMatch(_argu_list->alist, _func->paramTypes)){
                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                    mw->ui->IRBrowser->append("Function "+QString::fromStdString(_ID->name)+" parameter is not match.");
                    return false;
                }
                else
                {
                    for (list<string>::iterator iter = _argu_list->alist.begin(); iter != _argu_list->alist.end(); iter++)
                        iterRps.emitt(Quadruple("par", *iter, "_", "_"));
                    _factor->name = newTemper.newTemp();
                    iterRps.emitt(Quadruple("call", _ID->name,"_", "_"));
                    iterRps.emitt(Quadruple("=", "@RETURN", "_", _factor->name));
                    pushSymbol(_factor,mw);
                }
                delete _rp;
                delete _argu_list;
                delete _lp;
                delete _ID;
                break;
            }
            case 47://factor ::= ID
            {
                Id* _ID = (Id*)popSymbol();
                if (CheckVar(_ID->name) == NULL){
                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                    mw->ui->IRBrowser->append(QString::fromStdString(_ID->name)+" is not declared.");
                    return false;
                }
                Factor* _factor = new Factor(rdPdc.left);
                _factor->name = _ID->name;
                pushSymbol(_factor,mw);
                delete _ID;
                break;
            }
            case 48://argu_list ::=
            {
                ArguList* _argu_list = new ArguList(rdPdc.left);
                pushSymbol(_argu_list,mw);
                break;
            }
            case 49://argu_list ::= exp
            {
                Exp* _exp = (Exp*)popSymbol();
                ArguList* _argu_list = new ArguList(rdPdc.left);
                _argu_list->alist.push_back(_exp->name);
                pushSymbol(_argu_list,mw);
                delete _exp;
                break;
            }
            case 50://argu_list ::= exp , argu_list
            {
                ArguList* _argu_list2 = (ArguList*)popSymbol();
                Symbol* _com = popSymbol();
                Exp* expression = (Exp*)popSymbol();
                ArguList* _argu_list1 = new ArguList(rdPdc.left);
                _argu_list2->alist.push_front(expression->name);
                _argu_list1->alist.assign(_argu_list2->alist.begin(),_argu_list2->alist.end());
                pushSymbol(_argu_list1,mw);
                delete _argu_list2;
                delete _com;
                break;
            }
//            case 51://factor ::= array
//            {
//                Array* _array = (Array*)popSymbol();
//                if (CheckVar(_array->name) == NULL){
//                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
//                    mw->ui->IRBrowser->append(QString::fromStdString(_array->name)+" is not declared.");
//                    return false;
//                }
//                Factor* _factor = new Factor(rdPdc.left);
//                _factor->name = _array->name;
//                pushSymbol(_factor,mw);
//                delete _array;
//                break;
//            }
//            case 52://array ::= ID [ factor ]
//            {
//                Symbol* _mrb = popSymbol();
//                Factor* _factor = (Factor*)popSymbol();
//                Symbol* _mlb = popSymbol();
//                Id* _ID = (Id*)popSymbol();
//                Array* _array = new Array(rdPdc.left);
//                _array->name = _ID->name;
//                _array->dimension = 1;
//                _array->dm2_size='\0';
//                _array->dm1_size=_factor->name;
//                pushSymbol(_array,mw);
//                delete _mrb;
//                delete _factor;
//                delete _mlb;
//                delete _ID;
//                break;
//            }
//            case 53://array ::= ID [ factor ] [ factor ]
//            {
//                Symbol* _mrb2 = popSymbol();
//                Factor* _factor2 = (Factor*)popSymbol();
//                Symbol* _mlb2 = popSymbol();
//                Symbol* _mrb1 = popSymbol();
//                Factor* _factor1 = (Factor*)popSymbol();
//                Symbol* _mlb1 = popSymbol();
//                Id* _ID = (Id*)popSymbol();
//                Array* _array = new Array(rdPdc.left);
//                _array->name = _ID->name;
//                _array->dimension = 2;
//                _array->dm2_size=_factor2->name;
//                _array->dm1_size=_factor1->name;
//                pushSymbol(_array,mw);
//                delete _mrb2;
//                delete _factor2;
//                delete _mlb2;
//                delete _mrb1;
//                delete _factor1;
//                delete _mlb1;
//                delete _ID;
//                break;
//            }
//            case 54://declare ::= int array ;
//            {
//                Symbol* _com = popSymbol();
//                Array* _array = (Array*)popSymbol();
//                Symbol* _int = popSymbol();
//                pushSymbol(new Symbol(rdPdc.left),mw);
//                varTable.push_back(Var{ _array->name,D_INT,nowBlockLevel,_array->dimension });
//                arrTable.push_back(Array(_array->name,_array->dimension,_array->dm1_size,_array->dm2_size));
//                delete _com;
//                delete _array;
//                delete _int;
//                break;
//            }
//            case 55://inner_var_declare ::= int array
//            {
//                Array* _array = (Array*)popSymbol();
//                Symbol* _int = popSymbol();
//                pushSymbol(new Symbol(rdPdc.left),mw);
//                varTable.push_back(Var{ _array->name,D_INT,nowBlockLevel,_array->dimension });
//                arrTable.push_back(Array(_array->name,_array->dimension,_array->dm1_size,_array->dm2_size));
//                delete _array;
//                delete _int;
//                break;
//            }
//            case 56://assign_stc ::= array = exp ;
//            {
//                Symbol* _com = popSymbol();
//                Exp* _exp = (Exp*)popSymbol();
//                Symbol* _assign = popSymbol();
//                Array* _array = (Array*)popSymbol();
//                Symbol* _assign_stc = new Symbol(rdPdc.left);
//                Array* dec_arr = CheckArray(_array->name);
//                if(!dec_arr){
//                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
//                    mw->ui->IRBrowser->append("array "+ QString::fromStdString(_array->name)+"is not declared");
//                    return false;
//                }
//                if(_array->dimension==2){
//                    string tmpName1 = newTemper.newTemp();
//                    iterRps.emitt(Quadruple("*", _array->dm1_size, dec_arr->dm2_size,tmpName1 ));
//                    string tmpName2 = newTemper.newTemp();
//                    iterRps.emitt(Quadruple("+", _array->dm2_size, tmpName1,tmpName2 ));
//                    string tmpName3 = newTemper.newTemp();
//                    iterRps.emitt(Quadruple("*", tmpName2, "4",tmpName3 ));
//                    iterRps.emitt(Quadruple("[]=", _exp->name, "_",_array->name+"["+tmpName3+"]" ));
//                }
//                else{
//                    string tmpName = newTemper.newTemp();
//                    iterRps.emitt(Quadruple("*", _array->dm1_size, "4",tmpName ));
//                    iterRps.emitt(Quadruple("[]=", _exp->name, "_",_array->name+"["+tmpName+"]" ));
//                }
//                pushSymbol(_assign_stc,mw);
//                delete _com;
//                delete _exp;
//                delete _assign;
//                delete _array;
//                break;
//            }
            case 51://factor ::= array
            {
                Array* _array = (Array*)popSymbol();
                if (CheckVar(_array->name) == NULL){
                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                    mw->ui->IRBrowser->append(QString::fromStdString(_array->name)+" is not declared.");
                    return false;
                }
                Factor* _factor = new Factor(rdPdc.left);
                _factor->name = _array->result;
                pushSymbol(_factor,mw);
                delete _array;
                break;
            }
            case 52://array ::= ID [ factor ]
            {
                Symbol* _mrb = popSymbol();
                Factor* _factor = (Factor*)popSymbol();
                Symbol* _mlb = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Array* _array = new Array(rdPdc.left);
                _array->name = _ID->name;
                _array->dimension = 1;
                _array->dm2_size='\0';
                _array->dm1_size=_factor->name;
                Array* dec_arr = CheckArray(_array->name);
                if(!dec_arr){
                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                    mw->ui->IRBrowser->append("array "+ QString::fromStdString(_array->name)+"is not declared");
                    return false;
                }

                    string tmpName = newTemper.newTemp();
                    iterRps.emitt(Quadruple("*", _array->dm1_size, "4",tmpName ));
                    _array->result=_array->name+"["+tmpName+"]";


                pushSymbol(_array,mw);
                delete _mrb;
                delete _factor;
                delete _mlb;
                delete _ID;
                break;
            }
            case 53://array ::= ID [ factor ] [ factor ]
            {
                Symbol* _mrb2 = popSymbol();
                Factor* _factor2 = (Factor*)popSymbol();
                Symbol* _mlb2 = popSymbol();
                Symbol* _mrb1 = popSymbol();
                Factor* _factor1 = (Factor*)popSymbol();
                Symbol* _mlb1 = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Array* _array = new Array(rdPdc.left);
                _array->name = _ID->name;
                _array->dimension = 2;
                _array->dm2_size=_factor2->name;
                _array->dm1_size=_factor1->name;
                Array* dec_arr = CheckArray(_array->name);
                if(!dec_arr){
                    mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                    mw->ui->IRBrowser->append("array "+ QString::fromStdString(_array->name)+"is not declared");
                    return false;
                }

                    string tmpName1 = newTemper.newTemp();
                    iterRps.emitt(Quadruple("*", _array->dm1_size, dec_arr->dm2_size,tmpName1 ));
                    string tmpName2 = newTemper.newTemp();
                    iterRps.emitt(Quadruple("+", _array->dm2_size, tmpName1,tmpName2 ));
                    string tmpName3 = newTemper.newTemp();
                    iterRps.emitt(Quadruple("*", tmpName2, "4",tmpName3 ));
                    _array->result=_array->name+"["+tmpName3+"]";


                pushSymbol(_array,mw);
                delete _mrb2;
                delete _factor2;
                delete _mlb2;
                delete _mrb1;
                delete _factor1;
                delete _mlb1;
                delete _ID;
                break;
            }
            case 54://inner_var_declare ::= int ID [ factor ]
            {
                Symbol* _mrb = popSymbol();
                Factor* _factor = (Factor*)popSymbol();
                Symbol* _mlb = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Symbol* _int = popSymbol();
                pushSymbol(new Symbol(rdPdc.left),mw);
                varTable.push_back(Var{ _ID->name,D_INT,nowBlockLevel,1 });
                arrTable.push_back(Array(_ID->name,1,_factor->name,"\0"));
                delete _mrb;
                delete _factor;
                delete _mlb;
                delete _ID;
                delete _int;
                break;
            }
            case 55://inner_var_declare ::= int ID [ factor ] [ factor ]
            {
                Symbol* _mrb2 = popSymbol();
                Factor* _factor2 = (Factor*)popSymbol();
                Symbol* _mlb2 = popSymbol();
                Symbol* _mrb1 = popSymbol();
                Factor* _factor1 = (Factor*)popSymbol();
                Symbol* _mlb1 = popSymbol();
                Id* _ID = (Id*)popSymbol();
                Symbol* _int = popSymbol();
                pushSymbol(new Symbol(rdPdc.left),mw);
                varTable.push_back(Var{ _ID->name,D_INT,nowBlockLevel,2 });
                arrTable.push_back(Array(_ID->name,2,_factor1->name,_factor2->name));
                delete _mrb2;
                delete _factor2;
                delete _mlb2;
                delete _mrb1;
                delete _factor1;
                delete _mlb1;
                delete _ID;
                delete _int;
                break;
            }
            case 56://assign_stc ::= array = exp ;
            {
                Symbol* _com = popSymbol();
                Exp* _exp = (Exp*)popSymbol();
                Symbol* _assign = popSymbol();
                Array* _array = (Array*)popSymbol();
                Symbol* _assign_stc = new Symbol(rdPdc.left);
                iterRps.emitt(Quadruple("[]=", _exp->name, "_",_array->result ));
                pushSymbol(_assign_stc,mw);
                delete _com;
                delete _exp;
                delete _assign;
                delete _array;
                break;
            }
            default:
                for (int i = 0; i < popSbNum; i++){
                    delete popSymbol();
                }
                pushSymbol(new Symbol(rdPdc.left),mw);
                break;
            }
        }
        else if (bh.behavior == accept)
        {//P ::= N declare_list
            IsAccept = true;
            Func*f = CheckFunc("main");
            if(!f){
                mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
                mw->ui->IRBrowser->append("function main cannot be found");
                return false;
            }
            Symbol* sb=popSymbol();
            N* _N = (N*)popSymbol();
            iterRps.backPatch(_N->nextList, f->enterPoint);
            delete sb;
            delete _N;
            break;
        }
    }
    if (!IsAccept){
        mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
        mw->ui->IRBrowser->append("unknown end");
        return false;
    }
    return true;
}

vector<pair<int, string> >Parser::getFuncEnter()
{
    vector<pair<int, string> >ret;
    for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++){
        //qDebug() << iter->enterPoint;
        //qDebug() <<  " " + QString::fromStdString(iter->name);
        ret.push_back(pair<int, string>(iter->enterPoint, iter->name));}
    sort(ret.begin(), ret.end());
    return ret;
}

Symbol* Parser::popSymbol()
{
    Symbol* ret = symbolStack.top();
    symbolStack.pop();
    statusStack.pop();
    return ret;
}

void Parser::pushSymbol(Symbol* sym,MainWindow *mw)
{
    symbolStack.push(sym);
    if (SLR1subTable.count(GOTO(statusStack.top(), *sym)) == 0){
        mw->ui->IRBrowser->append("<font color='red'><b>sematic error!</b></font>");
        mw->ui->IRBrowser->append(QString::fromStdString(sym->content)+" unknown end");
    }
    Behavior bh = SLR1subTable[GOTO(statusStack.top(), *sym)];
    statusStack.push(bh.nextStat);
}

Func* Parser::CheckFunc(string ID)
{
    for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++)
        if (iter->name == ID)
            return &(*iter);
    return NULL;
}

Var* Parser::CheckVar(string ID)
{
    for (vector<Var>::reverse_iterator iter = varTable.rbegin(); iter != varTable.rend(); iter++)
        if (iter->name == ID)
            return &(*iter);
    return NULL;
}

Array* Parser::CheckArray(string ID)
{
    for (vector<Array>::reverse_iterator iter = arrTable.rbegin(); iter != arrTable.rend(); iter++)
        if (iter->name == ID)
            return &(*iter);
    return NULL;
}

bool Parser::paraSizeMatch(list<string>&argument_list, list<DataType>&parameter_list)
{
    if (argument_list.size() != parameter_list.size())
        return false;
    else
        return true;
}
