#include "lexer.h"

bool Lexer::IsLetter(char ch)
{
    if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z'))
        return true;
    else      
        return false;

}

bool Lexer::IsDigit(char ch)
{
    if (ch >= '0'&&ch <= '9')
        return true;
    else
        return false;
}

char Lexer::getNextChar(string src){
    char c=src[i_of_src];
    while(i_of_src<src.length()){
        c=src[i_of_src];
        if(c==' '||c=='\t'){
            i_of_src++;
            continue;
        }
        break;
    }
    return c;
}

void Lexer::lexer(MainWindow *mw){

    term.reset();
    QString src =mw->ui->textEdit->toPlainText();
    i_of_src=0;
    string s= src.toStdString();

    while(i_of_src<s.length()){
        if(!this->scan(s,mw))
            break;
        i_of_src++;
    }
    //check if the end of terminal is "#"
    if(term.termVec.empty()||term.termVec[term.termVec.size()-1].first!=END){
        term.insert(END,"#");
    }

}

bool Lexer::scan(string src,MainWindow *mw)
{
    char c= getNextChar(src);

    switch(c){
    case'\n':
        break;
    case'#':
        writeout(END,0,mw);
        term.insert(END,"#");
        return 0;
        break;
    case'(':
        writeout(LB,0,mw);
        term.insert(LB,"(");
        break;
    case')':
        writeout(RB,0,mw);
        term.insert(RB,")");
        break;
    case'{':
        writeout(BLB,0,mw);
        term.insert(BLB,"{");
        break;
    case'}':
        writeout(BRB,0,mw);
        term.insert(BRB,"}");
        break;
    case'[':
        writeout(MLB,0,mw);
        term.insert(MLB,"[");
        break;
    case']':
        writeout(MRB,0,mw);
        term.insert(MRB,"]");
        break;
    case';':
        writeout(SEM,0,mw);
        term.insert(SEM,";");
        break;
    case',':
        writeout(COM,0,mw);
        term.insert(COM,",");
        break;
    case'*':
        writeout(MUL,0,mw);
        term.insert(MUL,"*");
        break;
    case'-':
        writeout(DEC,0,mw);
        term.insert(DEC,"-");
        break;
    case'+':
        writeout(ADD,0,mw);
        term.insert(ADD,"+");
        break;
    case'/':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='/'){//行注释
            i_of_src++;
            while(i_of_src<src.length()){
                if(src[i_of_src]=='\n')
                    break;
                else i_of_src++;
            }
        }
        else if(i_of_src+1<src.length()&&src[i_of_src+1]=='*'){//段注释
            i_of_src++;
            while(i_of_src<src.length()){
                if(i_of_src+1<src.length()&&src[i_of_src]=='*'&&src[i_of_src+1]=='/'){
                    i_of_src++;
                    break;}
                else i_of_src++;
            }
        }
        else{
            writeout(DIV,0,mw);//除法
            term.insert(DIV,"/");
        }
        break;
    case'=':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='='){
            i_of_src++;
            writeout(EQ,0,mw);
            term.insert(EQ,"==");
        }
        else{
            writeout(VL,0,mw);
            term.insert(VL,"=");
        }
        break;
    case'>':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='='){
            i_of_src++;
            writeout(GE,0,mw);
            term.insert(GE,">=");
        }
        else{
            writeout(GT,0,mw);
            term.insert(GT,">");
        }
        break;
    case'<':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='='){
            i_of_src++;
            writeout(LE,0,mw);
            term.insert(LE,"<=");
        }
        else{
            writeout(LT,0,mw);
            term.insert(LT,"<");
        }
        break;
    case'!':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='='){
            i_of_src++;
            writeout(NEQ,0,mw);
            term.insert(NEQ,"!=");
        }
        else{
            writeout(ERRORCHAR,0,mw);
            QMessageBox mesg;
            mesg.warning(mw,"warning","find char can not identify");
            return 0;
        }
        break;
    case'|':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='|'){
            i_of_src++;
            writeout(OR,0,mw);
            term.insert(OR,"||");
        }
        else{
            writeout(ERRORCHAR,0,mw);
            QMessageBox mesg;
            mesg.warning(mw,"warning","find char can not identify");
            return 0;
        }
        break;
    case'&':
        if(i_of_src+1<src.length()&&src[i_of_src+1]=='&'){
            i_of_src++;
            writeout(OR,0,mw);
            term.insert(AND,"&&");
        }
        else{
            writeout(ERRORCHAR,0,mw);
            QMessageBox mesg;
            mesg.warning(mw,"warning","find char can not identify");
            return 0;
        }
        break;
    default:
        if(this->IsDigit(c)){
            string num;
            while(i_of_src<src.length()){
                if(IsDigit(src[i_of_src])){
                    num+=src[i_of_src];
                    i_of_src++;
                }
                else{
                    i_of_src--;
                    break;
                }
            }
            term.insert(INT10,num);
            writeout(INT10,stoi(num),mw);
        }
        else if(IsLetter(c)){
            string s;
            while(i_of_src<src.length()){
                if(IsLetter(src[i_of_src])||IsDigit(src[i_of_src])){
                    s+=src[i_of_src];
                    i_of_src++;
                }
                else{
                    i_of_src--;
                    break;
                }
            }
            if(s=="int"){
                writeout(INT,0,mw);
                term.insert(INT,"int");
            }
            else if(s=="void"){
                writeout(VOID,0,mw);
                term.insert(VOID,"void");
            }
            else if(s=="if"){
                writeout(IF,0,mw);
                term.insert(IF,"if");
            }
            else if(s=="else"){
                writeout(ELSE,0,mw);
                term.insert(ELSE,"else");
            }
            else if(s=="do"){
                writeout(DO,0,mw);
                term.insert(DO,"do");
            }
            else if(s=="while"){
                writeout(WHILE,0,mw);
                term.insert(WHILE,"while");
            }
            else if(s=="for"){
                writeout(FOR,0,mw);
                term.insert(FOR,"for");
            }
            else if(s=="return"){
                writeout(RETURN,0,mw);
                term.insert(RETURN,"return");
            }
            else {
                //qDebug()<<src[i_of_src+1];
                term.insert(ID,s,true);
                writeout(ID,term.IDnum,mw);
            }
        }
        else {
            writeout(ERRORCHAR,c,mw);
            QMessageBox mesg;
            mesg.warning(mw,"warning","find char can not identify");
            return 0;
        }
    }
    return 1;
}

