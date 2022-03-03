#include "DPLL.h"
#include <time.h>

#define DEBUG2 0

#define VSIDS 2
#define VSIDSCOUNT 100
#define LEARNLENGTH_MAX 20      //限制学习子句的数目
#define OUTTIME 240000

int beginTime;
status reset=FALSE; //重启，即清除现在所有变量的赋值状态，重新选择一组决策变量进行赋值然后进行正常的搜索过程
int MAXC = 10;

enum strategy {mom, vsids};

status saveChange(ChangeStack* head,int tag,Clause* clause,Node* node)
{
    //head为头指针
    if(!head)
        return ERROR;
    ChangeStack* newchange=(ChangeStack*)malloc(sizeof(ChangeStack));
    if(!newchange)
        return ERROR;
    newchange->tag=tag;
    switch(tag)
    {
    case CLAUSE:
        newchange->Cp=clause;
        newchange->Np=NULL;
        break;
    case LITERAL:       //变元改变意味着子句也发生改变
        newchange->Np=node;
        newchange->Cp=clause;
        break;
    case SPLIT:     //作为单子句
        newchange->Cp=clause;
        break;
    default:
        break;
    }
    newchange->next=head->next;
    head->next=newchange;
    return OK;
}

status SimplifySingleClause(CNF* cnf,int var,ChangeStack* head)
{
    Node* node;
    Clause* clausep;
    Clause* clause=cnf->root;
    while(clause)
    {
        //如果该字句是可满足的，则删除该字句，若无法确定，则删除这个文字
        switch(evaluateClause(clause,var))
        {
        case TRUE:
            removeClause(cnf,clause);
            if(head)        //保存修改！
                saveChange(head,CLAUSE,clause,NULL);
            clause=clause->next;
            break;
        case NOTSURE:
            node=removevar(clause,-var);        //-var!!!
            if(head)
                saveChange(head,LITERAL,clause,node);
            //若移除变元后子句变为单子句或者空子句，为了操作方便，将其置于顶部，同时返回空子句
            if(clause->length<=1)
            {
                clausep=clause->next;
                removeClause(cnf,clause);
                insertClause(cnf,clause);
                if(clause->length==0)
                    return ERROR;      //化简之后存在空子句，返回失败
               // else
                    clause=clausep;
            }
            else
                clause=clause->next;
            break;
        default:
            clause=clause->next;
            break;
        }
    }
    return OK;
}

status advancedSimplifySingleClause(CNF* cnf,Clause* clause,ChangeStack* head)
{
    int var=clause->head->var;
    int index=abs(var);
    LiteralIndex* lindex;       //指向当前变元所在的索引
    Clause* clausep;
    Node* node;
    removeClause(cnf,clause);   //移除插入的单子句
    saveChange(head,CLAUSE,clause,NULL);
    for(lindex=cnf->lindex[index].next;lindex;lindex=lindex->next)  //通过索引找到含有变元var的子句
    {
        clausep=lindex->Cp;     //clausep初始化为指向含有变元var的第一个子句（创建索引时的第一个）
        node=lindex->Np;
        if(clausep->isremoved==FALSE)
        {
            if(node->var==var)
            {
                removeClause(cnf,clausep);  //删除包含L的子句，
                if(head)
                    saveChange(head,CLAUSE,clausep,NULL);   //保存修改
            }
            else if(node->var==-var)        //删除子句中含有非L的文字
            {
                node=removevar(clausep,-var);
                if(head)
                    saveChange(head,LITERAL,clausep,node);

            //将单子句和空子句置顶，节省搜索时间
            if(clausep->length<=1)
            {
                removeClause(cnf,clausep);
                insertClause(cnf,clausep);
                if(clausep->length==0)
                    return ERROR;
            }//end of if
           }//end of else if
        }//end of if
    }//end of for
    return OK;
}

status backAssign(CNF* cnf,int floor)
{
    for(int i=1;i<=cnf->varnum;i++)
    {
        if(cnf->varfloor[i]==floor)     //找到该层的变元
        {
            //回复数据
            cnf->var[i]=NOTSURE;
            cnf->varfloor[i]=-1;
        }
    }
    return OK;
}

