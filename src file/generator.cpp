#include "generator.h"
#include "qdebug.h"

VarInfo::VarInfo(int waitInfo, bool active)
{
    this->waitInfo = waitInfo;
    this->active = active;
}

void VarInfo::output(ostream& out)
{
    out << "(";
    if (waitInfo == -1)
        out << "^";
    else
        out << waitInfo;
    out << ",";
    if (active)
        out << "y";
    else
        out << "^";
    out << ")";
}

InfoQuadruple::InfoQuadruple(Quadruple qd, VarInfo src1_info, VarInfo src2_info, VarInfo dst_info)
{
    this->qd = qd;
    this->src1_info = src1_info;
    this->src2_info = src2_info;
    this->dst_info = dst_info;
}

void InfoQuadruple::output(ostream& out)
{
    out << "(" << qd.op << "," << qd.src1 << "," << qd.src2 << "," << qd.dst << ")";
    src1_info.output(out);
    src2_info.output(out);
    dst_info.output(out);
}

Generator::Generator(IR* pir,MainWindow *mw)
{
    if(!pir){
        mw->ui->CodeBrowser->append("<font color='red'><b>have not generator intermediate code!</b></font>");
        return;
    }
    genInfoBlock(&pir->funcBlocks);
    if(genCode()){
        outputObjCode(mw);
    }
}

bool isVar(string name)
{
    return isalpha(name[0]);
}

bool isNum(string name)
{
    return isdigit(name[0]);
}

bool isControlOp(string op)
{
    if (op[0] == 'j' || op == "call" || op == "return" || op == "get")
        return true;
    return false;
}

