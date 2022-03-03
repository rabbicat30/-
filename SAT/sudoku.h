#ifndef SUDOKU_H_INCLUDED
#define SUDOKU_H_INCLUDED

#include "clause.h"

typedef struct Sudoku{
    int sdk[6][6];
    struct Sudoku* next;        //若干s数独有解，s.next指向其解
}Sudoku;

Sudoku* createSudoku();
CNF* transtoCNF();    //转换为cnf文件
status solveSudoku();   //求解数独
status DFSSSudoku();
status printSudoku();   //打印数独
Sudoku* transtoSudoku();     //cnf转换为数独
status randSudoku();        //随机生成数独
status rule2_select4unit();     //约束2，找任意四个单元
//添加到cnf文件中时子句转换的约束
status rule1();
status rule2();
status rule3();
//生成数独时的约束
status checkRule1();
status checkRule2();
status checkRule3();
status checkSudoku();      //生成数独填数是检查是否满足约束
status SaveSudoku();       //保存输入的数独到文件中
Sudoku* loadSudoku();        //读取文件中的数独文件
status SolveMySukodu();      //验证输入的数独是否满足数独约束规则
status istheSameSudoku();    //检查输入的数独是否是待求解的数独
#endif // SUDOKU_H_INCLUDED