status MOM(CNF* cnf)
{
    //最短子句出现频率最大优先法
    //子句长度越大越容易满足，因此从较短子句中挑选出较大频率的文字使其优先满足
    //算法的重点是求解出和文字l相对应的J指，最终选J值最大的文字l赋值为真
    //ni记为包含文字l的字句Ci的字句的长度，则J值即为2的负ni次方求和,长度越长,J值越小
    double* J=(double*)malloc(sizeof(double)*(cnf->varnum+1));
    for(int i=1;i<=cnf->varnum;i++)     //初始化为0
        J[i]=0;
    Clause* clause;
    Node* node;
    double max=-1;
    int index;
   //printcnf(cnf);
    for(clause=cnf->root;clause;clause=clause->next)
        for(node=clause->head;node;node=node->next)
            J[abs(node->var)] +=1.0/(1<<(clause->length));
    for(int i=1;i<=cnf->varnum;i++)
    {
        if(max<J[i]&&cnf->var[i]==NOTSURE)
        //寻找最大的J值对应的变元(只确定变元的绝对值，不确定是真或者非）
        {
            max=J[i];
            index=i;
        }
    }
    //确定取真或者取非,看子句集中哪一种取得多,频率最大
   // printf("index为：%d\n",index);
    int positive=0;
    int negative=0;
    for(clause=cnf->root;clause;clause=clause->next)
        for(node=clause->head;node;node=node->next)
        {
            if(node->var>0)
                positive++;
            else
                negative++;
        }
    free(J);
    return (positive>negative)?index:(-index);
}

status VSIDSS(CNF* cnf)
{
    // 为每一个变元的正负文字各设一个计数器，初始值设为在子句集种的出现次数
    //根据未赋值变量中的文字计数器的值的排序，从中选择一个是计数器值最大的一个文字作为决策变量
    int max=0;
    int index;
    static int count=0;        //记录决策次数
    for(int i=1;i<=cnf->varnum;i++)
        if(max<=cnf->vsids[i]+cnf->vsids[i+cnf->varnum])        //计数该文字出现次数（正和非）
            if(cnf->var[i]==NOTSURE)        //找到最大
            {
                max=cnf->vsids[i]+cnf->vsids[i+cnf->varnum];
                index=i;
            }
    count++;
    if(count>=VSIDSCOUNT)
    {
        //为了防止一些变量长时间得不到赋值，经过一定的决策次数之后，每一个文字的计数器的值都会除以一个常数
        count=0;
        for(int i=1;i<=cnf->varnum*2;i++)
            cnf->vsids[i] /=2;
    }
    return (cnf->vsids[index]>cnf->vsids[index+cnf->varnum])?index:(-index);

}

status combineStrategy(CNF* cnf,enum strategy s)
{
    int i=0;
    switch(s)
    {
    case mom:
        i=MOM(cnf);
        break;
    case vsids:
        i=VSIDSS(cnf);
        break;
    }
    return i;
}

status DPLL(CNF* cnf,int f)
{
    //f为决策层，采用递归的思想，从f=0开始往下走
    Clause* clause;
    ChangeStack st;
    st.next=NULL;
    int v;
    if(f==0)        //第一层，开始算法
        beginTime=clock();
    while((clause=LocateUnitClause(cnf))!=NULL)     //存在单子句
    {
        arrayAssign(cnf,clause->head->var,f);
        SimplifySingleClause(cnf,clause->head->var,&st);      //利用单子句规则化简子句集S
        if(haveEmptyClause(cnf)==TRUE)      //如果S存在空子句
        {
            backChange(cnf,&st,-1,NULL);
            backAssign(cnf,f);
            if(clock()-beginTime>=OUTTIME)
            {
                printf("超时！\n");
                return NOTSURE;
            }
            return FALSE;
        }
        else if(haveClause(cnf)==FALSE)     //S为空，满足
        {
            backChange(cnf,&st,-1,NULL);
            return TRUE;
        }
    }//end of while
    //选择策略选择变元v
    v=combineStrategy(cnf,mom);
    arrayAssign(cnf,v,f);
    addClause(cnf,1,&v); //变元v单独作为子句加入到子句集S中,第二个参数为变元数目不是变元的值！！！
    saveChange(&st,SPLIT,cnf->root,NULL);
    int ans=DPLL(cnf,f+1);
    if(ans==TRUE)
    {
       backChange(cnf,&st,-1,NULL);
        return TRUE;
    }
    if(ans==NOTSURE)
    {
        backChange(cnf,&st,-1,NULL);
        backAssign(cnf,f);
        return NOTSURE;
    }
    backChange(cnf,&st,1,NULL);
    v=-v;       //取非
    arrayAssign(cnf,v,f);
    addClause(cnf,1,&v);        //变元v单独作为子句加入到cnf中
    saveChange(&st,SPLIT,cnf->root,NULL);
    ans=DPLL(cnf,f+1);
    backChange(cnf,&st,-1,NULL);
    if(ans==FALSE)
        backAssign(cnf,f);
    return ans;
}

