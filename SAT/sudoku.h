#ifndef SUDOKU_H_INCLUDED
#define SUDOKU_H_INCLUDED

#include "clause.h"

typedef struct Sudoku{
    int sdk[6][6];
    struct Sudoku* next;        //����s�����н⣬s.nextָ�����
}Sudoku;

Sudoku* createSudoku();
CNF* transtoCNF();    //ת��Ϊcnf�ļ�
status solveSudoku();   //�������
status DFSSSudoku();
status printSudoku();   //��ӡ����
Sudoku* transtoSudoku();     //cnfת��Ϊ����
status randSudoku();        //�����������
status rule2_select4unit();     //Լ��2���������ĸ���Ԫ
//��ӵ�cnf�ļ���ʱ�Ӿ�ת����Լ��
status rule1();
status rule2();
status rule3();
//��������ʱ��Լ��
status checkRule1();
status checkRule2();
status checkRule3();
status checkSudoku();      //�������������Ǽ���Ƿ�����Լ��
status SaveSudoku();       //����������������ļ���
Sudoku* loadSudoku();        //��ȡ�ļ��е������ļ�
status SolveMySukodu();      //��֤����������Ƿ���������Լ������
status istheSameSudoku();    //�������������Ƿ��Ǵ���������
#endif // SUDOKU_H_INCLUDED
