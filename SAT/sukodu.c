#include "sudoku.h"
#include "dpll.h"

Sudoku* createSudoku(int difficult)
{
    int i,j,k;
    int x,y;
    int flag=0;
    Sudoku* s=(Sudoku* )malloc(sizeof(Sudoku));
    Sudoku* ss=NULL;
    if(!s)
    {
       printf("�ڴ���䲻����\n");
        return NULL;
    }

    for(i=0;i<6;i++)
        for(j=0;j<6;j++)
            s->sdk[i][j]=-1;
    if(randSudoku(s,0,0)==TRUE)
    {
       printf("\n\n����������ɣ�-------------------------------------------\n\n");
        //������ڵ�һ���գ���ʱ���ɵ�����һ����ֻ��һ�����
        x=rand()%6;
        y=rand()%6;
        s->sdk[x][y]=-1;
        //�������̴��¾���Ϊ5�����䣬��Ӧ��5���Ѷȵȼ���ÿ���������Ϊ7-8������
        int max=7*difficult;        //��������ڵĶ���������
        while(1)
        {
            solveSudoku(s,&i);      //�������������ý�ĸ���i
            if(i>1)
            {
                flag--;
                s->sdk[x][y]=k;     //���ڵ���s->sdk[x][y]���ڵ�֮���޷���֤��Ψһ�⣬�ָ��������ԭ����ֵ
                for(ss=s->next;ss;ss=s->next)   //��֮ǰ��������в�Ψһ�Ľ�ȫ��ɾ��
                {
                    s->next=ss->next;
                    free(ss);
                }
            }
            else        //��һ������һ��֮�󣬵õ�Ψһ��������̣����Լ����ڶ�
            {
                flag++;
                if(flag>=max)
                    break;
                do
                {

                   x=rand()%6;
                   y=rand()%6;
                }while(s->sdk[x][y]==-1);       //�ҵ������ڵĸ���
                k=s->sdk[x][y];
                s->sdk[x][y]=-1;            //�ڶ�
            }
        }
        free(ss);
        s->next=NULL;
        return s;
    }
    else
        return NULL;
}

status randSudoku(Sudoku* s,int x,int y)
{
    int i=-1;
    int tmp=-2;        //Ϊ�˷�ֹ��ʱ�������ֵ����Ϊͬһ��ֵ
    do
    {
        do
        {
            i++;
            if(i>1)
            {
                s->sdk[x][y]=-1;
                return FALSE;       //������0����1����������
            }
        tmp=s->sdk[x][y];
        s->sdk[x][y]=rand()%2;  //����0-1
        if(tmp==s->sdk[x][y])
                s->sdk[x][y]=(s->sdk[x][y])?0:1;    //����

        }while(checkSudoku(s)!=TRUE);
        //printf("��%d�е�%d�����ɳɹ���\n",x,y);
        if(x==5&&y==5)
        {
            printf("���ɳɹ���\n");
            return TRUE;
        }
    }while(((y==5)?randSudoku(s,x+1,0):randSudoku(s,x,y+1))!=TRUE);
    return TRUE;

}
status SaveSudoku(void)
{
    int s[6][6];
    FILE* fp;
    char path[200];
    printf("�������������\n��-1��ʾ�ո�������0��1\n");
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
            scanf("%d",&s[i][j]);
    printf("���뱣���·����\n");
    scanf("%s",path);
    if((fp=fopen(path,"w+"))==NULL)
    {
        printf("�ļ�����ʧ�ܣ�\n");
        exit(-1);
    }
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
            fwrite(&s[i][j],sizeof(int),1,fp);
    fclose(fp);
    return OK;
}
Sudoku* loadSudoku(void)
{
    Sudoku* s=(Sudoku* )malloc(sizeof(Sudoku));
    s->next=NULL;       //��ʼ�޷�ȷ���⣬�ǵø�ֵΪNULL
    printf("������ص��ļ����ƣ�\n");
    char filename[50];
    scanf("%s",filename);
    FILE* fp=fopen(filename,"rb");
    if(!fp)
    {
        printf("�ļ���ʧ�ܣ�\n");
        return NULL;
    }
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
            fread(&s->sdk[i][j],sizeof(int),1,fp);
    fclose(fp);
    return s;


}