status advancedDPLL(CNF* cnf,int f)
{
    //优化后的dpll算法，即cdll算法
    int back=f;//回溯的层数
    Clause* clause;
    ChangeStack st;
    st.floor=f;
    st.next=NULL;
    int ans;
    int count=0;
    //VSIDS策略
    if(f==0)
    {
        beginTime=clock();
        Node* node;
        for(clause=cnf->root;clause;clause=clause->next)
            for(node=clause->head;node;node=node->next)
            {
                if(node->var>0)
                    cnf->vsids[node->var]++;    //统计正文字
                else
                    cnf->vsids[cnf->varnum-node->var]++;        //负文字
            }
    }
        //在求解是，由于最初的变量决策顺序可能不是最优的，这就会导致在搜索陷入某些子句空间而白白消耗时间
        //此时采用重新启动机制，即清楚目前所有变量的赋值状态，重新选择一组决策变量进行赋值
        do
        {
            count++;
            if(reset==TRUE)
            {
                //重启动
                reset=FALSE;
                LearnClause* lclause=cnf->learn_root;
                LearnClause* lclausep=NULL;
                while(lclause)
                {
                    //子句删除机制，删除一些很长的学习子句
                    if(lclause->clause->rmv==NULL&&lclause->clause->length>LEARNLENGTH_MAX)
                    {
                        //删除那些没有移除变元也就是没有赋值过的变元，并且长度很长的学习子句
                        lclausep=lclause->next;
                        deleteClauseLIndex(cnf,lclause->clause);
                        deleteLearnClause(cnf,lclause);
                        lclause=lclausep;
                    }
                    else
                        lclause=lclause->next;
                }
            }
            while((clause=LocateUnitClause(cnf))!=NULL) //单子句规则简化
            {
                arrayAssign(cnf,clause->head->var,f);
                advancedSimplifySingleClause(cnf,clause,&st);
                if(haveEmptyClause(cnf)==TRUE)
                {
                    if(clock()-beginTime>=OUTTIME)
                    {
                        printf("超时！\n");
                        return -1-f;
                    }
                    int* a=(int* )malloc(sizeof(int)*(cnf->varnum+3));  //包含总数，变元，唯一蕴含点
                    backChange(cnf,&st,-1,a);   //还原，第一次回复应该是删除后的子句变空，产生冲突
                    //若发生冲突，添加学习子句并进行非时序回溯
                    back=a[0];
                    free(a);
                    backLearnClause(cnf,f);
                    //赋值回溯
                    backAssign(cnf,f);
                    if(count>MAXC)
                    {
                        MAXC++;
                        //重启动
                        reset=TRUE;
                        //学习子句回溯
                        return -f;
                    }
                    return back-f;
                }
                else if(haveClause(cnf)==FALSE)
                {
                    backChange(cnf,&st,-1,NULL);
                    backLearnClause(cnf,f);
                    return TRUE;
                }
            }//end of while
            int v=combineStrategy(cnf,mom);
            addClause(cnf,1,&v);
            saveChange(&st,SPLIT,cnf->root,NULL);
            ans=advancedDPLL(cnf,f+1);
            backChange(cnf,&st,1,NULL);
        }while(ans==FALSE);
        if(ans==TRUE)
        {
            backChange(cnf,&st,-1,NULL);
            backLearnClause(cnf,f);
            return TRUE;
        }
        //由返回值决定回溯层数
        else if(ans!=0)
        {
            backChange(cnf,&st,-1,NULL);
            backLearnClause(cnf,f);
            backAssign(cnf,f);
            return ans+1;
        }
        else if(ans==0)
            return ans;

}

