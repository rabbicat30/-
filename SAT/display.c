#include "display.h"

#define DEBUG 0

void CNFpage()
{
    int op=1;
    CNF* cnf=NULL;
    char cnffile[200];      //ע����Ϊһ������ռ2���ֽڣ��������������·�����н϶຺�֣�����֮ǰд100����
    static int flag=FALSE;  //flag������ʾ�Ƿ��Ѿ������ļ�
    int t;      //
    int dpll;
    char res_save_path[200];
    while(op)
    {
        system("cls");
        printf("\n\n");
        printf("      Menu for CNF based on SAT \n");
        printf("-------------------------------------------------\n");
        printf("---------------------CNF-------------------------\n");
        printf("1. ����cnf�ļ�                     2. ���cnf�ļ�\n");
        printf("3. ��ӡcnf�ļ�                     4. ���       \n");
        printf("0. ������һ��\n");
        printf("-------------------------------------------------\n");
        printf("ѡ����Ĳ�����0-3����");
        scanf("%d",&op);
        switch(op)
        {
        case 1:
                printf("������ص��ļ���·����\n");
                scanf("%s",cnffile);
                if(strcmp(cnffile+strlen(cnffile)-4,".cnf")==0)     //ȷ����cnf�ļ�
                {
                    if(!(cnf=loadCNF(cnffile)))
                        printf("�ļ���ʧ�ܣ�\n");
                    else
                        printf("�򿪳ɹ���\n");
                }
            flag=TRUE;
            getchar();getchar();

            break;
        case 2:
            if(flag==FALSE)
                printf("��δ����cnf�ļ���\n");
            else if(!cnf)
                printf("cnf������!\n");
            else
            {

                int type;
                clearCNF(cnf);
                printf("��cnf�ļ��ı�Ԫ��Ϊ��%d\n",cnf->varnum);
                printf("��cnf�ļ����Ӿ���Ϊ��%d\n",cnf->clausenum);
                printf("ѡ�� 1. �Ż����         2.�������\n");
                scanf("%d",&type);
                switch(type)
                {
                case 1:
                    createLIndex(cnf);
                    t=clock();
                    dpll=advancedDPLL(cnf,0);
                    t=clock()-t;
                    break;
                case 2:
                    t=clock();
                    dpll=DPLL(cnf,0);
                    t=clock()-t;
                    break;
                default:
                    break;
                }//end of awitch
                if(dpll==TRUE)
                    {
                        printf("���ɹ���\n");
                        printf("���𰸣�\n");
                        for(int i=1;i<=cnf->varnum;i++)     //������Ϊ0�����ʾ�����Ԫ��cnf�ļ���û�г���
                        {
                                printf("%d ",cnf->var[i]*i);
                                if(i%10==0)
                                    printf("\n");

                        }
                        printf("\n");
                        printf("����res�ļ�\n");
                        changetoRes(res_save_path,cnffile);
                        if(saveRes(res_save_path,dpll,cnf,t))
                            printf("res�ļ�����ɹ���\n");
                        else
                            printf("res�ļ�����ʧ�ܣ�\n");
                    }
                    else
                    {
                        printf("���ʧ�ܣ�\n");
                        printf("����res�ļ�\n");
                        changetoRes(res_save_path,cnffile);
                        if(saveRes(res_save_path,dpll,cnf,t))
                            printf("res�ļ�����ɹ���\n");
                        else
                            printf("res�ļ�����ʧ�ܣ�\n");
                    }

                printf("�ķ�ʱ�䣺%dms\n",t);
            }//end of else
            getchar();getchar();
            break;
        case 3:
            printf("��ӡcnf�ļ�:\n");
            if(!cnf)
                printf("cnf�ļ������ڣ�\n");
            else
                printClause(cnf);
             getchar();getchar();
             break;
        case 4:
            if(!cnf)
                printf("�ļ������ڣ�\n");
            else if(dpll==FALSE)
                printf("������\n");
            else
                check(cnf);
            getchar();getchar();
            break;
        case 0:
            break;
        }//end of switch
    }//end of while(op)
}//end of cnfpage


