#include "DPLL.h"
#include <time.h>

#define DEBUG2 0

#define VSIDS 2
#define VSIDSCOUNT 100
#define LEARNLENGTH_MAX 20      //����ѧϰ�Ӿ����Ŀ
#define OUTTIME 240000

int beginTime;
status reset=FALSE; //������������������б����ĸ�ֵ״̬������ѡ��һ����߱������и�ֵȻ�������������������
int MAXC = 10;

enum strategy {mom, vsids};

status saveChange(ChangeStack* head,int tag,Clause* clause,Node* node)
{
    //headΪͷָ��
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
    case LITERAL:       //��Ԫ�ı���ζ���Ӿ�Ҳ�����ı�
        newchange->Np=node;
        newchange->Cp=clause;
        break;
    case SPLIT:     //��Ϊ���Ӿ�
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
        //������־��ǿ�����ģ���ɾ�����־䣬���޷�ȷ������ɾ���������
        switch(evaluateClause(clause,var))
        {
        case TRUE:
            removeClause(cnf,clause);
            if(head)        //�����޸ģ�
                saveChange(head,CLAUSE,clause,NULL);
            clause=clause->next;
            break;
        case NOTSURE:
            node=removevar(clause,-var);        //-var!!!
            if(head)
                saveChange(head,LITERAL,clause,node);
            //���Ƴ���Ԫ���Ӿ��Ϊ���Ӿ���߿��Ӿ䣬Ϊ�˲������㣬�������ڶ�����ͬʱ���ؿ��Ӿ�
            if(clause->length<=1)
            {
                clausep=clause->next;
                removeClause(cnf,clause);
                insertClause(cnf,clause);
                if(clause->length==0)
                    return ERROR;      //����֮����ڿ��Ӿ䣬����ʧ��
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
    LiteralIndex* lindex;       //ָ��ǰ��Ԫ���ڵ�����
    Clause* clausep;
    Node* node;
    removeClause(cnf,clause);   //�Ƴ�����ĵ��Ӿ�
    saveChange(head,CLAUSE,clause,NULL);
    for(lindex=cnf->lindex[index].next;lindex;lindex=lindex->next)  //ͨ�������ҵ����б�Ԫvar���Ӿ�
    {
        clausep=lindex->Cp;     //clausep��ʼ��Ϊָ���б�Ԫvar�ĵ�һ���Ӿ䣨��������ʱ�ĵ�һ����
        node=lindex->Np;
        if(clausep->isremoved==FALSE)
        {
            if(node->var==var)
            {
                removeClause(cnf,clausep);  //ɾ������L���Ӿ䣬
                if(head)
                    saveChange(head,CLAUSE,clausep,NULL);   //�����޸�
            }
            else if(node->var==-var)        //ɾ���Ӿ��к��з�L������
            {
                node=removevar(clausep,-var);
                if(head)
                    saveChange(head,LITERAL,clausep,node);

            //�����Ӿ�Ϳ��Ӿ��ö�����ʡ����ʱ��
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
        if(cnf->varfloor[i]==floor)     //�ҵ��ò�ı�Ԫ
        {
            //�ظ�����
            cnf->var[i]=NOTSURE;
            cnf->varfloor[i]=-1;
        }
    }
    return OK;
}

status MOM(CNF* cnf)
{
    //����Ӿ����Ƶ��������ȷ�
    //�Ӿ䳤��Խ��Խ�������㣬��˴ӽ϶��Ӿ�����ѡ���ϴ�Ƶ�ʵ�����ʹ����������
    //�㷨���ص�������������l���Ӧ��Jָ������ѡJֵ��������l��ֵΪ��
    //ni��Ϊ��������l���־�Ci���־�ĳ��ȣ���Jֵ��Ϊ2�ĸ�ni�η����,����Խ��,JֵԽС
    double* J=(double*)malloc(sizeof(double)*(cnf->varnum+1));
    for(int i=1;i<=cnf->varnum;i++)     //��ʼ��Ϊ0
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
        //Ѱ������Jֵ��Ӧ�ı�Ԫ(ֻȷ����Ԫ�ľ���ֵ����ȷ��������߷ǣ�
        {
            max=J[i];
            index=i;
        }
    }
    //ȷ��ȡ�����ȡ��,���Ӿ伯����һ��ȡ�ö�,Ƶ�����
   // printf("indexΪ��%d\n",index);
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
    // Ϊÿһ����Ԫ���������ָ���һ������������ʼֵ��Ϊ���Ӿ伯�ֵĳ��ִ���
    //����δ��ֵ�����е����ּ�������ֵ�����򣬴���ѡ��һ���Ǽ�����ֵ����һ��������Ϊ���߱���
    int max=0;
    int index;
    static int count=0;        //��¼���ߴ���
    for(int i=1;i<=cnf->varnum;i++)
        if(max<=cnf->vsids[i]+cnf->vsids[i+cnf->varnum])        //���������ֳ��ִ��������ͷǣ�
            if(cnf->var[i]==NOTSURE)        //�ҵ����
            {
                max=cnf->vsids[i]+cnf->vsids[i+cnf->varnum];
                index=i;
            }
    count++;
    if(count>=VSIDSCOUNT)
    {
        //Ϊ�˷�ֹһЩ������ʱ��ò�����ֵ������һ���ľ��ߴ���֮��ÿһ�����ֵļ�������ֵ�������һ������
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
    //fΪ���߲㣬���õݹ��˼�룬��f=0��ʼ������
    Clause* clause;
    ChangeStack st;
    st.next=NULL;
    int v;
    if(f==0)        //��һ�㣬��ʼ�㷨
        beginTime=clock();
    while((clause=LocateUnitClause(cnf))!=NULL)     //���ڵ��Ӿ�
    {
        arrayAssign(cnf,clause->head->var,f);
        SimplifySingleClause(cnf,clause->head->var,&st);      //���õ��Ӿ���򻯼��Ӿ伯S
        if(haveEmptyClause(cnf)==TRUE)      //���S���ڿ��Ӿ�
        {
            backChange(cnf,&st,-1,NULL);
            backAssign(cnf,f);
            if(clock()-beginTime>=OUTTIME)
            {
                printf("��ʱ��\n");
                return NOTSURE;
            }
            return FALSE;
        }
        else if(haveClause(cnf)==FALSE)     //SΪ�գ�����
        {
            backChange(cnf,&st,-1,NULL);
            return TRUE;
        }
    }//end of while
    //ѡ�����ѡ���Ԫv
    v=combineStrategy(cnf,mom);
    arrayAssign(cnf,v,f);
    addClause(cnf,1,&v); //��Ԫv������Ϊ�Ӿ���뵽�Ӿ伯S��,�ڶ�������Ϊ��Ԫ��Ŀ���Ǳ�Ԫ��ֵ������
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
    v=-v;       //ȡ��
    arrayAssign(cnf,v,f);
    addClause(cnf,1,&v);        //��Ԫv������Ϊ�Ӿ���뵽cnf��
    saveChange(&st,SPLIT,cnf->root,NULL);
    ans=DPLL(cnf,f+1);
    backChange(cnf,&st,-1,NULL);
    if(ans==FALSE)
        backAssign(cnf,f);
    return ans;
}

status advancedDPLL(CNF* cnf,int f)
{
    //�Ż����dpll�㷨����cdll�㷨
    int back=f;//���ݵĲ���
    Clause* clause;
    ChangeStack st;
    st.floor=f;
    st.next=NULL;
    int ans;
    int count=0;
    //VSIDS����
    if(f==0)
    {
        beginTime=clock();
        Node* node;
        for(clause=cnf->root;clause;clause=clause->next)
            for(node=clause->head;node;node=node->next)
            {
                if(node->var>0)
                    cnf->vsids[node->var]++;    //ͳ��������
                else
                    cnf->vsids[cnf->varnum-node->var]++;        //������
            }
    }
        //������ǣ���������ı�������˳����ܲ������ŵģ���ͻᵼ������������ĳЩ�Ӿ�ռ���װ�����ʱ��
        //��ʱ���������������ƣ������Ŀǰ���б����ĸ�ֵ״̬������ѡ��һ����߱������и�ֵ
        do
        {
            count++;
            if(reset==TRUE)
            {
                //������
                reset=FALSE;
                LearnClause* lclause=cnf->learn_root;
                LearnClause* lclausep=NULL;
                while(lclause)
                {
                    //�Ӿ�ɾ�����ƣ�ɾ��һЩ�ܳ���ѧϰ�Ӿ�
                    if(lclause->clause->rmv==NULL&&lclause->clause->length>LEARNLENGTH_MAX)
                    {
                        //ɾ����Щû���Ƴ���ԪҲ����û�и�ֵ���ı�Ԫ�����ҳ��Ⱥܳ���ѧϰ�Ӿ�
                        lclausep=lclause->next;
                        deleteClauseLIndex(cnf,lclause->clause);
                        deleteLearnClause(cnf,lclause);
                        lclause=lclausep;
                    }
                    else
                        lclause=lclause->next;
                }
            }
            while((clause=LocateUnitClause(cnf))!=NULL) //���Ӿ�����
            {
                arrayAssign(cnf,clause->head->var,f);
                advancedSimplifySingleClause(cnf,clause,&st);
                if(haveEmptyClause(cnf)==TRUE)
                {
                    if(clock()-beginTime>=OUTTIME)
                    {
                        printf("��ʱ��\n");
                        return -1-f;
                    }
                    int* a=(int* )malloc(sizeof(int)*(cnf->varnum+3));  //������������Ԫ��Ψһ�̺���
                    backChange(cnf,&st,-1,a);   //��ԭ����һ�λظ�Ӧ����ɾ������Ӿ��գ�������ͻ
                    //��������ͻ�����ѧϰ�Ӿ䲢���з�ʱ�����
                    back=a[0];
                    free(a);
                    backLearnClause(cnf,f);
                    //��ֵ����
                    backAssign(cnf,f);
                    if(count>MAXC)
                    {
                        MAXC++;
                        //������
                        reset=TRUE;
                        //ѧϰ�Ӿ����
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
        //�ɷ���ֵ�������ݲ���
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
    //���÷�ʱ����ݣ�����ͻ���������Ǽ򵥵ػص����ڵ�
    //�����̺�ͼ���ҵ����ͻ�������ľ��߱�����ֵ�Ĳ�����ѡ�����ļ�Ϊ��Ҫ���صĲ���
    //����Ĳ����ֱ�Ϊ��cnf�ļ�����ͻ��Ԫ���飬��ͻ��Ԫ�ĸ�������Ԫx
    int j,s;
    int* copy=a;
    int floor=0;
    int backtofloor=0;
    for(j=i-1;j>=0;j--)
        if(floor<cnf->varfloor[abs(copy[j])])
            floor=cnf->varfloor[abs(copy[j])];      //���;��߲����������ٸ�ֵ�Ĵ���
    for(s=0,j=0;j<i;j++)
    {
        if(cnf->varfloor[abs(copy[j])]!=floor)      //ɾ�������Ѿ���ֵ�ı�Ԫ
        {
            copy[s]=copy[j];
            s++;
        }
    }
    for(j=0;j<s;j++)            //��ʱcpoy���������ı�Ԫ���߲�����ΪҪ���صľ��߲�
        if(backtofloor<cnf->varfloor[abs(copy[j])])
            backtofloor=cnf->varfloor[abs(copy[j])];
    //ѧϰ�Ӿ�̫������Ϊ���Ӿ䣬�ڻ���ʱɾ��
    int pre_length=s+1; //ѧϰ�Ӿ�ԭ��
    Floor* F=(Floor*)malloc(sizeof(Floor)*s);
    for(i=0;i<s;i++)
    {
        F[i].a=copy[i];
        F[i].floor=cnf->varfloor[abs(copy[i])];
    }
    qsort(F,s,sizeof(Floor),compare_Des);       //����Ԫ�ľ��߲��Ϊ����
    for(i=0;i<s;i++)
        copy[i]=F[i].a;
    free(F);
    copy[s]=-x;     //���뵱ǰ��ľ��߱���x��xȡ��ʱ������ͻ������ѧϰ�Ӿ���Ӧ��ȡ���ڼ��룩
    s++;
    int temp;
    for(j=s-1;j>0;j--)
    {
        temp=copy[j-1];
        copy[j-1]=copy[j];
        copy[j]=temp;
    }
    addClause(cnf,s,copy);      //��ѧϰ�Ӿ���뵽cnf�ļ���
    LearnClause* lclause=(LearnClause* )malloc(sizeof(LearnClause));
    lclause->clause=cnf->root;
    lclause->count=pre_length;
    lclause->floor=floor;
    lclause->isInStack=FALSE;
    lclause->next=cnf->learn_root;
    cnf->learn_root=lclause;
    //vsids������ѧϰ�Ӿ��vsids����ͬʱ����
    for(i=0;i<pre_length;i++)
    {
        if(copy[i]>0)
            cnf->vsids[copy[i]]++;
        else
            cnf->vsids[cnf->varnum-copy[i]]++;
    }
    //���ѧϰ�Ӿ������
    addLIndex(cnf,lclause->clause);

    //ѧϰ�Ӿ��Ԫ�и�ֵ
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
            if(nodep==NULL&&nodeq==NULL)        //�ظ�ѧϰ�Ӿ�
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
    //learnarray�����У���0��Ԫ�ر����˸�����ĳ��ȣ���һ��Ԫ�ص���i��Ԫ��Ϊ��ͻ�ı�Ԫ��i��ʾ�����Ĵ�������i+1��Ԫ��ΪΨһ�̺���
    //���̺�ͼ�У�Ψһ�̺��㼴Ϊ��ǰ�ڵ㵽��ͻ�ڵ������·���ж�Ҫ�����Ľڵ�
    //Ψһ�̺�������Ӧ�ı����ĸ�ֵ�ǵ�ǰ���߲������ͻ��ֱ��ԭ��
    //ѧϰ�Ӿ���ǽ���Щ�ڵ�����Ӧ�ı�Ԫ��ֵȡ�䷵���ɵ��־�����Ӿ伯��
    ChangeStack* stack=head->next;
    int flag=FALSE;
    int* IGraph;    //�̺�ͼ
    int* visitedIGraph;  //��Ԫ�Ƿ�ֵ
    int i=1,j;
    int islearn=FALSE;
    int backtofloor=head->floor;
    Node* node;
    while(stack&&time!=0)
    {
        switch(stack->tag)
        {
        case CLAUSE:
            insertClause(cnf,stack->Cp);    //����ɾ�����Ӿ�Ż��Ӿ伯��
            break;
        case LITERAL:
            backvar(stack->Cp,stack->Np);   //�ָ���Ԫ
            if(learnarray&&flag==FALSE)     //��������ͻ�ı�Ԫ����a[]��
            {
                if(stack->Cp->length==1)        //��������ͻ�ı�Ԫ���뵽������
                    {
                        flag=TRUE;
                        IGraph=(int* )malloc(sizeof(int)*(cnf->varnum+2));
                        visitedIGraph=(int* )malloc(sizeof(int)*(cnf->varnum+1));
                        for(j=1;j<=cnf->varnum;j++)
                            visitedIGraph[j]=FALSE;
                        learnarray[i++]=stack->Cp->head->var;       //��Ÿñ�Ԫ�ı���
                        learnarray[i++]=-(stack->Cp->head->var);
                        visitedIGraph[abs(stack->Cp->head->var)]=TRUE;      //��Ԫvar�Ѿ�����
                    }
            }
            break;
        case SPLIT:
            deleteClause(cnf,stack->Cp);        //��Ϊsplit�Ǿ��߱�����Ϊ���Ӿ�����ȥ�ģ����Իָ���Ӧ��ɾ��
            break;
        default:
            break;
        }
        if(learnarray&&islearn==FALSE)
        {
            if(stack->Cp->length==1)
            {
                learnarray[i]=-(stack->Cp->head->var);      //�����ڣ�ȡ������Ϊstack��������ֶ��Ǿ��߱�Ԫ�ķ�
                for(j=1;stack->Cp->head->var!=-learnarray[j];j++)
                    ;
                if(j!=i)
                {                   //�����ڣ�����
                    IGraph[j]=TRUE;
                    int s=j;
                    for(j=2;j<i;j++)
                        if(IGraph[j]!=TRUE)
                            break;
                    if(i==j&&i!=3&&islearn==FALSE)
                    {
                        islearn=TRUE;
                        learnarray[0]=i-1;      //�±�Ϊ0��ų�ͻ��Ԫ�ĸ���
                        learnarray[i]=-learnarray[s];
                        int f=createLearnClause(cnf,learnarray+1,learnarray[0],-learnarray[s]);
                        if(f<backtofloor)
                            backtofloor=f;
                    }
                    else
                    {
                        for(node=stack->Cp->rmv;node;node=node->next)   //ɾ���ظ��ı�Ԫ
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
    printf("�����\n");
    for(clause=cnf->root,i=1;clause;clause=clause->next,i++)
    {
        printf("���ڼ���%d���Ӿ䣺",i);
        for(node=clause->head;node;node=node->next)
        {
            ans=cnf->var[abs(node->var)];
            if(EVALUE(node->var)==ans)
            {
                printf("��ȷ! ");
                flag=1;
                break;
            }
            else if(EVALUE(node->var)==-ans)
                continue;
             else if(ans==NOTSURE)
                printf("%d��ȷ��(δ����) ",abs(node->var));
        }
        if(!flag)
            printf("�Ӿ䲻���㣡\n");     //ֻҪ��һ����ԪȡֵΪ�棬���Ӿ���ǿ������
        if(i%4==0)
            printf("\n");
        else printf("\t\t");
    }
    printf("�����ɣ�\n");
    return OK;
}
