#include "clause.h"

status createCNF(CNF* cnf,int varnum,int clausenum)
{
    cnf->varnum=varnum;
    cnf->clausenum=clausenum;
    cnf->root=NULL;
    cnf->learn_root=NULL;
    cnf->var=(int*)malloc(sizeof(int)*(varnum+1));
    cnf->vsids=(int* )malloc(sizeof(int)*(varnum*2+1));     //������ΪΪÿһ����Ԫ������������ռ�
    cnf->varfloor=(int* )malloc(sizeof(int)*(varnum+1));    //����������������Ϊ��Ԫ����Ŀ
    cnf->lindex=NULL;
    if(cnf->var&&cnf->varfloor)
    {
        clearCNF(cnf);
        return OK;
    }
    else
        return ERROR;
}

status destroyCNF(CNF* cnf)
{
    if(!cnf)
        return ERROR;
    else
    {
        deleteallClause(cnf);
        deleteallLIdex(cnf);
        free(cnf->var);
        free(cnf->learn_root);
        free(cnf->root);
        free(cnf->varfloor);
        free(cnf->vsids);
        return OK;
    }
}

status clearCNF(CNF* cnf)
{
    LearnClause* lclause;
    for(lclause=cnf->learn_root;lclause;lclause=cnf->learn_root)
    {
        deleteClauseLIndex(cnf,lclause->clause);
        deleteLearnClause(cnf,lclause);
    }
    for(int i=0;i<=cnf->varnum;i++)
    {
        cnf->var[i]=0;
        cnf->varfloor[i]=-1;
        cnf->vsids[i]=cnf->vsids[i+cnf->varnum]=0;
    }
    return OK;
}

status addClause(CNF* cnf,int var_num,int* clause)
{
    //�Ӿ䲻���ڣ���Ҫ����
    Clause* newclause=(Clause*)malloc(sizeof(Clause));
    if(!newclause||var_num<=0)
        return ERROR;
    else
    {
        newclause->head=NULL;

        newclause->rmv=NULL;
        newclause->length=0;      //��Ϊ�������Ԫ��ӵģ�������һ��ʼ�ͽ����ӵĳ��ȶ���Ϊvar_num��
        newclause->isremoved=FALSE;

        for(int i=0;i<var_num;i++)
            addvar(newclause,clause[i]);
       if(cnf->root)
            cnf->root->pre=newclause;
        newclause->next=cnf->root;  //���ܰ�����ľ��ӷŵ�if�������
        newclause->pre=NULL;
        cnf->root=newclause;
        cnf->clausenum++;
       return OK;
    }
}

status createLIndex(CNF* cnf)
{
    //cnf->lindex[1].nextָ���ԪΪһ�ķ�����
    if(cnf->clausenum==0||cnf->varnum==0)
        return ERROR;
    if(cnf->lindex!=NULL)       //ɾ��ԭ����
        deleteallLIdex(cnf);
    else
        cnf->lindex=(LiteralIndex*)malloc(sizeof(LiteralIndex)*(cnf->varnum+1));    //Ϊvarnum����Ԫ�����ܵ�������
    Clause* clause;
    for(int i=1;i<=cnf->varnum;i++)
        cnf->lindex[i].next=NULL;       //�±���ڱ�Ԫ
    for(clause=cnf->root;clause;clause=clause->next)
        addLIndex(cnf,clause);      //Ϊÿһ����Ԫ������������ÿһ���ṹ�����Cpָ��ñ�Ԫ���ֵ��Ӿ��λ��
    return OK;
}

//�ܾ�������ֵֹ�
status addLIndex(CNF* cnf,Clause* clause)
{
    if(cnf->lindex==NULL)
        return FALSE;       //cnf����δ����
    Node* nodep;
    LiteralIndex* lp;
    for(nodep=clause->head;nodep;nodep=nodep->next)     //ÿ�μ��붼��ͷ�巨
    {
        lp=(LiteralIndex*)malloc(sizeof(LiteralIndex));
        lp->Np=nodep;
        lp->Cp=clause;
        lp->next=cnf->lindex[abs(nodep->var)].next;     //������ָ�����������ϵĸñ�Ԫ���ڵĺ�һ��Ԫ
        cnf->lindex[abs(nodep->var)].next=lp;
    }
    return OK;
}

status insertClause(CNF* cnf,Clause* clause)
{
    //�Ӿ��Ѵ���ֻ����
    if(cnf->root)
        cnf->root->pre=clause;
    clause->next=cnf->root;
    clause->pre=NULL;
    cnf->root=clause;
    clause->isremoved=FALSE;
    cnf->clausenum++;
    return OK;
}