CNF* transtoCNF(Sudoku* s)
{
    int i,j,k;
    CNF* cnf=(CNF* )malloc(sizeof(CNF));
    if(!createCNF(cnf,30000,0))
        return NULL;

    //��Ԥ��ĸ��ӵ�����ת��Ϊ�Ӿ���뵽�Ӿ伯��
    for(i=0;i<6;i++)
    {
        for(j=0;j<6;j++)
        {
            k=(i+1)*10+j+1;
            if(s->sdk[i][j]==0)
            {
                 k=-k;
                 addClause(cnf,1,&k);
            }
            else if(s->sdk[i][j]==1)
                addClause(cnf,1,&k);
        }
    }
    //printf("����ԭ���ݳɹ���\n");
    rule1(cnf);
    //printf("Լ��һ��ɣ�\n");
    rule2(cnf);
    //printf("Լ������ɣ�\n");
    rule3(cnf);
    //printf("Լ������ɣ�\n");
    return cnf;
}
status rule1(CNF* cnf)
{
//Լ��һ��ÿһ�У�ÿһ����������3��1��0
    //ÿһ��û��������3��1��0
    int i,j,k;
    int pos1[3];
    int neg1[3];     //�ֱ��ż��������������Ԫ����������
    int count;
    for(i=0;i<6;i++)
    {
        for(j=0,count=0;j<6;j++,count++)
        {
            k=(i+1)*10+j+1;
            pos1[count%3]=k;
            neg1[count%3]=-k;
            if((count+1)%3==0)       //ÿȡ��������������ʱ������Ҫ�����Ӿ�
            {
                addClause(cnf,3,pos1);
                addClause(cnf,3,neg1);
                j -=2;
            }
        }
    }
    //ÿһ��û�У�����i,j����
    for(j=0;j<6;j++)
    {
        for(i=0,count=0;i<6;i++,count++)
        {
            k=(i+1)*10+j+1;
            pos1[count%3]=k;
            neg1[count%3]=-k;
            if((count+1)%3==0)       //ÿȡ��������������ʱ������Ҫ�����Ӿ�
            {
                addClause(cnf,3,pos1);
                addClause(cnf,3,neg1);
                i -=2;
            }
        }
    }
    return OK;
}

status rule2(CNF* cnf)
{
//Լ������ÿһ��ÿһ����1��0�ĸ�����ͬ
    //��6���У���1��0����Ϊ3������ζ����ѡ���ĸ���Ԫ������һ��1����0
    //����ÿһ�л���ÿһ����һ��15��ѡ��
    int i,j;
    int select[15][4];     //���ÿһ�ֿ���
    rule2_select4unit(select);     //ֻ�轫��һ�е�15�п���ȫ���г����������У��о����ɴ˱仯
    int pos_select[15][4];
    int neg_select[15][4];
    int count;
    for(i=0;i<15;i++)
        for(j=0;j<4;j++)
        {
            pos_select[i][j]=select[i][j];
            neg_select[i][j]=-pos_select[i][j];
        }

    //���ȿ�����
    for(i=0;i<6;i++)
    {
        for(count=0;count<15;count++)
        {
            addClause(cnf,4,pos_select[count]);
            addClause(cnf,4,neg_select[count]);
        }
        for(count=0;count<15;count++)
            for(j=0;j<4;j++)
            {
                pos_select[count][j] +=10;
                neg_select[count][j]=-pos_select[count][j];
            }
    }
    //�еĻ���ʵ���ǽ�������������ֽ��з�ת
    for(i=0;i<15;i++)
        for(j=0;j<4;j++)
        {
            pos_select[i][j]=(select[i][j])%10*10+(select[i][j])/10;
            neg_select[i][j]=-pos_select[i][j];
        }
    for(j=0;j<6;j++)
    {
        for(count=0;count<15;count++)
        {
            addClause(cnf,4,pos_select[count]);
            addClause(cnf,4,neg_select[count]);
        }
        for(count=0;count<15;count++)
            for(i=0;i<4;i++)
            {
                pos_select[count][i] +=1;       //ע�����ͬ��ÿһ��֮��ֻ���1
                neg_select[count][i]=-pos_select[count][i];
            }
    }
    return OK;
}

