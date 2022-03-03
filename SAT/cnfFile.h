#ifndef CNFFILE_H_INCLUDED
#define CNFFILE_H_INCLUDED

#include "clause.h"

#define LITERALMAX 200

typedef int status;

CNF* loadCNF();     //读取cnf文件
status changetoRes(char* path,char* cnfpath );      //求解结果输出到同名res文件
status saveRes(char* respath,status dpll,CNF* cnf,int time);        //保存res文件
status saveCNF();   //保存cnf文件

#endif // CNFFILE_H_INCLUDED
