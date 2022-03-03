#ifndef DPLL_H_INCLUDED
#define DPLL_H_INCLUDED

#include "clause.h"
#include <math.h>

#define CLAUSE 0
#define LITERAL 1
#define SPLIT 2

typedef struct Stack{
    int tag;        //��ʶ�Ӿ���Ԫ
    int floor;      //���߲���
    Node* Np;
    Clause* Cp;
    struct Stack* next;     //��һ���޸���
}ChangeStack;   //�����޸�

typedef struct Floor{
    int a;      //��ͻ��Ԫ
    int floor;      //��Ԫ���ڵľ��߲�
}Floor;

status DPLL();
status advancedDPLL();

status SimplifySingleClause();  //�����Ӿ�
status MOM(CNF* cnf);
status VSIDSS(CNF* cnf);
status combineStrategy();
status saveChange();        //�����޸�
status backChange();        //�ָ��޸�
status createLearnClause(CNF* cnf,int* a,int i,int x);    //����ѧϰ�Ӿ�
status backLearnClause(CNF* cnf,int floor);         //ѧϰ�Ӿ����
status deleteRepeateLearnClause();                 //ɾ���ظ���ѧϰ�Ӿ�
status backAssign(CNF* cnf,int floor);              //��ֵ����

status compare_Des(const void* a,const void* b);

status check();
#endif //DPLL_H_INCLUDED