Clause* removeClause(CNF* cnf,Clause* clause)
{
    //ֻ�ı�ָ���ָ�򵫲��ͷſռ�
    Clause* pre_clause=clause->pre;
    if(clause->next)    //�������һ��
        clause->next->pre=pre_clause;
    if(pre_clause)      //���ǵ�һ��
        pre_clause->next=clause->next;
    else
        cnf->root=clause->next;
    clause->isremoved=TRUE;
    cnf->clausenum--;
    return clause;
}

status deleteClause(CNF* cnf,Clause* clause)
{
    Clause* pre_clause=clause->pre;
    if(clause->next)
        clause->next->pre=pre_clause;
    if(pre_clause)
        pre_clause->next=clause->next;
    else
        cnf->root=clause->next;
    //�ͷ��Ӿ�����ֿռ�
    Node* nodep;
    for(nodep=clause->head;nodep;nodep=clause->head)
    {
        clause->head=nodep->next;
        free(nodep);
    }
    //�ǵ�ɾ���Ƴ���Ԫ�ڴ�
    for(nodep=clause->rmv;nodep;nodep=clause->rmv)
    {
        clause->rmv=nodep->next;
        free(nodep);
    }
    free(clause);
    cnf->clausenum--;
    return OK;
}

status deleteallClause(CNF* cnf,Clause* clause)
{
    Clause* clausep;
    //�ͷ��Ӿ�ռ�
    for(clausep=cnf->root;clausep;clausep=cnf->root)
        deleteClause(cnf,clause);
    //�ǵ��ͷ�ѧϰ�Ӿ�Ŀռ�
    LearnClause* lclausep;
    for(lclausep=cnf->learn_root;lclausep;lclausep=cnf->learn_root)
    {
        cnf->learn_root=lclausep->next;     //û���ͷű�Ԫ�ռ�
        free(lclausep);
    }
    return OK;
}

status addvar(Clause* clause,int var)
{
    Node* node=(Node* )malloc(sizeof(Node));
    if(!node)
        return ERROR;
    node->var=var;
    node->next=clause->head;
    clause->head=node;
    clause->length++;
    return OK;
}

status deleteLearnClause(CNF* cnf,LearnClause* lclause)
{
    LearnClause* pre_lcp=cnf->learn_root;
    if(pre_lcp==lclause)    //��һ��ѧϰ�Ӿ�
        cnf->learn_root=lclause->next;
    else
    {
        for( ;pre_lcp&&pre_lcp->next!=lclause;pre_lcp=pre_lcp->next) //�ҵ���ɾ��ѧϰ�Ӿ��ǰ��
            ;
    }
    if(!pre_lcp)
            return ERROR;
    pre_lcp->next=lclause->next;
    deleteClause(cnf,lclause->clause);
    free(lclause);
    return OK;

}

Node* removevar(Clause* clause,int var)
{
    //ֻ���Ƴ�����ɾ��
    //ֻ��ָ�벻�ñ���
    Node* nodep=clause->head;
    Node* nodeq=NULL;
   // Node* node=NULL;
    //int flag=0;
    for( ;nodep;nodep=nodep->next)
    {
        if(nodep->var==var)
        {
            //flag=1;
            //node=nodep;
            clause->length--;
            if(!nodeq)      //��һ���Ƴ��ı�Ԫ
                clause->head=nodep->next;       //����Ԫ�Ƴ��Ӿ伯,ע����head����next
            else
                nodeq->next=nodep->next;
            nodep->next=clause->rmv;    //���Ƴ��ı�Ԫ�����Ƴ���Ԫ��
            clause->rmv=nodep;      //����ֱ��дclause->rmv=nodep��Ϊ�޷�ȷ��֮ǰ�Ƿ������������Ƴ����Ӿ�
            return nodep;
        }
        nodeq=nodep;
    }
   // if(!flag)
        return NULL;        //û�ҵ��ñ�Ԫ
  //  else
  //      return node;
}

status backvar(Clause* clause,Node* node)
{
    //����Ϊָ���Ԫ��ָ��node����Ԫ���ڵ��־�clauseָ��
    //�޸��Ƴ��Ӿ�
    if(clause->rmv==node)       //��һ����Ԫ
        clause->rmv=node->next;
    else
    {
        Node* nodep=NULL;    //ǰ��ָ��
        for(nodep=clause->rmv;nodep&&nodep->next!=node;nodep=nodep->next)
            ;
        if(!nodep)
            return ERROR;
        nodep->next=node->next;
    }
    //���ָ��ı�Ԫ�Ų����Ӿ伯
    node->next=clause->head;
    clause->head=node;
    clause->length++;
    return OK;
}

