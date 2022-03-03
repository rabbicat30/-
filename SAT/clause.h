#ifndef CLAUSE_H_INCLUDED
#define CLAUSE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>


#define OK 1
#define ERROR 0

#define TRUE 1
#define FALSE -1
#define NOTSURE 0
#define NOTCONTAIN 2

#define EVALUE(x) ((x>0)?1:-1)


typedef int status;
typedef char boolean;


typedef struct Node{
    int var;    //正为正，负取非
    struct Node* next;  //指向下一个变元
}Node;  //变元定义

typedef struct Clause{
    int length;     //子句中变元的数目
    Node* head;     //指向子句中的第一个变元
    Node* rmv;      //指向第一个被移除的变元
    status isremoved;   //子句是否被移除子句集
    struct Clause* pre;     //指向上一条子句
    struct Clause* next;    //指向下一条子句
}Clause;    //子句定义

typedef struct CNF{
    int varnum;     //变元个数
    int clausenum;  //子句的数目
    int* varfloor;  //变元所在的决策层
    int* var;       //变元取值数组，1为真，-1为假，0为不确定
    int* vsids;     //VSIDS策略积分器数组
    Clause* root;    //指向第一个子句
    struct LearnClause* learn_root;  //指向第一个学习子句
    struct LiteralIndex* lindex;//变元索引,指向第一个索引
}CNF;   //cnf子句集定义

//当冲突产生时，导致冲突的字句会被记录下来即为学习子句，可以防止在接下来的搜索过程中再次发生同样的冲突
//利用新近学习到的冲突子句可以计算出最优的回数点
//冲突分析是先找到产生冲突的字句，然后再根据冲突的字句判断回溯到的目标层数

typedef struct LearnClause{
    boolean isInStack;      //判断是否在回溯栈中
    int floor;              //决策层数
    int count;              //调用次数
    Clause* clause;         //指向该学习子句对应的子句
    struct LearnClause* next;           //指向下一条学习子句
}LearnClause;

typedef struct LiteralIndex {       //cnf索引的next指向子句索引，下标对应变元
    Node* Np;               //指向与下标一致的变元
    Clause* Cp;             //指向变元出现的子句
    struct LiteralIndex* next;//指向下一个索引
} LiteralIndex;

//相关函数声明
status createCNF();     //创建CNF
status destroyCNF();    //销毁CNF
status clearCNF();      //清空cnf答案

status createLIndex();     //创建变元索引
status addLIndex();         //添加索引
status deleteClauseLIndex();    //删除子句变元索引
status deleteallLIdex();        //删除所有变元索引

status addClause();     //添加子句
status insertClause(CNF* cnf,Clause* cl);   //顶部插入子句
Clause* removeClause();  //移除子句
status deleteClause();  //删除子句
status deleteallClause();   //删除所有子句
status deleteLearnClause(); //删除学习子句
status arrayAssign();   //变元真假取值保存到数组

boolean isClauseEmpty();    //子句是否为空
boolean isClauseUnit();     //判断是否为单元子句
boolean evaluateClause();   //子句表达式是否为真
boolean haveEmptyClause();  //是否有空子句
boolean haveClause();       //子句集是否为空

Clause* LocateUnitClause();      //是否有单元子句并返回

status printClause();   //打印子句
status printLearnClause();  //打印学习子句
status printLIndex();       //打印索引

status addvar();    //添加变元
status deletevar();     //删除变元
status backvar();       //恢复变元
Node* removevar();      //移除变元，并返回

status printcnf();

#endif // CLAUSE_H_INCLUDED
