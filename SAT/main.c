#include "display.h"
int main()
{
    int op=1;
    srand(time(NULL));
    while(op)
    {
        system("cls");      //清屏
        printf("\n\n");
        printf("      Menu for CNF based on SAT \n");
        printf("-------------------------------------------------\n");
        printf("1. CNF\n");
        printf("2. 二进制数独\n");
        printf("0. 退出\n");
        printf("-------------------------------------------------\n");
        printf("选择你的操作[0-2]：");
        scanf("%d",&op);
        switch(op)
        {
        case 1:
            CNFpage();
            getchar();getchar();
            break;
         case 2:
            SudokuPage();
            getchar();getchar();
            break;
        case 0:
            break;
        default:
            printf("输入错误，请重新输入！\n");
            break;
        }
    }
    printf("欢迎再次使用!\n");
    getchar();getchar();
    return 0;
}
