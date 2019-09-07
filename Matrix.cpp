#include"Matrix.h"

/**********************************************
 *@brief  Matrix_Add   矩阵加法 m3=m1+m2
 
 *@param  m1   I   输入矩阵m1
 *@param  m2   I   输入矩阵m2
 *@param  m3   O   输出矩阵m3
 **********************************************/
void Matrix_Add(const  struct Matrix* m1,const struct Matrix *m2, struct Matrix *m3)
{
    if(m1->Col!=m2->Col || m1->Row!=m2->Row) {printf( "Error dimension in MatrixInv!\n",strerror(5));}
    for (int i=0;i<m1->Row;i++)
    {
        for (int j=0;j<m1->Col;j++)
        {
            int l=i*m1->Col+j;
            m3->Val[l]=m1->Val[l]+m2->Val[l];
        }
    }
    m3->Col=m1->Col;
    m3->Row=m1->Row;
}

/**********************************************
 *@brief  Matrix_Subtract   矩阵减法 m3=m1-m2
 
 *@param  m1   I   输入矩阵m1
 *@param  m2   I   输入矩阵m2
 *@param  m3   O   输出矩阵m3
 **********************************************/
void Matrix_Subtract(const struct Matrix* m1,const struct Matrix *m2, struct Matrix *m3)  //m3=m1+m2
{
    if(m1->Col!=m2->Col || m1->Row!=m2->Row) {printf( "Error dimension in MatrixInv!\n",strerror(5)); return ;}
    for (int i=0;i<m1->Row;i++)
    {
        for (int j=0;j<m1->Col;j++)
        {
            int l=i*m1->Col+j;
            m3->Val[l]=m1->Val[l]-m2->Val[l];
        }
    }
    m3->Col=m1->Col;
    m3->Row=m1->Row;
}


/**********************************************
 *@brief  Matrix_Mutiply   矩阵乘法 m3=m1*m2
 
 *@param  m1   I   输入矩阵m1
 *@param  m2   I   输入矩阵m2
 *@param  m3   O   输出矩阵m3
 **********************************************/
void Matrix_Mutiply(const struct Matrix* m1,const struct Matrix *m2,struct Matrix *m3)
{
    m3->Row=m1->Row;
    m3->Col=m2->Col;
	if (m1->Col != m2->Row) {
		//printf( "Error dimension in MatrixInv!\n",strerror(5));
	};
    for (int i=0;i<m1->Row;i++)
    { 
        for (int j=0;j<m2->Col;j++)
        {
            int l=i*m3->Col+j;
            //m3->Val[i][j]=0;
            m3->Val[l]=0;
            for (int k=0; k <m1->Col;k++)
            {
                int l1=i*m1->Col+k;
                int l2=k*m2->Col+j;
              //  m3->Val[l] += m1->Val[i][k] * m2->Val[k][j];
                m3->Val[l] +=( m1->Val[l1]*m2->Val[l2] ) ;
            }
        }
    }
}



/**********************************************
 *@brief  Matrix_Transpose   矩阵转置 m2=m1_T
 
 *@param  m1   I   输入矩阵m1
 *@param  m2   O   输出矩阵m2
 **********************************************/
void Matrix_Transpose(const struct Matrix* m1,struct Matrix* m2)
{
    m2->Col=m1->Row;
    m2->Row=m1->Col;
    for(int i=0;i<m1->Row;i++)
    {
        for(int j=0;j<m1->Col;j++)
        {
            int l1=i*m1->Col+j;
            int l2=j*m1->Row+i;
            m2->Val[l2]=m1->Val[l1];
        }
    }
}


/**********************************************
 *@brief  Matrix_Transpose   矩阵求逆 m2=inv(m1)
 
 *@param  m1   I   输入矩阵m1
 *@param  m2   O   输出矩阵m2  0 no  1 yes
 **********************************************/
int Matrix_Inv(const struct Matrix* m1,struct Matrix* m2)
{
    if(m1->Col!=m1->Row) {
		//printf( "Error dimension in MatrixInv!\n",strerror(5));
	}
    int i,j,k,l,u,v,is[MAX_SIZE],js[MAX_SIZE];   ///< matrix dimension <= 10
    double d, p;
    int n=m1->Col;
    if( n<= 0 )
    {
//        printf( "Error dimension in MatrixInv!\n",strerror(5));
        //exit(EXIT_FAILURE);
        return 0;
    }
    
    double b[MAX_SIZE*MAX_SIZE]={0};
    
    /// 将输入矩阵赋值给输出矩阵b，下面对b矩阵求逆，a矩阵不变
    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++)
        {
            b[i*n+j]=m1->Val[i*n+j];
            //m2->Val[i][j]=m1->Val[i][j];
        }
    }
    
    for(k=0;k<n;k++)
    {
        d=0.0;
        for(i=k;i<n;i++)   ///< 查找右下角方阵中主元素的位置
        {
            for(j=k;j<n;j++)
            {
                l=n*i+j;
                p = fabs(b[l]);
                if(p>d)
                {
                    d=p;
                    is[k]=i;
                    js[k]=j;
                }
            }
        }
        if(d<DBL_EPSILON)   ///< 主元素接近于0，矩阵不可逆
        {
         //   printf("Divided by 0 in MatrixInv!\n",strerror(5));
            // exit(EXIT_FAILURE);
            return 0;
        }
        if( is[k]!=k )  ///< 对主元素所在的行与右下角方阵的首行进行调换
        {
            for(j=0;j<n;j++)
            {
                u=k*n+j;
                v=is[k]*n+j;
                p=b[u];
                b[u]=b[v];
                b[v]=p;
            }
        }
        
        if( js[k]!=k )  ///< 对主元素所在的列与右下角方阵的首列进行调换
        {
            for( i=0; i<n; i++ )
            {
                u=i*n+k;
                v=i*n+js[k];
                p=b[u];
                b[u]=b[v];
                b[v]=p;
            }
        }
        
        l=k*n+k;
        b[l]=1.0/b[l];  ///< 初等行变换
        for( j=0; j<n; j++ )
        {
            if( j!=k )
            {
                u=k*n+j;
                b[u]=b[u]*b[l];
            }
        }
        for(i=0;i<n; i++)
        {
            if(i!=k)
            {
                for(j=0; j<n; j++ )
                {
                    if( j!=k )
                    {
                        u=i*n+j;
                        b[u]=b[u]-b[i*n+k]*b[k*n+j];
                    }
                }
            }
        }
        for(i=0;i<n;i++)
        {
            if(i!=k)
            {
                u=i*n+k;
                b[u]=-b[u]*b[l];
            }
        }
    }
    
    for(k=n-1;k>=0;k--)  ///<  将上面的行列调换重新恢复
    {
        if(js[k]!=k)
        {
            for(j=0;j<n;j++)
            {
                u=k*n+j;
                v=js[k]*n+j;
                p=b[u];
                b[u]=b[v];
                b[v]=p;
            }
        }
        if(is[k]!=k)
        {
            for(i=0;i<n;i++)
            {
                u=i*n+k;
                v=is[k]+i*n;
                p=b[u];
                b[u]=b[v];
                b[v]=p;
            }
        }
    }
    for (int i=0; i < n; i++)                 ///< 给b赋Val的值
    {
        for(int j=0;j<n;j++)
        {
            m2->Val[i*n+j]=b[i*n+j];
        }
    }
    m2->Col=n;
    m2->Row=n;
    return 1;
}



