#include "display.h"
int main()
{
    int op=1;
    srand(time(NULL));
    while(op)
    {
        system("cls");      //����
        printf("\n\n");
        printf("      Menu for CNF based on SAT \n");
        printf("-------------------------------------------------\n");
        printf("1. CNF\n");
        printf("2. ����������\n");
        printf("0. �˳�\n");
        printf("-------------------------------------------------\n");
        printf("ѡ����Ĳ���[0-2]��");
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
            printf("����������������룡\n");
            break;
        }
    }
    printf("��ӭ�ٴ�ʹ��!\n");
    getchar();getchar();
    return 0;
}
