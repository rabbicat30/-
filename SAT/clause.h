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
    int var;    //��Ϊ������ȡ��
    struct Node* next;  //ָ����һ����Ԫ
}Node;  //��Ԫ����

typedef struct Clause{
    int length;     //�Ӿ��б�Ԫ����Ŀ
    Node* head;     //ָ���Ӿ��еĵ�һ����Ԫ
    Node* rmv;      //ָ���һ�����Ƴ��ı�Ԫ
    status isremoved;   //�Ӿ��Ƿ��Ƴ��Ӿ伯
    struct Clause* pre;     //ָ����һ���Ӿ�
    struct Clause* next;    //ָ����һ���Ӿ�
}Clause;    //�Ӿ䶨��

typedef struct CNF{
    int varnum;     //��Ԫ����
    int clausenum;  //�Ӿ����Ŀ
    int* varfloor;  //��Ԫ���ڵľ��߲�
    int* var;       //��Ԫȡֵ���飬1Ϊ�棬-1Ϊ�٣�0Ϊ��ȷ��
    int* vsids;     //VSIDS���Ի���������
    Clause* root;    //ָ���һ���Ӿ�
    struct LearnClause* learn_root;  //ָ���һ��ѧϰ�Ӿ�
    struct LiteralIndex* lindex;//��Ԫ����,ָ���һ������
}CNF;   //cnf�Ӿ伯����

//����ͻ����ʱ�����³�ͻ���־�ᱻ��¼������Ϊѧϰ�Ӿ䣬���Է�ֹ�ڽ������������������ٴη���ͬ���ĳ�ͻ
//�����½�ѧϰ���ĳ�ͻ�Ӿ���Լ�������ŵĻ�����
//��ͻ���������ҵ�������ͻ���־䣬Ȼ���ٸ��ݳ�ͻ���־��жϻ��ݵ���Ŀ�����

typedef struct LearnClause{
    boolean isInStack;      //�ж��Ƿ��ڻ���ջ��
    int floor;              //���߲���
    int count;              //���ô���
    Clause* clause;         //ָ���ѧϰ�Ӿ��Ӧ���Ӿ�
    struct LearnClause* next;           //ָ����һ��ѧϰ�Ӿ�
}LearnClause;

typedef struct LiteralIndex {       //cnf������nextָ���Ӿ��������±��Ӧ��Ԫ
    Node* Np;               //ָ�����±�һ�µı�Ԫ
    Clause* Cp;             //ָ���Ԫ���ֵ��Ӿ�
    struct LiteralIndex* next;//ָ����һ������
} LiteralIndex;

//��غ�������
status createCNF();     //����CNF
status destroyCNF();    //����CNF
status clearCNF();      //���cnf��

status createLIndex();     //������Ԫ����
status addLIndex();         //�������
status deleteClauseLIndex();    //ɾ���Ӿ��Ԫ����
status deleteallLIdex();        //ɾ�����б�Ԫ����

status addClause();     //����Ӿ�
status insertClause(CNF* cnf,Clause* cl);   //���������Ӿ�
Clause* removeClause();  //�Ƴ��Ӿ�
status deleteClause();  //ɾ���Ӿ�
status deleteallClause();   //ɾ�������Ӿ�
status deleteLearnClause(); //ɾ��ѧϰ�Ӿ�
status arrayAssign();   //��Ԫ���ȡֵ���浽����

boolean isClauseEmpty();    //�Ӿ��Ƿ�Ϊ��
boolean isClauseUnit();     //�ж��Ƿ�Ϊ��Ԫ�Ӿ�
boolean evaluateClause();   //�Ӿ���ʽ�Ƿ�Ϊ��
boolean haveEmptyClause();  //�Ƿ��п��Ӿ�
boolean haveClause();       //�Ӿ伯�Ƿ�Ϊ��

Clause* LocateUnitClause();      //�Ƿ��е�Ԫ�Ӿ䲢����

status printClause();   //��ӡ�Ӿ�
status printLearnClause();  //��ӡѧϰ�Ӿ�
status printLIndex();       //��ӡ����

status addvar();    //��ӱ�Ԫ
status deletevar();     //ɾ����Ԫ
status backvar();       //�ָ���Ԫ
Node* removevar();      //�Ƴ���Ԫ��������

status printcnf();

#endif // CLAUSE_H_INCLUDED