status rule3(CNF* cnf)
{
    //Լ�������������ظ����к���
    //�������в��ظ�����Ҫ����1+6+6*2=19�����ӱ�Ԫ
    //��ʾԼ��������Ҫ19*15*2=570�����ӱ�Ԫ
    //�����2*15*��9*6+7��=1830�������Ӿ�
    //1. �����x�е�j�к͵�y�е�j�о�ȡ1���Ӿ䣬��1xyj1,�Ե�3�к͵�5��Ϊ��
    int i,j,x,y;
    int line0[6];       //13510��13520...13560
    int line1[6];       //13511��13521...13561
    int line[6];        //1351��1352...1356
    int var1,var2;      //��¼����������Ԫ
    for(x=0;x<6;x++)
        for(y=x+1;y<6;y++)
            {
                //��line0��ֵ��ת��Ϊ�Ӿ�����Ӿ伯
                for(j=0;j<6;j++)    //��j����Ԫ
                {
                        var1=(x+1)*10+j+1;       //31
                        var2=(y+1)*10+j+1;      //51
                        line0[j]=0+(j+1)*10+(y+1)*100+(x+1)*1000+1*10000;   //13510
                        line1[j]=1+(j+1)*10+(y+1)*100+(x+1)*1000+1*10000;   //13511
                        //13511=31��51,���������Ӿ�,13511��13510������6���Ӿ�
                        int clause11[2],clause01[2];
                        clause11[0]=var1;clause11[1]=-line1[j];
                        clause01[0]=-var1;clause01[1]=-line0[j];
                        addClause(cnf,2,clause11);addClause(cnf,2,clause01);
                        int clause12[2],clause02[2];
                        clause12[0]=var2;clause12[1]=-line1[j];
                        clause02[0]=-var2;clause02[1]=-line0[j];
                        addClause(cnf,2,clause12);addClause(cnf,2,clause02);
                        int clause13[3],clause03[3];
                        clause13[0]=-var1;clause13[1]=-var2;clause13[2]=line1[j];
                        clause03[0]=var1;clause03[1]=var2;clause03[2]=line0[j];
                        addClause(cnf,3,clause13);addClause(cnf,3,clause03);
                        line[j]=j+1+(y+1)*10+(x+1)*100+1000;    //1351
                        //1351ת��Ϊ3���Ӿ�
                        int c1[2],c2[2],c3[3];
                        c1[0]=-line1[j];c1[1]=line[j];
                        addClause(cnf,2,c1);
                        c2[0]=-line0[j];c2[1]=line[j];
                        addClause(cnf,2,c2);
                        c3[0]=line0[j];c3[1]=line1[j];c3[2]=-line[j];
                        addClause(cnf,3,c3);
                }
                //135��Ϊ��ʽ����7���Ӿ�,��һ����7����Ԫ����������Ϊ2����Ԫ��ȫ������
                int clause1[7];
                int clause[6][2];
                for(i=0;i<6;i++)
                {
                    clause[i][0]=line[i];       //135i
                    clause1[i]=-line[i];
                    clause[i][1]=y+1+(x+1)*10+100;  //135
                    addClause(cnf,2,clause[i]);
                }
                clause1[6]=-((y+1)+(x+1)*10+100);
                addClause(cnf,7,clause1);
                clause1[6]=-clause1[6];
                addClause(cnf,1,&clause1[6]);       //Ҫ��������135���룡����
            }
    //2. ���ظ��У�����һλ��1�����б�2��2xyj1��ʾ��x�к͵�y�еĵ�j�ж�ȡ1��
    for(x=0;x<6;x++)
        for(y=x+1;y<6;y++)
            {
                //��line0��ֵ��ת��Ϊ�Ӿ�����Ӿ伯
                for(j=0;j<6;j++)    //��j����Ԫ
                {
                        var1=(j+1)*10+x+1;       //31       //һ��Ҫע���ʱj����Ϊ�У����� Ӧ����j+1�ڳ���10
                        var2=(j+1)*10+y+1;      //51
                        line0[j]=0+(j+1)*10+(y+1)*100+(x+1)*1000+2*10000;   //23510
                        line1[j]=1+(j+1)*10+(y+1)*100+(x+1)*1000+2*10000;   //23511
                        //23511=31��51,���������Ӿ�,13511��13510������6���Ӿ�
                        int clause11[2],clause01[2];
                        clause11[0]=var1;clause11[1]=-line1[j];
                        clause01[0]=-var1;clause01[1]=-line0[j];
                        addClause(cnf,2,clause11);addClause(cnf,2,clause01);
                        int clause12[2],clause02[2];
                        clause12[0]=var2;clause12[1]=-line1[j];
                        clause02[0]=-var2;clause02[1]=-line0[j];
                        addClause(cnf,2,clause12);addClause(cnf,2,clause02);
                        int clause13[3],clause03[3];
                        clause13[0]=-var1;clause13[1]=-var2;clause13[2]=line1[j];
                        clause03[0]=var1;clause03[1]=var2;clause03[2]=line0[j];
                        addClause(cnf,3,clause13);addClause(cnf,3,clause03);
                        line[j]=j+1+(y+1)*10+(x+1)*100+2000;    //2351
                        //1351ת��Ϊ3���Ӿ�
                        int c1[2],c2[2],c3[3];
                        c1[0]=-line1[j];c1[1]=line[j];
                        addClause(cnf,2,c1);
                        c2[0]=-line0[j];c2[1]=line[j];
                        addClause(cnf,2,c2);
                        c3[0]=line0[j];c3[1]=line1[j];c3[2]=-line[j];
                        addClause(cnf,3,c3);
                }
                //135��Ϊ��ʽ����7���Ӿ�,��һ����7����Ԫ����������Ϊ2����Ԫ��ȫ������
                    int clause1[7];
                    int clause[6][2];
                    for(i=0;i<6;i++)
                    {
                        clause[i][0]=line[i];
                        clause1[i]=-line[i];
                        clause[i][1]=y+1+(x+1)*10+200;  //235
                        addClause(cnf,2,clause[i]);
                    }
                    clause1[6]=-((y+1)+(x+1)*10+200);
                    addClause(cnf,7,clause1);
                    clause1[6]=-clause1[6];
                    addClause(cnf,1,&clause1[6]);
            }
    return OK;
}


