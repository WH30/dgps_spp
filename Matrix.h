#ifndef Matrix_h
#define Matrix_h
#ifndef MAX_SIZE   ///<æÿ’Û◊Ó¥ÛŒ¨∂»
#define MAX_SIZE 50  
#endif

#ifndef DBL_EPSILON   ///<æÿ’Û«ÛƒÊ0
#define DBL_EPSILON 0.001  
#endif

#include<stdio.h>
#include<string.h>
#include<math.h>
 
/**************************
 *@brief  Matrix  æÿ’Û
 **************************/
struct Matrix
{
    int Row;  ///< ––
    int Col;  ///< ¡–
    double Val[MAX_SIZE*MAX_SIZE];
};

/**********************************************
 *@brief  Matrix_Add   æÿ’Ûº”∑® m3=m1+m2
 
 *@param  m1   I    ‰»Îæÿ’Ûm1
 *@param  m2   I    ‰»Îæÿ’Ûm2
 *@param  m3   O    ‰≥ˆæÿ’Ûm3
 **********************************************/
void Matrix_Add(const struct Matrix* m1,const struct Matrix *m2,struct Matrix *m3);

/**********************************************
 *@brief  Matrix_Subtract   æÿ’Ûºı∑® m3=m1-m2
 
 *@param  m1   I    ‰»Îæÿ’Ûm1
 *@param  m2   I    ‰»Îæÿ’Ûm2
 *@param  m3   O    ‰≥ˆæÿ’Ûm3
 **********************************************/
void Matrix_Subtract(const struct Matrix* m1,const struct Matrix *m2,struct Matrix *m3);  

/**********************************************
 *@brief  Matrix_Mutiply   æÿ’Û≥À∑® m3=m1*m2
 
 *@param  m1   I    ‰»Îæÿ’Ûm1
 *@param  m2   I    ‰»Îæÿ’Ûm2
 *@param  m3   O    ‰≥ˆæÿ’Ûm3
 **********************************************/
void Matrix_Mutiply(const struct Matrix* m1,const struct Matrix *m2, struct Matrix *m3);

/**********************************************
 *@brief  Matrix_Transpose   æÿ’Û◊™÷√ m2=m1_T
 
 *@param  m1   I    ‰»Îæÿ’Ûm1
 *@param  m2   O    ‰≥ˆæÿ’Ûm2
 **********************************************/
void Matrix_Transpose(const struct Matrix* m1, struct Matrix* m2);

/**********************************************
 *@brief  Matrix_Transpose   æÿ’Û«ÛƒÊ m2=inv(m1)
 
 *@param  m1   I    ‰»Îæÿ’Ûm1
 *@param  m2   O    ‰≥ˆæÿ’Ûm2
 **********************************************/
int Matrix_Inv(const struct Matrix* m1,struct Matrix* m2);


#endif