void Generator::genInfoBlock(map<string, vector<Block>>* funcBlocks)
{

    for (map<string, vector<Block> >::iterator fbiter = funcBlocks->begin(); fbiter != funcBlocks->end(); fbiter++)
    {
        vector<InfoBlock> infoBlock;
        vector<Block>& blocks = fbiter->second;
        vector<set<string> >inLive, outLive, DEF, USE;//入口活跃变量，出口活跃变量，定义变量，使用变量
        //活跃变量的数据流方程
        //确定DEF，USE
        for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++)
        {
            set<string>def, use;//定值，使用
            for (vector<Quadruple>::iterator qiter = biter->codes.begin(); qiter != biter->codes.end(); qiter++)
            {
                if (qiter->op == "j" || qiter->op == "call"){}//不存在变量
                else if (qiter->op[0] == 'j')
                {//j>= j<=,j==,j!=,j>,j<
                    if (isVar(qiter->src1) && def.count(qiter->src1) == 0) //如果源操作数1还没有被定值
                        use.insert(qiter->src1);
                    if (isVar(qiter->src2) && def.count(qiter->src2) == 0) //如果源操作数2还没有被定值
                        use.insert(qiter->src2);
                }
                else
                {
                    if (isVar(qiter->src1) && def.count(qiter->src1) == 0) //如果源操作数1还没有被定值
                        use.insert(qiter->src1);
                    if (isVar(qiter->src2) && def.count(qiter->src2) == 0) //如果源操作数2还没有被定值
                        use.insert(qiter->src2);
                    if (isVar(qiter->dst) && use.count(qiter->dst) == 0) //如果目的操作数还没有被引用
                        def.insert(qiter->dst);
                }
            }
            DEF.push_back(def);
            USE.push_back(use);
            inLive.push_back(use);
            outLive.push_back(set<string>());
        }

        //确定inLive，outLive
        bool change = true;
        while (change)
        {
            change = false;
            int blockIndex = 0;
            for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++, blockIndex++)
            {//for each block
                int next1 = biter->next1;
                int next2 = biter->next2;
                if (next1 != -1)
                {
                    for (set<string>::iterator inlIter = inLive[next1].begin(); inlIter != inLive[next1].end(); inlIter++)
                    {
                        if (outLive[blockIndex].insert(*inlIter).second == true)//插入下一基本块入口活跃变量成功
                        {
                            if (DEF[blockIndex].count(*inlIter) == 0)
                                inLive[blockIndex].insert(*inlIter);
                            change = true;
                        }
                    }
                }
                if (next2 != -1)
                {
                    for (set<string>::iterator inlIter = inLive[next2].begin(); inlIter != inLive[next2].end(); inlIter++)
                    {
                        if (outLive[blockIndex].insert(*inlIter).second == true)
                        {
                            if (DEF[blockIndex].count(*inlIter) == 0)
                                inLive[blockIndex].insert(*inlIter);
                            change = true;
                        }
                    }
                }
            }
        }

        funcOutLive[fbiter->first] = outLive;
        funcInLive[fbiter->first] = inLive;

        for (vector<Block>::iterator iter = blocks.begin(); iter != blocks.end(); iter++)
        {
            InfoBlock ifBlock;
            ifBlock.next1 = iter->next1;
            ifBlock.next2 = iter->next2;
            ifBlock.name = iter->name;
            for (vector<Quadruple>::iterator qIter = iter->codes.begin(); qIter != iter->codes.end(); qIter++)
                ifBlock.InfoQuadrupleVec.push_back(InfoQuadruple(*qIter, VarInfo(-1, false), VarInfo(-1, false), VarInfo(-1, false)));
            infoBlock.push_back(ifBlock);
        }

        vector<map<string, VarInfo> > symTables;//每个基本块对应一张符号表
        //初始化符号表
        for (vector<Block>::iterator biter = blocks.begin(); biter != blocks.end(); biter++)
        {//for each block
            map<string, VarInfo>symTable;
            for (vector<Quadruple>::iterator citer = biter->codes.begin(); citer != biter->codes.end(); citer++)
            {//for each Quadruple
                if (citer->op == "j" || citer->op == "call"){}
                else if (citer->op[0] == 'j')
                {//j>= j<=,j==,j!=,j>,j<
                    if (isVar(citer->src1))
                        symTable[citer->src1] = VarInfo{ -1,false };
                    if (isVar(citer->src2))
                        symTable[citer->src2] = VarInfo{ -1,false };
                }
                else
                {
                    if (isVar(citer->src1))
                        symTable[citer->src1] = VarInfo{ -1,false };
                    if (isVar(citer->src2))
                        symTable[citer->src2] = VarInfo{ -1,false };
                    if (isVar(citer->dst))
                        symTable[citer->dst] = VarInfo{ -1,false };
                }
            }
            symTables.push_back(symTable);
        }

        int blockIndex = 0;
        //set live var --true
        for (vector<set<string> >::iterator iter = outLive.begin(); iter != outLive.end(); iter++, blockIndex++)
            for (set<string>::iterator liter = iter->begin(); liter != iter->end(); liter++)
                symTables[blockIndex][*liter] = VarInfo{ -1,true };

        blockIndex = 0;
        //计算每个四元式的待用信息和活跃信息
        for (vector<InfoBlock>::iterator ibiter = infoBlock.begin(); ibiter != infoBlock.end(); ibiter++, blockIndex++)
        {
            int codeIndex = ibiter->InfoQuadrupleVec.size() - 1;
            for (vector<InfoQuadruple>::reverse_iterator citer = ibiter->InfoQuadrupleVec.rbegin(); citer != ibiter->InfoQuadrupleVec.rend(); citer++, codeIndex--) {//逆序遍历基本块中的代码
                if (citer->qd.op == "j" || citer->qd.op == "call"){}
                else if (citer->qd.op[0] == 'j')
                {//j>= j<=,j==,j!=,j>,j<
                    if (isVar(citer->qd.src1))
                    {
                        citer->src1_info = symTables[blockIndex][citer->qd.src1];
                        symTables[blockIndex][citer->qd.src1] = VarInfo{ codeIndex,true };
                    }
                    if (isVar(citer->qd.src2))
                    {
                        citer->src2_info = symTables[blockIndex][citer->qd.src2];
                        symTables[blockIndex][citer->qd.src2] = VarInfo{ codeIndex,true };
                    }
                }
                else
                {
                    if (isVar(citer->qd.src1))
                    {
                        citer->src1_info = symTables[blockIndex][citer->qd.src1];
                        symTables[blockIndex][citer->qd.src1] = VarInfo{ codeIndex,true };
                    }
                    if (isVar(citer->qd.src2))
                    {
                        citer->src2_info = symTables[blockIndex][citer->qd.src2];
                        symTables[blockIndex][citer->qd.src2] = VarInfo{ codeIndex,true };
                    }
                    if (isVar(citer->qd.dst))
                    {
                        citer->dst_info = symTables[blockIndex][citer->qd.dst];
                        symTables[blockIndex][citer->qd.dst] = VarInfo{ -1,false };
                    }
                }
            }
        }
        funcInfoBlocks[fbiter->first] = infoBlock;
    }
}