Sudoku* transtoSudoku(CNF* cnf)
{
    Sudoku* s=(Sudoku* )malloc(sizeof(Sudoku));
    s->next=NULL;
    int i,j;
    for(i=0;i<6;i++)
        for(j=0;j<6;j++)
        {
            int var=j+1+(i+1)*10;
            //locvar(cnf,var);
            if(cnf->var[var]==TRUE)
                s->sdk[i][j]=1;
            else
                s->sdk[i][j]=0;
        }
    return s;
}

status rule2_select4unit(int s[][4])
{
    int i;
    //���ȸ㶨��һ��
    for(i=0;i<10;i++)       //11��ͷ��ʮ��
        s[i][0]=11;
    for( ;i<14;i++)        //12��ͷ������,i��10��ʼ��10��11��12��13��Ϊ12��ͷ
        s[i][0]=12;
    s[i][0]=13;
    //�ڶ���
    for(i=0;i<6;i++)    //�ڶ���Ϊ12����6��
        s[i][1]=12;
    for( ;i<9;i++)      //�ڶ���Ϊ13����6��7��8
        s[i][1]=13;
    s[i][1]=14;         //�ڶ���Ϊ14�Ľ�һ����i=9;
    for(i=10;i<13;i++)     //i=10,11,12ȡ13
        s[i][1]=13;
    for( ;i<15;i++)         //i=13,14ȡ14
        s[i][1]=14;
    //������
    s[0][2]=s[1][2]=s[2][2]=13;
    s[3][2]=s[4][2]=s[6][2]=s[7][2]=s[10][2]=s[11][2]=14;
    s[5][2]=s[8][2]=s[9][2]=s[13][2]=s[14][2]=s[12][2]=15;
    //���ĸ�
    s[0][3]=14;
    s[1][3]=s[3][3]=s[6][3]=s[10][3]=15;
    s[2][3]=s[4][3]=s[5][3]=s[7][3]=s[8][3]=s[9][3]=s[11][3]=s[12][3]=s[13][3]=s[14][3]=16;
    return OK;
}

status checkSudoku(Sudoku* s)
{
    if(checkRule1(s)==TRUE)
    {
        //printf("Լ��һ���㣡\n");
        if(checkRule2(s)==TRUE)
        {
            //printf("Լ�������㣡\n");
            if(checkRule3(s)==TRUE)
            {
               // printf("Լ�������㣡\n");
                return TRUE;
            }

            else
            {
            //    printf("Լ���������㣡\n");
                return FALSE;
            }

        }
        else
        {
          //  printf("Լ���������㣡\n");
            return FALSE;
        }

    }
    else
    {
       // printf("Լ��һ�����㣡\n");
        return FALSE;
    }

}

