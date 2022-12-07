#include "IR.h"
#include<cmath>
#include<QDebug>

void IR::divideBlocks(vector<pair<int, string> > funcEnter)
{
      // qDebug()<<funcEnter.size();
    for (vector<pair<int, string> >::iterator iter = funcEnter.begin(); iter != funcEnter.end(); iter++)
    { //for each func
        vector<Block>blocks;
        priority_queue<int, vector<int>, greater<int> >blockEnter;
        blockEnter.push(iter->first);
        int endIndex = iter + 1 == funcEnter.end()? codeVec.size(): (iter + 1)->first;
//        qDebug()<<endIndex;
        for (int i = iter->first; i != endIndex; i++)//in this block first Quadruple->last Quadruple
        {
//            qDebug()<<"("+QString::fromStdString(codeVec[i].op)+","+QString::fromStdString(codeVec[i].src1)+","+QString::fromStdString(codeVec[i].src2)+","+QString::fromStdString(codeVec[i].dst)+")";
            if (codeVec[i].op[0] == 'j')
            {
                if (codeVec[i].op == "j") //j
                    blockEnter.push(atoi(codeVec[i].dst.c_str()));
                else
                {//j==,j!=，j>=，j>，j<=，j<
                    if (i + 1 < endIndex)
                        blockEnter.push(i + 1);
                    blockEnter.push(atoi(codeVec[i].dst.c_str()));
                }
            }
            else if (codeVec[i].op == "return" || codeVec[i].op == "call")
                if (i + 1 < endIndex)
                    blockEnter.push(i + 1);
        }

        Block block;
        map<int, string>labelEnter;//入口点和标签的对应关系
        map<int, int>enterBlockRela;//建立入口点和block的对应关系
        int firstFlag = true;//函数块第一块标记，该块命名为函数名
        int enter;
        int lastEnter = blockEnter.top();
        blockEnter.pop();
        while (!blockEnter.empty())
        {
            //插入四元式到block中
            enter = blockEnter.top();
            blockEnter.pop();
            if (enter == lastEnter)
                continue;
            for (int i = lastEnter; i != enter; i++)
                block.codes.push_back(codeVec[i]);
            if (!firstFlag)
            {//该基本块不是函数块的第一块基本块
                block.name = newLabel.newLabel();
                labelEnter[lastEnter] = block.name;
            }
            else
            {//该基本块是函数块的第一块基本块
                block.name = iter->second;
                firstFlag = false;
            }
            enterBlockRela[lastEnter] = blocks.size();
            blocks.push_back(block);
            lastEnter = enter;
            block.codes.clear();
        }
        if (!firstFlag)
        {//该基本块不是函数块的第一块基本块
            block.name = newLabel.newLabel();
            labelEnter[lastEnter] = block.name;
        }
        else
        {//该基本块是函数块的第一块基本块
            block.name = iter->second;
            firstFlag = false;
        }
        if (iter + 1 != funcEnter.end()) //在两个函数的起点之间
            for (int i = lastEnter; i != (iter+1)->first; i++)
                block.codes.push_back(codeVec[i]);
        else
        {//在最后一个函数至中间代码末尾
            for (int i = lastEnter; i != codeVec.size(); i++)
                block.codes.push_back(codeVec[i]);
        }
        enterBlockRela[lastEnter] = blocks.size();
        blocks.push_back(block);
        int blockIndex = 0;
        for (vector<Block>::iterator bIter = blocks.begin(); bIter != blocks.end(); bIter++, blockIndex++)
        {
            vector<Quadruple>::reverse_iterator lastCode = bIter->codes.rbegin();
            if (lastCode->op[0] == 'j')
            {
                if (lastCode->op == "j") {//若操作符是j
                    bIter->next1 = enterBlockRela[atoi(lastCode->dst.c_str())];
                    bIter->next2 = -1;
                }
                else {//若果操作符是j=-,,j!=.j>=，j>，j<=，j<
                    bIter->next1 = blockIndex + 1;
                    bIter->next2 = enterBlockRela[atoi(lastCode->dst.c_str())];
                    bIter->next2 = bIter->next1 == bIter->next2 ? -1 : bIter->next2;
                }
                lastCode->dst = labelEnter[atoi(lastCode->dst.c_str())];
            }
            else if (lastCode->op == "return")
                bIter->next1 = bIter->next2 = -1;
            else
            {
                bIter->next1 = blockIndex + 1;
                bIter->next2 = -1;
            }
        }
        funcBlocks[iter->second] = blocks;
    }
}

void IR::emitt(Quadruple q)
{
    this->codeVec.push_back(q);
}

void IR::backPatch(list<int>nextList, int quad)
{
    for (list<int>::iterator iter = nextList.begin(); iter != nextList.end(); iter++)
        codeVec[*iter].dst = to_string(quad);
}

int IR::nextQuad()
{
    return codeVec.size();
}

void IR::clear(){
    codeVec.clear();
    funcBlocks.clear();
    newLabel.reset();
}

void IR::output(MainWindow* mw){

    ofstream fout;
    fout.open("InterCode.txt");
    if (!fout.is_open())
    {
         mw->ui->IRBrowser->append("<font color=red><b>"+QString::fromStdString("cannot open InterCode.txt") + "</b></font>" );
         return;
    }
    int i = 0;
    for (vector<Quadruple>::iterator iter = codeVec.begin(); iter != codeVec.end(); iter++, i++)
    {

        QString str = QString("%1").arg(QString::number(i), (int)(log10(codeVec.size()))+1, QLatin1Char(' ')) ;
        mw->ui->IRBrowser->append(str +"  ("  +QString::fromStdString(iter->op+"," +iter->src1 + "," + iter->src2 + "," + iter->dst + ")" ));
        fout << setw((int)(log10(codeVec.size()))+1) << i;
        fout << "  ( " << iter->op << " , "<< iter->src1 << " , "
             << iter->src2 << " , "<<iter->dst << " )"<<endl;
    }
}

Id::Id(const Symbol& sym, const string& name) : Symbol(sym)
{
    this->name = name;
}

Num::Num(const Symbol& sym, const string& number) : Symbol(sym)
{
    this->number = number;
}

Array::Array(const Symbol& sym, const string& name) : Symbol(sym)
{
    this->name = name;
}

Array::Array(string name, int dimension,string dm1_size,string dm2_size){
    this->name = name;
    this->dimension = dimension;
    this->dm1_size = dm1_size;
    this->dm2_size = dm2_size;
}