bool Generator::genCode()
{
    ObjCode.push_back("lui $sp,0x10040000");//$sp--esp
    ObjCode.push_back("lui $fp,0x1003FFFC");//$fp--ebp
    ObjCode.push_back("j main");
    for (map<string, vector<InfoBlock> >::iterator iter = funcInfoBlocks.begin(); iter != funcInfoBlocks.end(); iter++){
        if(!genInfoBlockCode(iter))
            return false;
    }
    ObjCode.push_back("end:");
    return true;
}

bool Generator::genInfoBlockCode(map<string, vector<InfoBlock> >::iterator iter)
{
    varOffset.clear();
    nowFunc = iter->first;
    vector<InfoBlock>& AinfoBlock = iter->second;
    for (vector<InfoBlock>::iterator iter = AinfoBlock.begin(); iter != AinfoBlock.end(); iter++)
    {
         nowInfoBlock = iter;
         genBlockCode(iter - AinfoBlock.begin());
    }
    return true;
}

bool Generator::genBlockCode(int nowBaseBlockIndex)
{
    int arg_num = 0;//par的实参个数
    list<pair<string, bool> > par_list;//函数调用用到的实参集list<实参名,是否活跃>
    Avalue.clear();
    Rvalue.clear();

    //入口活跃变量地址
    set<string>& inl = funcInLive[nowFunc][nowBaseBlockIndex];
    for (set<string>::iterator iter = inl.begin(); iter != inl.end(); iter++)
        Avalue[*iter].insert(*iter);

    //初始化空闲寄存器
    freeSReg.clear();
    for (int i = 0; i < SREG_NUM; i++)
        freeSReg.push_back(string("$s") + to_string(i));

    ObjCode.push_back(nowInfoBlock->name + ":");
    if (nowBaseBlockIndex == 0)//是func的第一个block
    {
        if (nowFunc != "main")
            ObjCode.push_back("sw $ra 4($sp)");//返回地址压栈
        stackTop = INT_WID*2;
    }
    for (vector<InfoQuadruple>::iterator qIter = nowInfoBlock->InfoQuadrupleVec.begin(); qIter != nowInfoBlock->InfoQuadrupleVec.end(); qIter++)
    {//对每个四元式
        nowQuadruple = qIter;
        //如果是基本块的最后一条语句
        if (qIter == nowInfoBlock->InfoQuadrupleVec.end()-1 )
        {
            //如果最后一条语句是控制语句，则先将出口活跃变量保存，再进行跳转(j,call,return)
            if (isControlOp(qIter->qd.op))
            {
                storeOutLiveVar(funcOutLive[nowFunc][nowBaseBlockIndex]);
                genCodeForQuatenary(arg_num, par_list);
            }
            //如果最后一条语句不是控制语句（是赋值语句），则先计算，再将出口活跃变量保存
            else
            {
                genCodeForQuatenary(arg_num, par_list);
                storeOutLiveVar(funcOutLive[nowFunc][nowBaseBlockIndex]);
            }
        }
        else
            genCodeForQuatenary(arg_num, par_list);
    }
    return true;
}

//基本块出口，将出口活跃变量保存在内存
void Generator::storeOutLiveVar(set<string>& outLiveVar)
{
    for (set<string>::iterator oiter = outLiveVar.begin(); oiter != outLiveVar.end(); oiter++)
    {
        string reg;//活跃变量所在的寄存器名称
        bool inFlag = false;//活跃变量在内存中的标志
        for (set<string>::iterator aiter = Avalue[*oiter].begin(); aiter != Avalue[*oiter].end(); aiter++)
        {
            if ((*aiter)[0] != '$')
            {//该活跃变量已经存储在内存中
                inFlag = true;
                break;
            }
            else
                reg = *aiter;
        }
        if (!inFlag) //如果该活跃变量不在内存中，则将reg中的var变量存入内存
            stVar(reg, *oiter);
    }
}