status checkRule1(Sudoku* s)
{
//��������Ƿ�Ϻ�Լ��
//Լ��һ
    //�����
    int i,j;
    int count;
    for(i=0;i<6;i++)
    {
        for(j=0,count=0;j<6;j++,count++)
        {
            if(!(s->sdk[i][j]==0||s->sdk[i][j]==1||s->sdk[i][j]==-1))
                return FALSE;
            else
            {
                if(s->sdk[i][j]==0&&s->sdk[i][j+1]==0&&s->sdk[i][j+2]==0)
                {
                    //printf("��%d %d %d�е�%d�в����㣡\n",j,j+1,j+2,i);
                    return FALSE;
                }

                else if(s->sdk[i][j]==1&&s->sdk[i][j+1]==1&&s->sdk[i][j+2]==1)
                {
                   // printf("��%d %d %d�е�%d�в����㣡\n",j,j+1,j+2,i);
                    return FALSE;
                }
                else
                {
                    if((count+1)%3==0&&count<11)
                        j-=2;
                    else
                        continue;
                }
            }
        }
    }
    //�����
    for(j=0;j<6;j++)
    {
        for(i=0,count=0;i<6;i++,count++)
        {
            if(!(s->sdk[i][j]==0||s->sdk[i][j]==1||s->sdk[i][j]==-1))
            {
                return FALSE;
            }

            else
            {
                if(s->sdk[i][j]==0&&s->sdk[i+1][j]==0&&s->sdk[i+2][j]==0)
                {
                   // printf("��%d %d %d�е�%d�в�����!\n",i,i+1,i+2,j);
                    return FALSE;
                }

                else if(s->sdk[i][j]==1&&s->sdk[i+1][j]==1&&s->sdk[i+2][j]==1)
                {
                   // printf("��%d %d %d�е�%d�в�����!\n",i,i+1,i+2,j);
                    return FALSE;
                }
                else
                {
                    if((count+1)%3==0&&count<11)
                        i-=2;
                    else
                        continue;
                }
            }
        }
    }
    return TRUE;
}

status checkRule2(Sudoku* s)
{
    //Լ����
    int i,j;
    int count0,count1;
    //�����
    for(i=0;i<6;i++)
    {
        for(j=0,count0=0,count1=0;j<6;j++)
        {
            if(s->sdk[i][j]==0)
                count0++;
            else if(s->sdk[i][j]==1)
                count1++;
        }
        //if(count0!=count1&&count0+count1==6)
        if(abs(count0-count1)>2||count0+count1==6)
            if(!(count0==count1&&count0==3))
                return FALSE;

    }
    //�����
    for(j=0;j<6;j++)
    {
        for(i=0,count0=0,count1=0;i<6;i++)
        {
            if(s->sdk[i][j]==0)
                count0++;
            else if(s->sdk[i][j]==1)
                count1++;
        }
       // if(count0!=count1&&count0+count1==6)
        if(abs(count0-count1)>2||count0+count1==6)      //abs�䱣֤�������4��1��2��0�����
            if(!(count0==count1&&count0==3))
                return FALSE;
    }
    return TRUE;
}

status checkRule3(Sudoku* s)
{
    int i,j,k;
    int count;
    //�����
    for(i=0;i<6;i++)    //��i�к͵�j�бȽ�
    {
        for(j=i+1,count=0;j<6;j++)
        {
            for(k=0;k<6;k++)
            {
                if(s->sdk[i][k]==s->sdk[j][k]&&s->sdk[i][k]!=-1&&s->sdk[j][k]!=-1)
                    count++;
            }
            if(count==6)
                return FALSE;
            count=0;        //��j�����j+1�ȽϺ�ǵý�count����
        }

    }
    //�����
    for(j=0;j<6;j++)    //��j�к͵�i�бȽ�
    {

        for(i=j+1,count=0;i<6;i++)
        {
            for(k=0;k<6;k++)
            {
                if(s->sdk[k][j]==s->sdk[k][i]&&s->sdk[k][j]!=-1&&s->sdk[k][i]!=-1)
                    count++;
            }
            if(count==6)        //�ظ��ĸ�����Ϊ�����Ҳ��Ǿ�Ϊ-1
                return FALSE;
            count=0;
        }

    }
    return TRUE;
}


status solveSudoku(Sudoku* s,int* solvecount)
{
    *solvecount=0;
    DFSSSudoku(s,0,0,solvecount);
    return (solvecount>0)?TRUE:FALSE;
}

