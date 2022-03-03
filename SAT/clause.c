#include "clause.h"

status createCNF(CNF* cnf,int varnum,int clausenum)
{
    cnf->varnum=varnum;
    cnf->clausenum=clausenum;
    cnf->root=NULL;
    cnf->learn_root=NULL;
    cnf->var=(int*)malloc(sizeof(int)*(varnum+1));
    cnf->vsids=(int* )malloc(sizeof(int)*(varnum*2+1));     //最大分配为为每一个变元的正负都分配空间
    cnf->varfloor=(int* )malloc(sizeof(int)*(varnum+1));    //搜索树的最大层数即为变元的数目
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
    //子句不存在，先要建立
    Clause* newclause=(Clause*)malloc(sizeof(Clause));
    if(!newclause||var_num<=0)
        return ERROR;
    else
    {
        newclause->head=NULL;

        newclause->rmv=NULL;
        newclause->length=0;      //因为是逐个变元添加的，不能再一开始就将句子的长度定义为var_num！
        newclause->isremoved=FALSE;

        for(int i=0;i<var_num;i++)
            addvar(newclause,clause[i]);
       if(cnf->root)
            cnf->root->pre=newclause;
        newclause->next=cnf->root;  //不能把下面的句子放到if语句里面
        newclause->pre=NULL;
        cnf->root=newclause;
        cnf->clausenum++;
       return OK;
    }
}

status createLIndex(CNF* cnf)
{
    //cnf->lindex[1].next指向变元为一的分索引
    if(cnf->clausenum==0||cnf->varnum==0)
        return ERROR;
    if(cnf->lindex!=NULL)       //删除原索引
        deleteallLIdex(cnf);
    else
        cnf->lindex=(LiteralIndex*)malloc(sizeof(LiteralIndex)*(cnf->varnum+1));    //为varnum个变元建立总的索引表
    Clause* clause;
    for(int i=1;i<=cnf->varnum;i++)
        cnf->lindex[i].next=NULL;       //下标对于变元
    for(clause=cnf->root;clause;clause=clause->next)
        addLIndex(cnf,clause);      //为每一个变元建立分索引，每一个结构体里的Cp指向该变元出现的子句的位置
    return OK;
}

//总觉得这里怪怪的
status addLIndex(CNF* cnf,Clause* clause)
{
    if(cnf->lindex==NULL)
        return FALSE;       //cnf索引未建立
    Node* nodep;
    LiteralIndex* lp;
    for(nodep=clause->head;nodep;nodep=nodep->next)     //每次加入都是头插法
    {
        lp=(LiteralIndex*)malloc(sizeof(LiteralIndex));
        lp->Np=nodep;
        lp->Cp=clause;
        lp->next=cnf->lindex[abs(nodep->var)].next;     //分索引指向总索引表上的该变元所在的后一变元
        cnf->lindex[abs(nodep->var)].next=lp;
    }
    return OK;
}

status insertClause(CNF* cnf,Clause* clause)
{
    //子句已存在只插入
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
    //只改变指针的指向但不释放空间
    Clause* pre_clause=clause->pre;
    if(clause->next)    //不是最后一个
        clause->next->pre=pre_clause;
    if(pre_clause)      //不是第一个
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
    //释放子句的文字空间
    Node* nodep;
    for(nodep=clause->head;nodep;nodep=clause->head)
    {
        clause->head=nodep->next;
        free(nodep);
    }
    //记得删除移除变元内存
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
    //释放子句空间
    for(clausep=cnf->root;clausep;clausep=cnf->root)
        deleteClause(cnf,clause);
    //记得释放学习子句的空间
    LearnClause* lclausep;
    for(lclausep=cnf->learn_root;lclausep;lclausep=cnf->learn_root)
    {
        cnf->learn_root=lclausep->next;     //没能释放变元空间
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
    if(pre_lcp==lclause)    //第一条学习子句
        cnf->learn_root=lclause->next;
    else
    {
        for( ;pre_lcp&&pre_lcp->next!=lclause;pre_lcp=pre_lcp->next) //找到待删的学习子句的前驱
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
    //只是移除不做删除
    //只改指针不该变量
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
            if(!nodeq)      //第一个移除的变元
                clause->head=nodep->next;       //将变元移除子句集,注意是head不是next
            else
                nodeq->next=nodep->next;
            nodep->next=clause->rmv;    //将移除的变元放入移除变元集
            clause->rmv=nodep;      //不能直接写clause->rmv=nodep因为无法确认之前是否有其他的已移除的子句
            return nodep;
        }
        nodeq=nodep;
    }
   // if(!flag)
        return NULL;        //没找到该变元
  //  else
  //      return node;
}

status backvar(Clause* clause,Node* node)
{
    //参数为指向变元的指针node及变元所在的字句clause指针
    //修改移除子句
    if(clause->rmv==node)       //第一个变元
        clause->rmv=node->next;
    else
    {
        Node* nodep=NULL;    //前驱指针
        for(nodep=clause->rmv;nodep&&nodep->next!=node;nodep=nodep->next)
            ;
        if(!nodep)
            return ERROR;
        nodep->next=node->next;
    }
    //将恢复的变元放插入子句集
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
            if(!nodeq)  //第一个
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
    //修改子句集中的索引
    for(node=clause->head;node;node=node->next)
    {
        //找到变元所在的索引位置的前驱
        for(lindex=&(cnf->lindex[abs(node->var)]);lindex->next->Cp!=clause;lindex=lindex->next)
            ;
        temp=lindex->next;
        lindex->next=temp->next;                     //不能直接写lindex->next=lindex->next->next！！！
        free(temp);
    }
    //不在子句集中，修改移除集中的索引
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
            //搞不懂啥意思
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
    printf("布尔变元数目:%d\n",cnf->varnum);
    printf("子句数目：%d\n",cnf->clausenum);
    printf("各变元取值:\n");
    for(int i=1;i<=cnf->varnum;i++)
    {
        printf("第%d个：%d ",i,cnf->var[i]);
        if(i%10==0)
            printf("\n");
    }
    printf("决策层数：\n");
    for(int i=1;i<=cnf->varnum;i++)
        printf("第%d个变元位于第%d层\n",i,cnf->varfloor[i]);
    return OK;
}

boolean haveEmptyClause(CNF* cnf)
{
    //判断是否含有空子句
    Clause* clause;
    for(clause=cnf->root;clause;clause=clause->next)
        if(clause->length==0)
            return TRUE;
    return FALSE;
}


boolean haveClause(CNF* cnf)
{
    //判断是否为空
    return (cnf->clausenum)?TRUE:FALSE;
}

boolean evaluateClause(Clause* clause,int var)
{
    //选择一个变元var，并赋值为真，判断子句是否满足
    Node* node;
    int flag=NOTCONTAIN;
    for(node=clause->head;node;node=node->next)
    {
        if(node->var==var)  //存在取值为真的变元，析取范式（子句）表达式为真
            return TRUE;
        else if(node->var==-var)
            flag=NOTSURE;   //存在该真变元的非变元，无法确定表达式的真假
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