status createLearnClause(CNF* cnf,int* a,int i,int x)
{
    //采用非时序回溯，当冲突发生，不是简单地回到父节点
    //采用蕴含图，找到与冲突所关联的决策变量赋值的层数，选出最大的即为所要返回的层数
    //传入的参数分别为：cnf文件，冲突变元数组，冲突变元的个数，变元x
    int j,s;
    int* copy=a;
    int floor=0;
    int backtofloor=0;
    for(j=i-1;j>=0;j--)
        if(floor<cnf->varfloor[abs(copy[j])])
            floor=cnf->varfloor[abs(copy[j])];      //降低决策层数，即减少赋值的次数
    for(s=0,j=0;j<i;j++)
    {
        if(cnf->varfloor[abs(copy[j])]!=floor)      //删除本层已经赋值的变元
        {
            copy[s]=copy[j];
            s++;
        }
    }
    for(j=0;j<s;j++)            //此时cpoy数组中最大的变元决策层数即为要返回的决策层
        if(backtofloor<cnf->varfloor[abs(copy[j])])
            backtofloor=cnf->varfloor[abs(copy[j])];
    //学习子句太长，化为单子句，在回溯时删除
    int pre_length=s+1; //学习子句原长
    Floor* F=(Floor*)malloc(sizeof(Floor)*s);
    for(i=0;i<s;i++)
    {
        F[i].a=copy[i];
        F[i].floor=cnf->varfloor[abs(copy[i])];
    }
    qsort(F,s,sizeof(Floor),compare_Des);       //将变元的决策层变为降序
    for(i=0;i<s;i++)
        copy[i]=F[i].a;
    free(F);
    copy[s]=-x;     //加入当前层的决策变量x（x取真时发生冲突，所以学习子句中应该取非在加入）
    s++;
    int temp;
    for(j=s-1;j>0;j--)
    {
        temp=copy[j-1];
        copy[j-1]=copy[j];
        copy[j]=temp;
    }
    addClause(cnf,s,copy);      //将学习子句插入到cnf文件中
    LearnClause* lclause=(LearnClause* )malloc(sizeof(LearnClause));
    lclause->clause=cnf->root;
    lclause->count=pre_length;
    lclause->floor=floor;
    lclause->isInStack=FALSE;
    lclause->next=cnf->learn_root;
    cnf->learn_root=lclause;
    //vsids，加入学习子句后，vsids数组同时更新
    for(i=0;i<pre_length;i++)
    {
        if(copy[i]>0)
            cnf->vsids[copy[i]]++;
        else
            cnf->vsids[cnf->varnum-copy[i]]++;
    }
    //添加学习子句的索引
    addLIndex(cnf,lclause->clause);

    //学习子句变元有赋值
    while(cnf->root->head)
        removevar(cnf->root,cnf->root->head->var);
    return backtofloor;
}

status backLearnClause(CNF* cnf,int floor)
{
    Node* node;
    LearnClause* lclause=cnf->learn_root;
    while(lclause)
    {
        if(lclause->isInStack==FALSE)
        {
            for(node=lclause->clause->rmv;node&&cnf->varfloor[abs(node->var)]==floor;node=lclause->clause->rmv)
                backvar(lclause->clause,node);
            if(node==NULL)
                lclause->isInStack=TRUE;
        }
        lclause=lclause->next;
    }
    return OK;
}

status deleteRepeateLearnClause(CNF* cnf,LearnClause* lclause)
{
    Node* nodep,*nodeq;
    LearnClause* lcp=cnf->learn_root;
    lcp=lclause->next;
    if(lcp)
    {
        nodep=lcp->clause->head;
        if(nodep&&nodep->var==lclause->clause->head->var)
        {
            for(nodep=lcp->clause->rmv,nodeq=cnf->learn_root->clause->rmv;nodep&&nodeq;nodep=nodep->next,nodeq=nodeq->next)
                if(nodep->var!=nodeq->var)
                    break;
            if(nodep==NULL&&nodeq==NULL)        //重复学习子句
            {
                LearnClause* temp=lcp->next;
                deleteLearnClause(cnf,lcp);
                lcp=temp;
            }
            else
                lcp=lcp->next;
        }
    }
    return OK;
}

