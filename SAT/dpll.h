#ifndef DPLL_H_INCLUDED
#define DPLL_H_INCLUDED

#include "clause.h"
#include <math.h>

#define CLAUSE 0
#define LITERAL 1
#define SPLIT 2

typedef struct Stack{
    int tag;        //标识子句或变元
    int floor;      //决策层数
    Node* Np;
    Clause* Cp;
    struct Stack* next;     //下一个修改区
}ChangeStack;   //保存修改

typedef struct Floor{
    int a;      //冲突变元
    int floor;      //变元所在的决策层
}Floor;

status DPLL();
status advancedDPLL();

status SimplifySingleClause();  //化简单子句
status MOM(CNF* cnf);
status VSIDSS(CNF* cnf);
status combineStrategy();
status saveChange();        //保存修改
status backChange();        //恢复修改
status createLearnClause(CNF* cnf,int* a,int i,int x);    //生成学习子句
status backLearnClause(CNF* cnf,int floor);         //学习子句回溯
status deleteRepeateLearnClause();                 //删除重复的学习子句
status backAssign(CNF* cnf,int floor);              //赋值回溯

status compare_Des(const void* a,const void* b);

status check();
#endif //DPLL_H_INCLUDED
