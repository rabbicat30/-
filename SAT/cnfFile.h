#ifndef CNFFILE_H_INCLUDED
#define CNFFILE_H_INCLUDED

#include "clause.h"

#define LITERALMAX 200

typedef int status;

CNF* loadCNF();     //��ȡcnf�ļ�
status changetoRes(char* path,char* cnfpath );      //����������ͬ��res�ļ�
status saveRes(char* respath,status dpll,CNF* cnf,int time);        //����res�ļ�
status saveCNF();   //����cnf�ļ�

#endif // CNFFILE_H_INCLUDED