bool Generator::genCodeForQuatenary(int &arg_num, list<pair<string, bool> > &par_list) {

    if (nowQuadruple->qd.op == "j")
        ObjCode.push_back("j " + nowQuadruple->qd.dst);
    else if (nowQuadruple->qd.op[0] == 'j')
    {//j>= j<=,j==,j!=,j>,j<
        string op;
        if (nowQuadruple->qd.op == "j>=")
            op = "bge";
        else if (nowQuadruple->qd.op == "j>")
            op = "bgt";
        else if (nowQuadruple->qd.op == "j==")
            op = "beq";
        else if (nowQuadruple->qd.op == "j!=")
            op = "bne";
        else if (nowQuadruple->qd.op == "j<")
            op = "blt";
        else if (nowQuadruple->qd.op == "j<=")
            op = "ble";
        string pos1 = allocateReg(nowQuadruple->qd.src1);
        string pos2 = allocateReg(nowQuadruple->qd.src2);
        ObjCode.push_back(op + " " + pos1 + " " + pos2 + " " + nowQuadruple->qd.dst);
        if (!nowQuadruple->src1_info.active)
            freeVar(nowQuadruple->qd.src1);
        if (!nowQuadruple->src2_info.active)
            freeVar(nowQuadruple->qd.src2);
    }
    else if (nowQuadruple->qd.op == "par")
        par_list.push_back(pair<string, bool>(nowQuadruple->qd.src1, nowQuadruple->src1_info.active));
    else if (nowQuadruple->qd.op == "call")
    {
        //参数压栈
        for (list<pair<string, bool> >::iterator aiter = par_list.begin(); aiter != par_list.end(); aiter++)
        {
            string pos = allocateReg(aiter->first);
            ObjCode.push_back(string("sw ") + pos + " " + to_string(stackTop + INT_WID * (++arg_num + 1)) + "($sp)");
            if (!aiter->second)
                freeVar(aiter->first);
        }
        //更新$sp
        ObjCode.push_back(string("sw $sp ") + to_string(stackTop) + "($sp)");
        ObjCode.push_back(string("addi $sp $sp ") + to_string(stackTop));
        //跳转到对应函数
        ObjCode.push_back(string("jal ") + nowQuadruple->qd.src1);
        //恢复现场
        ObjCode.push_back(string("lw $sp 0($sp)"));
    }
    else if (nowQuadruple->qd.op == "return")
    {
        if (isNum(nowQuadruple->qd.src1)) //返回值为数字
            ObjCode.push_back("addi $v0 $zero " + nowQuadruple->qd.src1);
        else if (isVar(nowQuadruple->qd.src1))
        {//返回值为变量
            set<string>::iterator piter = Avalue[nowQuadruple->qd.src1].begin();
            if ((*piter)[0] == '$')
                ObjCode.push_back(string("add $v0 $zero ") + *piter);
            else
                ObjCode.push_back(string("lw $v0 ") + to_string(varOffset[*piter]) + "($sp)");
        }
        if (nowFunc == "main")
            ObjCode.push_back("j end");
        else
        {
            ObjCode.push_back("lw $ra 4($sp)");
            ObjCode.push_back("jr $ra");
        }
    }
    else if (nowQuadruple->qd.op == "get")
    {
        varOffset[nowQuadruple->qd.dst] = stackTop;
        stackTop += INT_WID;
        Avalue[nowQuadruple->qd.dst].insert(nowQuadruple->qd.dst);
    }
    else if (nowQuadruple->qd.op == "="||nowQuadruple->qd.op == "[]=")
    {
        string src1Pos;
        if (nowQuadruple->qd.src1 == "@RETURN")
            src1Pos = "$v0";
        else
            src1Pos = allocateReg(nowQuadruple->qd.src1);
        Rvalue[src1Pos].insert(nowQuadruple->qd.dst);
        Avalue[nowQuadruple->qd.dst].insert(src1Pos);
    }
    else
    {// + - * /
        string src1Pos = allocateReg(nowQuadruple->qd.src1);
        string src2Pos = allocateReg(nowQuadruple->qd.src2);
        string desPos = getReg();
        if (nowQuadruple->qd.op == "+")
            ObjCode.push_back(string("add ") + desPos + " " + src1Pos + " " + src2Pos);
        else if (nowQuadruple->qd.op == "-")
            ObjCode.push_back(string("sub ") + desPos + " " + src1Pos + " " + src2Pos);
        else if (nowQuadruple->qd.op == "*")
            ObjCode.push_back(string("mul ") + desPos + " " + src1Pos + " " + src2Pos);
        else if (nowQuadruple->qd.op == "/")
        {
            ObjCode.push_back(string("div ") + src1Pos + " " + src2Pos);
            ObjCode.push_back(string("mflo ") + desPos);
        }
        if (!nowQuadruple->src1_info.active)
            freeVar(nowQuadruple->qd.src1);
        if (!nowQuadruple->src2_info.active)
            freeVar(nowQuadruple->qd.src2);
    }
    return true;
}

