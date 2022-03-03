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
        printf("���ļ�ʧ�ܣ�\n");
        return NULL;
    }
    CNF* cnf=(CNF*)malloc(sizeof(CNF));
    if(!cnf)
        return NULL;
        //��ȡע��
    while((ch=fgetc(fp))=='c')
        while((ch=fgetc(fp))!='\n')
            ;
    for(int i=0;i<5;i++)    //����p cnf����ַ�
        ch=fgetc(fp);
    fscanf(fp,"%d",&var_num);
    fscanf(fp,"%d",&clause_num);
    if(!createCNF(cnf,var_num,0))       //���ﲻ��ֱ�ӽ�clause_num��������Ϊ��create �����в�δΪ�Ӿ䴴���ռ�
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
        }while(clause[j-1]!=0);   //����ʱǰ��j�����ˣ�������j-1����0������'0'
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
    for(i=0;i<strlen(cnffilename)-4;i++)    //��ʼд��-3������.�޷�����resfilename��
        resfilename[i]=cnffilename[i];
    //��ʱres�ļ���Ϊ***
    resfilename[i]='\0';
    strncat(resfilename,".res",100);     //����ļ���׺
    printf("res�ļ���%s\n",resfilename);
    return OK;
}

status saveRes(char* resfilename,status dpll,CNF* cnf,int time)
{
    //sΪ�������v��ʾÿ����Ԫ��ȡֵ��t��ʾʱ��
    //����Ĳ����ֱ��ʾ���������res�ļ�����cnf�ļ������״̬������ʱ��
    FILE* fp=fopen(resfilename,"w");
    if(!fp)
    {
        printf("�ļ���ʧ�ܣ�\n");
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
        printf("�ļ���ʧ��!\n");
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
