#include "cnfFile.h"
#include<string.h>

CNF* loadCNF(char* filename)
{
    FILE* fp;
    char ch;
    int var_num,clause_num;
    fp=fopen(filename,"r");
    int clause[200];
    if(!fp)
    {
        printf("打开文件失败！\n");
        return NULL;
    }
    CNF* cnf=(CNF*)malloc(sizeof(CNF));
    if(!cnf)
        return NULL;
        //读取注释
    while((ch=fgetc(fp))=='c')
        while((ch=fgetc(fp))!='\n')
            ;
    for(int i=0;i<5;i++)    //包括p cnf五个字符
        ch=fgetc(fp);
    fscanf(fp,"%d",&var_num);
    fscanf(fp,"%d",&clause_num);
    if(!createCNF(cnf,var_num,0))       //这里不能直接将clause_num创建，因为在create 函数中并未为子句创建空间
    {
        fclose(fp);
        return NULL;
    }
    for(int i=0;i<clause_num;i++)
    {
        int j=0;
        do
        {
            fscanf(fp,"%d",&clause[j]);
            j++;
        }while(clause[j-1]!=0);   //结束时前面j自增了，所以是j-1，是0，不是'0'
        if(!addClause(cnf,j-1,clause))
        {
            fclose(fp);
            return NULL;
        }
    }
    fclose(fp);
    return cnf;
}

status changetoRes(char* resfilename,char* cnffilename)
{
    int i;
    for(i=0;i<strlen(cnffilename)-4;i++)    //开始写的-3，发现.无法放入resfilename中
        resfilename[i]=cnffilename[i];
    //此时res文件名为***
    resfilename[i]='\0';
    strncat(resfilename,".res",100);     //添加文件后缀
    printf("res文件：%s\n",resfilename);
    return OK;
}

status saveRes(char* resfilename,status dpll,CNF* cnf,int time)
{
    //s为求解结果，v表示每个变元的取值，t表示时间
    //传入的参数分别表示：待保存的res文件名，cnf文件，求解状态，花费时间
    FILE* fp=fopen(resfilename,"w");
    if(!fp)
    {
        printf("文件打开失败！\n");
        return ERROR;
    }
    fprintf(fp,"s %d\n",dpll);
    fprintf(fp,"v");
    for(int i=1;i<=cnf->varnum;i++)
        fprintf(fp," %d",i*(cnf->var[i]));
    fprintf(fp,"\nt %d",time);
    fclose(fp);
    return OK;
}

status saveCNF(CNF* cnf,char* cnffilename)
{
    FILE* fp=fopen(cnffilename,"w");
    Clause* clause;
    Node* node;
    if(!fp)
    {
        printf("文件打开失败!\n");
        exit(-1);
    }
    fprintf(fp,"p cnf %d %d\n",cnf->varnum,cnf->clausenum);
    for(clause=cnf->root;clause;clause=clause->next)
    {
        for(node=clause->head;node;node=node->next)
            fprintf(fp,"%d ",node->var);
        fprintf(fp,"0\n");
    }
    fclose(fp);
    return OK;
}