void Generator::outputObjCode(MainWindow *mw)
{
    ofstream fout;
    fout.open("objectCode.asm");
    if (!fout.is_open())
    {
         mw->ui->CodeBrowser->append("<font color=red><b>"+QString::fromStdString("cannot open objectCode.asm") + "</b></font>" );
         return;
    }
    for(int i=0;i<ObjCode.size();i++){
        fout<<ObjCode[i]<<endl;
        mw->ui->CodeBrowser->append(QString::fromStdString(ObjCode[i]));
    }
}

//释放寄存器里的变量
void Generator::freeVar(string var)
{
    for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++)
    {
        if ((*iter)[0] == '$')
        {
            Rvalue[*iter].erase(var);
            if (Rvalue[*iter].size() == 0 && (*iter)[1] == 's')
                freeSReg.push_back(*iter);
        }
    }
    Avalue[var].clear();
}

//为引用变量分配寄存器
string Generator::allocateReg()
{
    //如果有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
    string reg;
    if (freeSReg.size())
    {
        reg = freeSReg.back();
        freeSReg.pop_back();
        return reg;
    }
    /*
    从已分配的寄存器中选取一个Ri为所需要的寄存器R。最好使得Ri满足以下条件：
    占用Ri的变量的值也同时存放在该变量的贮存单元中
    或者在基本块中要在最远的将来才会引用到或不会引用到。
    */
    int maxNextPos = 0;
    for (map<string, set<string> >::iterator iter = Rvalue.begin(); iter != Rvalue.end(); iter++)
    {//遍历所有的寄存器
        int nextPos = INT_MAX;
        for (set<string>::iterator viter = iter->second.begin(); viter != iter->second.end(); viter++)
        {//遍历寄存器中储存的变量
            bool inFlag = false;//变量已在其他地方存储的标志
            for (set<string>::iterator aiter = Avalue[*viter].begin(); aiter != Avalue[*viter].end(); aiter++)
            {//遍历变量的存储位置
                if (*aiter != iter->first)
                {//如果变量存储在其他地方
                    inFlag = true;
                    break;
                }
            }
            if (!inFlag)
            {//如果变量仅存储在寄存器中，就看未来在何处会引用该变量
                for (vector<InfoQuadruple>::iterator cIter = nowQuadruple; cIter != nowInfoBlock->InfoQuadrupleVec.end(); cIter++)
                {
                    if (*viter == cIter->qd.src1 || *viter == cIter->qd.src2)
                        nextPos = cIter - nowQuadruple;
                    else if (*viter == cIter->qd.dst)
                        break;
                }
            }
        }
        if (nextPos == INT_MAX)//找到变量已在其他地方存储
        {
            reg = iter->first;
            break;
        }
        else if (nextPos > maxNextPos)
        {
            maxNextPos = nextPos;
            reg = iter->first;
        }
    }

    for (set<string>::iterator iter = Rvalue[reg].begin(); iter != Rvalue[reg].end(); iter++)
    {
        //对ret的寄存器中保存的变量*iter，他们都将不再存储在ret中
        Avalue[*iter].erase(reg);
        //如果V的地址描述数组AVALUE[V]说V还保存在R之外的其他地方，则不需要生成存数指令
        if (Avalue[*iter].size() > 0){}
        //如果V不会在此之后被使用，则不需要生成存数指令
        else
        {
            bool storeFlag = true;
            vector<InfoQuadruple>::iterator cIter;
            for (cIter = nowQuadruple; cIter != nowInfoBlock->InfoQuadrupleVec.end(); cIter++)
            {
                if (cIter->qd.src1 == *iter || cIter->qd.src2 == *iter)
                {//V在本基本块中被引用
                    storeFlag = true;
                    break;
                }
                if (cIter->qd.dst == *iter)
                {//V在本基本块中被赋值
                    storeFlag = false;
                    break;
                }
            }
            if (cIter == nowInfoBlock->InfoQuadrupleVec.end())
            {//V在本基本块中未被引用，且也没有被赋值
                int index = nowInfoBlock - funcInfoBlocks[nowFunc].begin();
                if (funcOutLive[nowFunc][index].count(*iter) == 1) //如果此变量是出口之后的活跃变量
                    storeFlag = true;
                else
                    storeFlag = false;
            }
            if (storeFlag)	//生成存数指令
                stVar(reg, *iter);
        }
    }
    Rvalue[reg].clear();//清空ret寄存器中保存的变量
    return reg;
}