status backChange(CNF* cnf,ChangeStack* head,int time,int* learnarray)
{
    //learnarray数组中，第0个元素保存了该数组的长度，第一个元素到第i个元素为冲突的变元，i表示撤销的次数，第i+1个元素为唯一蕴含点
    //在蕴含图中，唯一蕴含点即为当前节点到冲突节点的所有路径中都要经过的节点
    //唯一蕴含点所对应的变量的赋值是当前决策层产生冲突的直接原因
    //学习子句就是将这些节点所对应的变元赋值取其返构成的字句加入子句集中
    ChangeStack* stack=head->next;
    int flag=FALSE;
    int* IGraph;    //蕴含图
    int* visitedIGraph;  //变元是否赋值
    int i=1,j;
    int islearn=FALSE;
    int backtofloor=head->floor;
    Node* node;
    while(stack&&time!=0)
    {
        switch(stack->tag)
        {
        case CLAUSE:
            insertClause(cnf,stack->Cp);    //将被删除的子句放回子句集中
            break;
        case LITERAL:
            backvar(stack->Cp,stack->Np);   //恢复变元
            if(learnarray&&flag==FALSE)     //将产生冲突的变元加入a[]中
            {
                if(stack->Cp->length==1)        //将发生冲突的变元加入到数组中
                    {
                        flag=TRUE;
                        IGraph=(int* )malloc(sizeof(int)*(cnf->varnum+2));
                        visitedIGraph=(int* )malloc(sizeof(int)*(cnf->varnum+1));
                        for(j=1;j<=cnf->varnum;j++)
                            visitedIGraph[j]=FALSE;
                        learnarray[i++]=stack->Cp->head->var;       //存放该变元的变量
                        learnarray[i++]=-(stack->Cp->head->var);
                        visitedIGraph[abs(stack->Cp->head->var)]=TRUE;      //变元var已经放入
                    }
            }
            break;
        case SPLIT:
            deleteClause(cnf,stack->Cp);        //因为split是决策变量作为单子句插入进去的，所以恢复是应该删除
            break;
        default:
            break;
        }
        if(learnarray&&islearn==FALSE)
        {
            if(stack->Cp->length==1)
            {
                learnarray[i]=-(stack->Cp->head->var);      //监视哨，取负是因为stack里面的文字都是决策变元的非
                for(j=1;stack->Cp->head->var!=-learnarray[j];j++)
                    ;
                if(j!=i)
                {                   //不存在，访问
                    IGraph[j]=TRUE;
                    int s=j;
                    for(j=2;j<i;j++)
                        if(IGraph[j]!=TRUE)
                            break;
                    if(i==j&&i!=3&&islearn==FALSE)
                    {
                        islearn=TRUE;
                        learnarray[0]=i-1;      //下标为0存放冲突变元的个数
                        learnarray[i]=-learnarray[s];
                        int f=createLearnClause(cnf,learnarray+1,learnarray[0],-learnarray[s]);
                        if(f<backtofloor)
                            backtofloor=f;
                    }
                    else
                    {
                        for(node=stack->Cp->rmv;node;node=node->next)   //删除重复的变元
                        {
                            if(visitedIGraph[abs(node->var)]==FALSE)
                            {
                                learnarray[i]=node->var;
                                IGraph[i]=FALSE;
                                visitedIGraph[abs(node->var)]=TRUE;
                                if(cnf->varfloor[abs(learnarray[i])]<head->floor)
                                    IGraph[i]=TRUE;
                                i++;
                            }
                        }//end of for
                    }//end of else
                }//end of if
            }//end of if
        }//end of if
        time--;
        head->next=head->next->next;
        free(stack);
        stack=head->next;
    }//end of while
    if(learnarray)
    {
        free(IGraph);
        free(visitedIGraph);
        learnarray[0]=backtofloor;
    }
    return OK;
}//end of backChange

int compare_Des(const void *a,const void *b)
{
    return ((Floor*)b)->floor - ((Floor*)a)->floor;
}

status check(CNF* cnf)
{
    Clause* clause;
    Node* node;
    int i,ans;
    int flag=0;
    printf("-----------------------------\n");
    printf("检查中\n");
    for(clause=cnf->root,i=1;clause;clause=clause->next,i++)
    {
        printf("正在检查第%d个子句：",i);
        for(node=clause->head;node;node=node->next)
        {
            ans=cnf->var[abs(node->var)];
            if(EVALUE(node->var)==ans)
            {
                printf("正确! ");
                flag=1;
                break;
            }
            else if(EVALUE(node->var)==-ans)
                continue;
             else if(ans==NOTSURE)
                printf("%d不确定(未出现) ",abs(node->var));
        }
        if(!flag)
            printf("子句不满足！\n");     //只要有一个变元取值为真，则子句就是可满足的
        if(i%4==0)
            printf("\n");
        else printf("\t\t");
    }
    printf("检查完成！\n");
    return OK;
}
