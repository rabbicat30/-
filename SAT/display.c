#include "display.h"

#define DEBUG 0

void CNFpage()
{
    int op=1;
    CNF* cnf=NULL;
    char cnffile[200];      //注意因为一个汉子占2个字节，而保存的用例的路径含有较多汉字，导致之前写100不够
    static int flag=FALSE;  //flag用来表示是否已经载入文件
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
        printf("1. 载入cnf文件                     2. 求解cnf文件\n");
        printf("3. 打印cnf文件                     4. 检查       \n");
        printf("0. 返回上一级\n");
        printf("-------------------------------------------------\n");
        printf("选择你的操作【0-3】：");
        scanf("%d",&op);
        switch(op)
        {
        case 1:
                printf("输入加载的文件的路径：\n");
                scanf("%s",cnffile);
                if(strcmp(cnffile+strlen(cnffile)-4,".cnf")==0)     //确保是cnf文件
                {
                    if(!(cnf=loadCNF(cnffile)))
                        printf("文件打开失败！\n");
                    else
                        printf("打开成功！\n");
                }
            flag=TRUE;
            getchar();getchar();

            break;
        case 2:
            if(flag==FALSE)
                printf("尚未加载cnf文件！\n");
            else if(!cnf)
                printf("cnf不存在!\n");
            else
            {

                int type;
                clearCNF(cnf);
                printf("该cnf文件的变元数为：%d\n",cnf->varnum);
                printf("该cnf文件的子句数为：%d\n",cnf->clausenum);
                printf("选择： 1. 优化求解         2.正常求解\n");
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
                        printf("求解成功！\n");
                        printf("求解答案：\n");
                        for(int i=1;i<=cnf->varnum;i++)     //如果输出为0，则表示这个变元在cnf文件中没有出现
                        {
                                printf("%d ",cnf->var[i]*i);
                                if(i%10==0)
                                    printf("\n");

                        }
                        printf("\n");
                        printf("保存res文件\n");
                        changetoRes(res_save_path,cnffile);
                        if(saveRes(res_save_path,dpll,cnf,t))
                            printf("res文件保存成功！\n");
                        else
                            printf("res文件保存失败！\n");
                    }
                    else
                    {
                        printf("求解失败！\n");
                        printf("保存res文件\n");
                        changetoRes(res_save_path,cnffile);
                        if(saveRes(res_save_path,dpll,cnf,t))
                            printf("res文件保存成功！\n");
                        else
                            printf("res文件保存失败！\n");
                    }

                printf("耗费时间：%dms\n",t);
            }//end of else
            getchar();getchar();
            break;
        case 3:
            printf("打印cnf文件:\n");
            if(!cnf)
                printf("cnf文件不存在！\n");
            else
                printClause(cnf);
             getchar();getchar();
             break;
        case 4:
            if(!cnf)
                printf("文件不存在！\n");
            else if(dpll==FALSE)
                printf("求解错误！\n");
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
        printf("1. 输入并保存你的数独                     2. 读取输入的数独并求解\n");
        printf("3. 求解随机数独                           0. 退出       \n");
        printf("-------------------------------------------------\n");
        printf("选择你的操作【0-3】：");
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
            printf("cnf文件中变元的最大值wei :%d\n",i);
            if(!cnf)
            {
                 printf("转换失败！\n");
                 return FALSE;
            }

            createLIndex(cnf);
            t=clock();
            dpll=advancedDPLL(cnf,0);
            t=clock()-t;
            if(dpll==TRUE)
            {
                printf("数独成功解出！\n");
                printf("用时：%dms\n",t);
                Sudoku* ss=transtoSudoku(cnf);
                s->next=ss;
                printSudoku(s,FALSE);
                free(ss);
            }

            else
                printf("求解失败\n");
            getchar();getchar();
            break;
        }
        case 3:
        {
            CNF* cnf;
            int t,dpll;
            Sudoku* s;
            int difficult;
            printf("输入难度：[1-3]\n");         //难度表示计算允许多解的次数
            scanf("%d",&difficult);
            if(!(s=createSudoku(difficult)))
            {
                printf("数独生成失败！\n");
                break;
            }
            else
            {
                printSudoku(s,TRUE);
                int op1;
                printf("1. 自己求解          2. 系统求解\n");
                scanf("%d",&op1);
                switch(op1)
                {
                case 1:
                    {
                        Sudoku* s1=(Sudoku* )malloc(sizeof(Sudoku));
                        if(!s1)
                        {
                            printf("创建数独失败！\n");
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
                            printf("cnf文件转换失败！\n");
                            return ERROR;
                        }
                        else
                        {
                            printf("数独成功转换为cnf!\n");
                            printf("变元数为:%d\n子句数为:%d\n",cnf->varnum,cnf->clausenum);
                            printf("正在使用优化后的求解器为您求解...\n");
                            //createLIndex(cnf);
                            t=clock();
                            dpll=DPLL(cnf,0);
                            t=clock()-t;
                            if(dpll==TRUE)
                            {
                                printf("数独成功解出！\n");
                                printf("用时：%dms\n",t);
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
                                    printf("1. 保存cnf文件及结果       ");
                                    printf("0.返回\n");
                                    printf("你的选择：【0-1】：\n");
                                    scanf("%d",&op);
                                    if(op==1)
                                    {
                                        printf("输入你保存的路径:\n");
                                        scanf("%s",cnffilename);
                                        if(saveCNF(cnf,cnffilename))
                                        {
                                            printf("cnf保存成功！\n");
                                            changetoRes(resfilename,cnffilename);
                                            if(saveRes(resfilename,dpll,cnf,t))
                                            {
                                                printf("res文件保存成功！\n");
                                                break;
                                            }
                                            else
                                                printf("res文件保存失败！\n");
                                        }
                                        else
                                            printf("cnf文件保存失败！\n");
                                    }
                                    else if(op==0)
                                        break;
                                    else
                                    {
                                        printf("输入错误！\n");
                                        break;
                                    }
                                }//end of while
                            }//end of if(dpll==TRUE)
                            else
                                printf("数独求解失败！\n");
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