//为引用变量分配寄存器
string Generator::allocateReg(string var)
{
    if (isNum(var))//若是数字
    {
        string reg = allocateReg();
        ObjCode.push_back(string("addi ") + reg + " $zero " + var);
        return reg;
    }
    for (set<string>::iterator iter = Avalue[var].begin(); iter != Avalue[var].end(); iter++)
        if ((*iter)[0] == '$') //如果变量已经保存在某个寄存器中
            return *iter;//直接返回该寄存器
    //如果该变量没有在某个寄存器中
    string reg = allocateReg();
    ObjCode.push_back(string("lw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
    Avalue[var].insert(reg);
    Rvalue[reg].insert(var);
    return reg;
}

//存储变量到内存
void Generator::stVar(string reg, string var)
{
    if (varOffset.find(var) != varOffset.end())
    {//如果已经为*iter分配好了存储空间
        ObjCode.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
    }
    else
    {
        varOffset[var] = stackTop;
        stackTop += INT_WID;
        ObjCode.push_back(string("sw ") + reg + " " + to_string(varOffset[var]) + "($sp)");
    }
    Avalue[var].insert(var);
}

//为目标变量分配寄存器
string Generator::getReg()
{
    //A:=B op C
    //如果B的现行值在某个寄存器Ri中，RVALUE[Ri]中只包含B
    //此外，或者B与A是同一个标识符或者B的现行值在执行四元式A:=B op C之后不会再引用
    //则选取Ri为所需要的寄存器R
    if (!isNum(nowQuadruple->qd.src1))//如果src1不是数字
    {
        //遍历src1所在的寄存器
        set<string>&src1pos = Avalue[nowQuadruple->qd.src1];
        for (set<string>::iterator iter = src1pos.begin(); iter != src1pos.end(); iter++)
        {
            if ((*iter)[0] == '$')
            {
                if (Rvalue[*iter].size() == 1) {//如果该寄存器中值仅仅存有src1
                    if (nowQuadruple->qd.dst == nowQuadruple->qd.src1 || !nowQuadruple->src1_info.active)
                    {//如果A,B是同一标识符或B以后不活跃
                        Avalue[nowQuadruple->qd.dst].insert(*iter);
                        Rvalue[*iter].insert(nowQuadruple->qd.dst);
                        return *iter;
                    }
                }
            }
        }
    }
    //为目标变量分配可能不正确
    string reg = allocateReg();
    Avalue[nowQuadruple->qd.dst].insert(reg);
    Rvalue[reg].insert(nowQuadruple->qd.dst);
    return reg;
}
