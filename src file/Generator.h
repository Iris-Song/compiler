#include "IR.h"
#include <set>

class VarInfo
{
public:
    int waitInfo;//待用信息
    bool active;//活跃信息

    VarInfo(int waitInfo, bool active);
    VarInfo(){};
    void output(ostream& out);
};

class InfoQuadruple//四元式上的待用活跃信息
{
public:
    Quadruple qd;
    VarInfo src1_info;
    VarInfo src2_info;
    VarInfo dst_info;
    InfoQuadruple(Quadruple qd, VarInfo src1_info, VarInfo src2_info, VarInfo dst_info);
    void output(ostream& out);
};

struct InfoBlock
{
    string name;
    vector<InfoQuadruple> InfoQuadrupleVec;
    int next1;
    int next2;
};

#ifndef GENERATOR_H
#define GENERATOR_H

#define DATA_SEG_ADDR 0x10010000
#define STACK_SEG_ADDR 0x10040000
#define SREG_NUM 8
#define INT_WID 4

//  "$zero", //$0 常量0(constant value 0)
//	"$at", //$1 保留给汇编器(Reserved for assembler)
//	"$v0","$v1", //$2-$3 函数调用返回值(values for results and expression evaluation)
//	"$a0","$a1","$a2","$a3", //$4-$7 函数调用参数(arguments)
//	"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7", //$8-$15 暂时的(或随便用的)
//	"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7", //$16-$23 保存的(或如果用，需要SAVE/RESTORE的)(saved)
//	"$t8","$t9", //$24-$25 暂时的(或随便用的)
//	"$k0","k1",//操作系统／异常处理保留，至少要预留一个
//	"$gp", //$28 全局指针(Global Pointer)
//	"$sp", //$29 堆栈指针(Stack Pointer)
//	"$fp", //$30 帧指针(Frame Pointer)
//	"$ra"//$31 返回地址(return address)



class Generator{
private:
    map<string, vector<InfoBlock>> funcInfoBlocks;
    vector<string> ObjCode;

    map<string, set<string>>Rvalue;//寄存器描述
    map<string, set<string>>Avalue;//地址描述

    list<string>freeSReg;//空闲的寄存器编号$s
    map<string, int>varOffset;//各变量的存储位置
    map<string, vector<set<string> > >funcOutLive;//各函数块中基本块的出口活跃变量集
    map<string, vector<set<string> > >funcInLive;//各函数块中基本块的入口活跃变量集
    string nowFunc;//当前分析的函数
    vector<InfoBlock>::iterator nowInfoBlock;//当前分析的基本块
    vector<InfoQuadruple>::iterator nowQuadruple;//当前分析的四元式
    int stackTop;//当前栈顶

    void genInfoBlock(map<string, vector<Block> >*funcBlocks);
    bool genCode();
    bool genInfoBlockCode(map<string, vector<InfoBlock> >::iterator);
    bool genBlockCode(int nowBaseBlockIndex);
    bool genCodeForQuatenary(int &arg_num, list<pair<string, bool> > &par_list) ;

    void stVar(string reg, string var);
    void storeOutLiveVar(set<string>&outl);
    void freeVar(string var);
    string getReg();
    string allocateReg();
    string allocateReg(string var);

public:
    Generator(IR *pir,MainWindow *mw);
    void outputObjCode(MainWindow *mw);
};

#endif // GENERATOR_H