status SudokuPage()
{
    int op=1;
    while(op)
    {

        printf("\n\n");
        printf("      Menu for SUDOKU \n");
        printf("-------------------------------------------------\n");
        printf("---------------------SUDOKU-------------------------\n");
        printf("1. ���벢�����������                     2. ��ȡ��������������\n");
        printf("3. ����������                           0. �˳�       \n");
        printf("-------------------------------------------------\n");
        printf("ѡ����Ĳ�����0-3����");
        scanf("%d",&op);
        switch(op)
        {
        case 1:
            SaveSudoku();
            getchar();getchar();
            break;
        case 2:
        {

            int t;
            int dpll;
            Sudoku* s=loadSudoku();
            if (!s)
                break;
            for(int i=0;i<6;i++)
            {
                for(int j=0;j<6;j++)
                    printf("%d ",s->sdk[i][j]);
                printf("\n");
            }
            printSudoku(s,TRUE);
            printf("\n");
            CNF* cnf=NULL;
            cnf=transtoCNF(s);
            Clause* clause;Node* node;
            int i=0;
            for(clause=cnf->root;clause;clause=clause->next)
                for(node=clause->head;node;node=node->next)
                {
                    if(abs(node->var)>i)
                        i=abs(node->var);
                }
            printf("cnf�ļ��б�Ԫ�����ֵwei :%d\n",i);
            if(!cnf)
            {
                 printf("ת��ʧ�ܣ�\n");
                 return FALSE;
            }

            createLIndex(cnf);
            t=clock();
            dpll=advancedDPLL(cnf,0);
            t=clock()-t;
            if(dpll==TRUE)
            {
                printf("�����ɹ������\n");
                printf("��ʱ��%dms\n",t);
                Sudoku* ss=transtoSudoku(cnf);
                s->next=ss;
                printSudoku(s,FALSE);
                free(ss);
            }

            else
                printf("���ʧ��\n");
            getchar();getchar();
            break;
        }
        case 3:
        {
            CNF* cnf;
            int t,dpll;
            Sudoku* s;
            int difficult;
            printf("�����Ѷȣ�[1-3]\n");         //�Ѷȱ�ʾ����������Ĵ���
            scanf("%d",&difficult);
            if(!(s=createSudoku(difficult)))
            {
                printf("��������ʧ�ܣ�\n");
                break;
            }
            else
            {
                printSudoku(s,TRUE);
                int op1;
                printf("1. �Լ����          2. ϵͳ���\n");
                scanf("%d",&op1);
                switch(op1)
                {
                case 1:
                    {
                        Sudoku* s1=(Sudoku* )malloc(sizeof(Sudoku));
                        if(!s1)
                        {
                            printf("��������ʧ�ܣ�\n");
                            break;
                        }
                        int state=SolveMySukodu(s,s1);
                        if(state==OK)
                            break;
                    }
                case 2:
                    {
                        cnf=transtoCNF(s);
                        if(!cnf)
                        {
                            printf("cnf�ļ�ת��ʧ�ܣ�\n");
                            return ERROR;
                        }
                        else
                        {
                            printf("�����ɹ�ת��Ϊcnf!\n");
                            printf("��Ԫ��Ϊ:%d\n�Ӿ���Ϊ:%d\n",cnf->varnum,cnf->clausenum);
                            printf("����ʹ���Ż���������Ϊ�����...\n");
                            //createLIndex(cnf);
                            t=clock();
                            dpll=DPLL(cnf,0);
                            t=clock()-t;
                            if(dpll==TRUE)
                            {
                                printf("�����ɹ������\n");
                                printf("��ʱ��%dms\n",t);
                                Sudoku* ss=transtoSudoku(cnf);
                                s->next=ss;
                                printSudoku(s,FALSE);
                                free(ss);
                                while(1)
                                {
                                    int op;
                                    char cnffilename[200];
                                    char resfilename[200];
                                    printf("-----------------------------------\n");
                                    printf("1. ����cnf�ļ������       ");
                                    printf("0.����\n");
                                    printf("���ѡ�񣺡�0-1����\n");
                                    scanf("%d",&op);
                                    if(op==1)
                                    {
                                        printf("�����㱣���·��:\n");
                                        scanf("%s",cnffilename);
                                        if(saveCNF(cnf,cnffilename))
                                        {
                                            printf("cnf����ɹ���\n");
                                            changetoRes(resfilename,cnffilename);
                                            if(saveRes(resfilename,dpll,cnf,t))
                                            {
                                                printf("res�ļ�����ɹ���\n");
                                                break;
                                            }
                                            else
                                                printf("res�ļ�����ʧ�ܣ�\n");
                                        }
                                        else
                                            printf("cnf�ļ�����ʧ�ܣ�\n");
                                    }
                                    else if(op==0)
                                        break;
                                    else
                                    {
                                        printf("�������\n");
                                        break;
                                    }
                                }//end of while
                            }//end of if(dpll==TRUE)
                            else
                                printf("�������ʧ�ܣ�\n");
                        }
                    }
                }



            }
            getchar();getchar();
            break;
        }
        case 0:
            break;
    }
    }
    return OK;
}