status deletevar(Clause* clause,int var)
{
    Node* nodep=NULL;
    Node* nodeq=NULL;
    for(nodep=clause->head;nodep;nodep=nodep->next)
    {
        if(nodep->var==var)
        {
            clause->length--;
            if(!nodeq)  //��һ��
                clause->head=nodep->next;
            else
                nodeq->next=nodep->next;
            free(nodep);
            return OK;
        }
      nodeq=nodep;
    }
    return ERROR;
}

status deleteClauseLIndex(CNF* cnf,Clause* clause)
{
    LiteralIndex* lindex;
    Node* node;
    LiteralIndex* temp;
    //�޸��Ӿ伯�е�����
    for(node=clause->head;node;node=node->next)
    {
        //�ҵ���Ԫ���ڵ�����λ�õ�ǰ��
        for(lindex=&(cnf->lindex[abs(node->var)]);lindex->next->Cp!=clause;lindex=lindex->next)
            ;
        temp=lindex->next;
        lindex->next=temp->next;                     //����ֱ��дlindex->next=lindex->next->next������
        free(temp);
    }
    //�����Ӿ伯�У��޸��Ƴ����е�����
    for(node=clause->rmv;node;node=node->next)
    {
        for(lindex=&(cnf->lindex[abs(node->var)]);lindex->next->Cp!=clause;lindex=lindex->next)
            ;
        temp=lindex->next;
        lindex->next=temp->next;
        free(temp);
    }
    return OK;
}

status deleteallLIdex(CNF* cnf,Clause* clause)
{
    if(cnf->lindex==NULL)
        return OK;
    for(int i=1;i<=cnf->varnum;i++)
    {
        while(cnf->lindex[i].next)
        {
            //�㲻��ɶ��˼
            LiteralIndex* lindex=cnf->lindex[i].next;
            cnf->lindex[i].next=lindex->next;
            free(lindex);
        }
    }
    return OK;
}

status arrayAssign(CNF* cnf,int var,int floor)
{
    cnf->var[abs(var)]=EVALUE(var);
    cnf->varfloor[abs(var)]=floor;
    return OK;
}


status printClause(CNF* cnf)
{
    printf("������Ԫ��Ŀ:%d\n",cnf->varnum);
    printf("�Ӿ���Ŀ��%d\n",cnf->clausenum);
    printf("����Ԫȡֵ:\n");
    for(int i=1;i<=cnf->varnum;i++)
    {
        printf("��%d����%d ",i,cnf->var[i]);
        if(i%10==0)
            printf("\n");
    }
    printf("���߲�����\n");
    for(int i=1;i<=cnf->varnum;i++)
        printf("��%d����Ԫλ�ڵ�%d��\n",i,cnf->varfloor[i]);
    return OK;
}

boolean haveEmptyClause(CNF* cnf)
{
    //�ж��Ƿ��п��Ӿ�
    Clause* clause;
    for(clause=cnf->root;clause;clause=clause->next)
        if(clause->length==0)
            return TRUE;
    return FALSE;
}


boolean haveClause(CNF* cnf)
{
    //�ж��Ƿ�Ϊ��
    return (cnf->clausenum)?TRUE:FALSE;
}

boolean evaluateClause(Clause* clause,int var)
{
    //ѡ��һ����Ԫvar������ֵΪ�棬�ж��Ӿ��Ƿ�����
    Node* node;
    int flag=NOTCONTAIN;
    for(node=clause->head;node;node=node->next)
    {
        if(node->var==var)  //����ȡֵΪ��ı�Ԫ����ȡ��ʽ���Ӿ䣩���ʽΪ��
            return TRUE;
        else if(node->var==-var)
            flag=NOTSURE;   //���ڸ����Ԫ�ķǱ�Ԫ���޷�ȷ�����ʽ�����
    }
    return flag;
}

boolean isClauseUnit(Clause* clause)
{
    return (clause->length==1)?TRUE:FALSE;
}

Clause* LocateUnitClause(CNF* cnf)
{
    Clause* clause;
    for(clause=cnf->root;clause;clause=clause->next)
        if(clause->length==1)
            return clause;
    return NULL;
}

boolean isClauseEmpty(Clause* clause)
{
    return (clause->length)?FALSE:TRUE;
}

status printcnf(CNF* cnf)
{
   printf("\n");
    for(Clause* clause=cnf->root;clause;clause=clause->next)
    {
        for(Node* node=clause->head;node;node=node->next)
            printf("%d ",node->var);
        printf("0\n");
    }
    printf("\n");
    return OK;

}