status DFSSSudoku(Sudoku* s,int x,int y,int* solvecount)
{
    //������ȵķ����ҵ����н�
    int i,j,k;
    Sudoku* ss;
    while(s->sdk[x][y]!=-1&&x<=5)   //�ҵ����ڵ��ĸ���
    {
        y++;
        if(y==6)
        {
            x++;    //����
            y=0;
        }
    }
    if(x>5)     //�Ѿ�ȫ�����
    {
        ss=(Sudoku* )malloc(sizeof(Sudoku));
        for(j=0;j<6;j++)
            for(k=0;k<6;k++)
                ss->sdk[j][k]=s->sdk[j][k];
        ss->next=s->next;
        s->next=ss;
        (*solvecount)++;
        return TRUE;
    }
    //if(x>5)
        //return TRUE;
    for(i=0;i<=1;i++)
    {
        s->sdk[x][y]=i;     //��������
        switch(checkSudoku(s))      //ÿ�γ��Լ���Ƿ�����������Ϸ������Լ��
        {
        case FALSE:
            break;
        case TRUE:
            if(x==5&&y==5)      //����ȫ������
            {
                ss=(Sudoku*)malloc(sizeof(Sudoku));
                for(j=0;j<6;j++)
                    for(k=0;k<6;k++)
                        ss->sdk[j][k]=s->sdk[j][k];
                ss->next=s->next;
                s->next=ss;
                s->sdk[x][y]=-1;
                (*solvecount)++;
                return TRUE;
            }
            else
                (y==5)?DFSSSudoku(s,x+1,0,solvecount):DFSSSudoku(s,x,y+1,solvecount);   //����һ�д���һ�еĵ�һ�����ӿ�ʼ������ֱ�ӽ��ұߵĸ���
            break;
        default:
            break;
        }
    }
    s->sdk[x][y]=-1;        //���ֵ��ָ�Ϊ����״̬
    return FALSE;
}

status printSudoku(Sudoku* s,status color)
{
    //�߿��ӡ��ɫ�����ִ�ӡ��ɫ
    //��colorȡfalse��Ϊ����
    int i,j;
    printf("��ӡ������\n");
    printf("\t ");
    for(j=0;j<11;j++)
        printf("-");
    printf("\n");
    for(i=0;i<6;i++)
    {
        printf("\t");
        printf("|");
        for(j=0;j<6;j++)
        {
            if(s->next)
            {
                if(s->sdk[i][j]==-1)
                    printf("%d%c",s->next->sdk[i][j],(j+1)%3==0?'|':' ');
                else
                    //�ֵı�����ɫ�Ǻ�ɫ��������ɫ����ɫ
                    printf("\033[40;32m%d\033[0m%c",s->sdk[i][j],((j+1)%3==0)?'|':' ');
            }
            else
            {
                if(s->sdk[i][j]==-1)
                    printf(" %c",((j+1)%3==0)?'|':' ');
                else if(color==FALSE)
                    printf("%d%c",s->sdk[i][j],((j+1)%3==0)?'|':' ');
                else
                    printf("\033[40;32m%d\033[0m%c",s->sdk[i][j],((j+1)%3==0)?'|':' ');
            }
        }
        printf("\n");
        if((i+1)%3==0)
        {
            printf("\t");
            printf("|");
            for(j=0;j<11;j++)
                printf("-");
            printf("|\n");
        }
    }
    return OK;
}

status SolveMySukodu(Sudoku* ss,Sudoku* s)
{
    //ss�Ǵ����������̣�s�Ǵ������������
    s->next=NULL;
    printf("�밴���̾�������Ĵ�\n");
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
            scanf("%d",&(s->sdk)[i][j]);
    if(istheSameSudoku(ss,s)==TRUE)
        {
           if(checkSudoku(s)==TRUE)
            {
                printf("���ɹ���");
                return OK;
            }
        }
    else
        printf("�����������������������\n");
    printf("���ʧ�ܣ�\n");
    printf("1. �ٴ����                      2.����ϵͳ\n");
    int op2;
    scanf("%d",&op2);
    switch(op2)
    {
    case 1:
        SolveMySukodu(ss,s);
        break;
    case 2:
        return FALSE;
    }

}

status istheSameSudoku(Sudoku* ss,Sudoku* s)
{
    //������ɵ������̺����������������Ƿ���ͬ
    //��Ҫ�Ƕ�δ�ڵ�Ԫ����м�鼴�����������̵ķ�-1���Ӧ���Ƿ���ͬ
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
    {
        if((ss->sdk)[i][j]!=-1)
           if((ss->sdk)[i][j]==(s->sdk)[i][j])
                ;
            else
                return FALSE;
    }
    return TRUE;
}