void Lexer::writeout(int type,int num,MainWindow *mw)
{
    switch(type){
    case ERRORCHAR:{
        mw->ui->lexerBrowser->append("<font color=red>&lt;$ERRORCHAR,"+ QString(num) +"&gt;</font>");
        break;
    }
    case INT10:case INT16:case INT8:
        mw->ui->lexerBrowser->append("<$IntNum,"+ QString::number(num) +">");
        break;
    case WHILE:
        mw->ui->lexerBrowser->append("<$WHILE,->");
        break;
    case DO:
        mw->ui->lexerBrowser->append("<$DO,->");
        break;
    case IF:
        mw->ui->lexerBrowser->append("<$IF,->");
        break;
    case VOID:
        mw->ui->lexerBrowser->append("<$VOID,->");
        break;
    case INT:
        mw->ui->lexerBrowser->append("<$INT,->");
        break;
    case ELSE:
        mw->ui->lexerBrowser->append("<$ELSE,->");
        break;
    case FOR:
        mw->ui->lexerBrowser->append("<$FOR,->");
        break;
    case RETURN:
        mw->ui->lexerBrowser->append("<$RETURN,->");
        break;
    case ID:
        mw->ui->lexerBrowser->append("<$ID,"+ QString::number(num) +">");
        break;
    case RB:
        mw->ui->lexerBrowser->append("<$),->");
        break;
    case LB:
        mw->ui->lexerBrowser->append("<$(,->");
        break;
    case SEM:
        mw->ui->lexerBrowser->append("<$;,->");
        break;
    case DIV:
        mw->ui->lexerBrowser->append("<$/,->");
        break;
    case MUL:
        mw->ui->lexerBrowser->append("<$*,->");
        break;
    case DEC:
        mw->ui->lexerBrowser->append("<$-,->");
        break;
    case ADD:
        mw->ui->lexerBrowser->append("<$+,->");
        break;
    case VL:
        mw->ui->lexerBrowser->append("<$=,->");
        break;
    case EQ:
        mw->ui->lexerBrowser->append("<$==,->");
        break;
    case NEQ:
        mw->ui->lexerBrowser->append("<$!=,->");
        break;
    case GE:
        mw->ui->lexerBrowser->append("<$>=,->");
        break;
    case GT:
        mw->ui->lexerBrowser->append("<$>,->");
        break;
    case LE:
        mw->ui->lexerBrowser->append("<$<=,->");
        break;
    case LT:
        mw->ui->lexerBrowser->append("<$<,->");
        break;
    case BLB:
        mw->ui->lexerBrowser->append("<${,->");
        break;
    case BRB:
        mw->ui->lexerBrowser->append("<$},->");
        break;
    case MLB:
        mw->ui->lexerBrowser->append("<$[,->");
        break;
    case MRB:
        mw->ui->lexerBrowser->append("<$],->");
        break;
    case COM:
        mw->ui->lexerBrowser->append("<$,,->");
        break;
    case END:
        mw->ui->lexerBrowser->append("<#,->");
        break;
    case AND:
        mw->ui->lexerBrowser->append("<&&,->");
        break;
    case OR:
        mw->ui->lexerBrowser->append("<||,->");
        break;
    default:break;
    }
    return;
}
