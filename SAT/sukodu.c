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
       printf("内存分配不到！\n");
        return NULL;
    }

    for(i=0;i<6;i++)
        for(j=0;j<6;j++)
            s->sdk[i][j]=-1;
    if(randSudoku(s,0,0)==TRUE)
    {
       printf("\n\n数独终盘完成！-------------------------------------------\n\n");
        //先随机挖掉一个空，此时生成的数独一定是只有一个解的
        x=rand()%6;
        y=rand()%6;
        s->sdk[x][y]=-1;
        //将数独盘大致均分为5个区间，对应到5个难度等级，每个区间大致为7-8个格子
        int max=7*difficult;        //定义可以挖的洞的最多个数
        while(1)
        {
            solveSudoku(s,&i);      //求解数独，并获得解的个数i
            if(i>1)
            {
                flag--;
                s->sdk[x][y]=k;     //新挖掉的s->sdk[x][y]格挖掉之后无法保证是唯一解，恢复这个格子原来的值
                for(ss=s->next;ss;ss=s->next)   //将之前求出的所有不唯一的解全部删除
                {
                    s->next=ss->next;
                    free(ss);
                }
            }
            else        //第一次挖完一个之后，得到唯一解的数独盘，尝试继续挖洞
            {
                flag++;
                if(flag>=max)
                    break;
                do
                {

                   x=rand()%6;
                   y=rand()%6;
                }while(s->sdk[x][y]==-1);       //找到可以挖的格子
                k=s->sdk[x][y];
                s->sdk[x][y]=-1;            //挖洞
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
    int tmp=-2;        //为了防止长时间随机赋值都赋为同一个值
    do
    {
        do
        {
            i++;
            if(i>1)
            {
                s->sdk[x][y]=-1;
                return FALSE;       //无论是0还是1都不能填入
            }
        tmp=s->sdk[x][y];
        s->sdk[x][y]=rand()%2;  //生成0-1
        if(tmp==s->sdk[x][y])
                s->sdk[x][y]=(s->sdk[x][y])?0:1;    //更换

        }while(checkSudoku(s)!=TRUE);
        //printf("第%d行第%d列生成成功！\n",x,y);
        if(x==5&&y==5)
        {
            printf("生成成功！\n");
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
    printf("输入你的数独！\n用-1表示空格，其余填0或1\n");
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
            scanf("%d",&s[i][j]);
    printf("输入保存的路径！\n");
    scanf("%s",path);
    if((fp=fopen(path,"w+"))==NULL)
    {
        printf("文件保存失败！\n");
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
    s->next=NULL;       //开始无法确定解，记得赋值为NULL
    printf("输入加载的文件名称：\n");
    char filename[50];
    scanf("%s",filename);
    FILE* fp=fopen(filename,"rb");
    if(!fp)
    {
        printf("文件打开失败！\n");
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

    //将预填的格子的数字转化为子句加入到子句集中
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
    //printf("加入原数据成功！\n");
    rule1(cnf);
    //printf("约束一完成！\n");
    rule2(cnf);
    //printf("约束二完成！\n");
    rule3(cnf);
    //printf("约束三完成！\n");
    return cnf;
}
status rule1(CNF* cnf)
{
//约束一：每一行，每一列无连续的3个1或0
    //每一行没有连续的3个1或0
    int i,j,k;
    int pos1[3];
    int neg1[3];     //分别存放加入的三个连续单元正，负变量
    int count;
    for(i=0;i<6;i++)
    {
        for(j=0,count=0;j<6;j++,count++)
        {
            k=(i+1)*10+j+1;
            pos1[count%3]=k;
            neg1[count%3]=-k;
            if((count+1)%3==0)       //每取得连续三个变量时，，就要增添子句
            {
                addClause(cnf,3,pos1);
                addClause(cnf,3,neg1);
                j -=2;
            }
        }
    }
    //每一列没有，互换i,j即可
    for(j=0;j<6;j++)
    {
        for(i=0,count=0;i<6;i++,count++)
        {
            k=(i+1)*10+j+1;
            pos1[count%3]=k;
            neg1[count%3]=-k;
            if((count+1)%3==0)       //每取得连续三个变量时，，就要增添子句
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
//约束二，每一行每一列中1和0的个数相同
    //在6阶中，则1，0个数为3个，意味着任选的四个单元至少有一个1或者0
    //对于每一行或者每一列有一共15中选法
    int i,j;
    int select[15][4];     //存放每一种可能
    rule2_select4unit(select);     //只需将第一行的15中可能全部列出来，其余行，列均可由此变化
    int pos_select[15][4];
    int neg_select[15][4];
    int count;
    for(i=0;i<15;i++)
        for(j=0;j<4;j++)
        {
            pos_select[i][j]=select[i][j];
            neg_select[i][j]=-pos_select[i][j];
        }

    //首先考虑行
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
    //列的话其实就是将数组里面的数字进行反转
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
                pos_select[count][i] +=1;       //注意这里，同行每一列之间只相差1
                neg_select[count][i]=-pos_select[count][i];
            }
    }
    return OK;
}

status rule3(CNF* cnf)
{
    //约束三，不存在重复的行和列
    //任意两行不重复，需要引入1+6+6*2=19个附加变元
    //表示约束三供需要19*15*2=570个附加变元
    //共添加2*15*（9*6+7）=1830条附加子句
    //1. 加入第x行第j列和第y行第j列均取1的子句，即1xyj1,以第3行和第5行为例
    int i,j,x,y;
    int line0[6];       //13510，13520...13560
    int line1[6];       //13511，13521...13561
    int line[6];        //1351，1352...1356
    int var1,var2;      //记录检查的两个单元
    for(x=0;x<6;x++)
        for(y=x+1;y<6;y++)
            {
                //将line0赋值并转换为子句加入子句集
                for(j=0;j<6;j++)    //第j个单元
                {
                        var1=(x+1)*10+j+1;       //31
                        var2=(y+1)*10+j+1;      //51
                        line0[j]=0+(j+1)*10+(y+1)*100+(x+1)*1000+1*10000;   //13510
                        line1[j]=1+(j+1)*10+(y+1)*100+(x+1)*1000+1*10000;   //13511
                        //13511=31并51,产生三个子句,13511和13510共加入6条子句
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
                        //1351转换为3条子句
                        int c1[2],c2[2],c3[3];
                        c1[0]=-line1[j];c1[1]=line[j];
                        addClause(cnf,2,c1);
                        c2[0]=-line0[j];c2[1]=line[j];
                        addClause(cnf,2,c2);
                        c3[0]=line0[j];c3[1]=line1[j];c3[2]=-line[j];
                        addClause(cnf,3,c3);
                }
                //135化为范式产生7条子句,第一条有7个变元，后六条均为2个变元，全部加入
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
                addClause(cnf,1,&clause1[6]);       //要将单独的135加入！！！
            }
    //2. 无重复列，将第一位的1换成列标2，2xyj1表示第x列和第y列的第j行都取1；
    for(x=0;x<6;x++)
        for(y=x+1;y<6;y++)
            {
                //将line0赋值并转换为子句加入子句集
                for(j=0;j<6;j++)    //第j个单元
                {
                        var1=(j+1)*10+x+1;       //31       //一定要注意此时j是作为行，所以 应该是j+1在乘以10
                        var2=(j+1)*10+y+1;      //51
                        line0[j]=0+(j+1)*10+(y+1)*100+(x+1)*1000+2*10000;   //23510
                        line1[j]=1+(j+1)*10+(y+1)*100+(x+1)*1000+2*10000;   //23511
                        //23511=31并51,产生三个子句,13511和13510共加入6条子句
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
                        //1351转换为3条子句
                        int c1[2],c2[2],c3[3];
                        c1[0]=-line1[j];c1[1]=line[j];
                        addClause(cnf,2,c1);
                        c2[0]=-line0[j];c2[1]=line[j];
                        addClause(cnf,2,c2);
                        c3[0]=line0[j];c3[1]=line1[j];c3[2]=-line[j];
                        addClause(cnf,3,c3);
                }
                //135化为范式产生7条子句,第一条有7个变元，后六条均为2个变元，全部加入
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
    //首先搞定第一个
    for(i=0;i<10;i++)       //11开头有十种
        s[i][0]=11;
    for( ;i<14;i++)        //12开头有四种,i从10开始，10，11，12，13均为12开头
        s[i][0]=12;
    s[i][0]=13;
    //第二个
    for(i=0;i<6;i++)    //第二个为12的有6个
        s[i][1]=12;
    for( ;i<9;i++)      //第二个为13的有6，7，8
        s[i][1]=13;
    s[i][1]=14;         //第二个为14的仅一个，i=9;
    for(i=10;i<13;i++)     //i=10,11,12取13
        s[i][1]=13;
    for( ;i<15;i++)         //i=13,14取14
        s[i][1]=14;
    //第三个
    s[0][2]=s[1][2]=s[2][2]=13;
    s[3][2]=s[4][2]=s[6][2]=s[7][2]=s[10][2]=s[11][2]=14;
    s[5][2]=s[8][2]=s[9][2]=s[13][2]=s[14][2]=s[12][2]=15;
    //第四个
    s[0][3]=14;
    s[1][3]=s[3][3]=s[6][3]=s[10][3]=15;
    s[2][3]=s[4][3]=s[5][3]=s[7][3]=s[8][3]=s[9][3]=s[11][3]=s[12][3]=s[13][3]=s[14][3]=16;
    return OK;
}

status checkSudoku(Sudoku* s)
{
    if(checkRule1(s)==TRUE)
    {
        //printf("约束一满足！\n");
        if(checkRule2(s)==TRUE)
        {
            //printf("约束二满足！\n");
            if(checkRule3(s)==TRUE)
            {
               // printf("约束三满足！\n");
                return TRUE;
            }

            else
            {
            //    printf("约束三不满足！\n");
                return FALSE;
            }

        }
        else
        {
          //  printf("约束二不满足！\n");
            return FALSE;
        }

    }
    else
    {
       // printf("约束一不满足！\n");
        return FALSE;
    }

}

status checkRule1(Sudoku* s)
{
//检查数独是否合乎约束
//约束一
    //检查行
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
                    //printf("第%d %d %d列第%d行不满足！\n",j,j+1,j+2,i);
                    return FALSE;
                }

                else if(s->sdk[i][j]==1&&s->sdk[i][j+1]==1&&s->sdk[i][j+2]==1)
                {
                   // printf("第%d %d %d列第%d行不满足！\n",j,j+1,j+2,i);
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
    //检查列
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
                   // printf("第%d %d %d行第%d列不满足!\n",i,i+1,i+2,j);
                    return FALSE;
                }

                else if(s->sdk[i][j]==1&&s->sdk[i+1][j]==1&&s->sdk[i+2][j]==1)
                {
                   // printf("第%d %d %d行第%d列不满足!\n",i,i+1,i+2,j);
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
    //约束二
    int i,j;
    int count0,count1;
    //检查行
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
    //检查列
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
        if(abs(count0-count1)>2||count0+count1==6)      //abs句保证不会出现4个1，2个0的情况
            if(!(count0==count1&&count0==3))
                return FALSE;
    }
    return TRUE;
}

status checkRule3(Sudoku* s)
{
    int i,j,k;
    int count;
    //检查行
    for(i=0;i<6;i++)    //第i行和第j行比较
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
            count=0;        //和j比完和j+1比较后记得将count清零
        }

    }
    //检查列
    for(j=0;j<6;j++)    //第j列和第i列比较
    {

        for(i=j+1,count=0;i<6;i++)
        {
            for(k=0;k<6;k++)
            {
                if(s->sdk[k][j]==s->sdk[k][i]&&s->sdk[k][j]!=-1&&s->sdk[k][i]!=-1)
                    count++;
            }
            if(count==6)        //重复的格子数为满格且不是均为-1
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
    //深度优先的方法找到所有解
    int i,j,k;
    Sudoku* ss;
    while(s->sdk[x][y]!=-1&&x<=5)   //找到被挖掉的格子
    {
        y++;
        if(y==6)
        {
            x++;    //换行
            y=0;
        }
    }
    if(x>5)     //已经全部填好
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
        s->sdk[x][y]=i;     //尝试填数
        switch(checkSudoku(s))      //每次尝试检查是否满足数独游戏的三个约束
        {
        case FALSE:
            break;
        case TRUE:
            if(x==5&&y==5)      //数独全部填完
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
                (y==5)?DFSSSudoku(s,x+1,0,solvecount):DFSSSudoku(s,x,y+1,solvecount);   //填完一行从下一行的第一个格子开始，否则直接接右边的格子
            break;
        default:
            break;
        }
    }
    s->sdk[x][y]=-1;        //求得值后恢复为待填状态
    return FALSE;
}

status printSudoku(Sudoku* s,status color)
{
    //边框打印白色，数字打印绿色
    //当color取false是为数字
    int i,j;
    printf("打印数独！\n");
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
                    //字的背景颜色是黑色，字体颜色是绿色
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
    //ss是待求解的数独盘，s是待输入的数独盘
    s->next=NULL;
    printf("请按照盘局输入你的答案\n");
    for(int i=0;i<6;i++)
        for(int j=0;j<6;j++)
            scanf("%d",&(s->sdk)[i][j]);
    if(istheSameSudoku(ss,s)==TRUE)
        {
           if(checkSudoku(s)==TRUE)
            {
                printf("求解成功！");
                return OK;
            }
        }
    else
        printf("输入的数独不是求解的数独！\n");
    printf("求解失败！\n");
    printf("1. 再次求解                      2.求助系统\n");
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
    //检查生成的数独盘和输入求解的数独盘是否相同
    //主要是对未挖单元格进行检查即对生成数独盘的非-1格对应的是否相同
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
