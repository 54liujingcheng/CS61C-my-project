#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/

/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fileds of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix(matrix **mat, int rows, int cols) {
	if(rows<=0||cols<=0)return -1;
	matrix* res=(matrix*)malloc(sizeof(matrix));
	if(res==NULL)return -1;
	res->ref_cnt=1;
	res->rows=rows;
	res->cols=cols;
	res->is_1d=((rows==1)||(cols==1));
	res->parent=NULL;
	res->data=(double**)malloc(sizeof(double*)*rows);
	if(res->data==NULL){
		free(res);
		return -1;
	}
    for(int i=0;i<rows;i++){
    	res->data[i]=(double*)malloc(sizeof(double)*cols);
    	if(res->data[i]==NULL){
    		for(int j=0;j<i;j++){
    			free(res->data[j]);
			}
			free(res->data);
			free(res);
			return -1;
		}
    	for(int j=0;j<cols;j++){
    		res->data[i][j]=0;
		}
	}
	*mat=res;
	return 0;
}//double pointer means to create a new mat and copy this new pointer
//into where **mat point at

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset,
                        int rows, int cols) {
    
    if(col_offset<0||row_offset<0||rows<=0||cols<=0)return -1;
    if(row_offset>=from->rows||col_offset>=from->cols)return -1;
    if(rows+row_offset>from->rows||cols+col_offset>from->cols)return -1;
	
    matrix* res=(matrix*)malloc(sizeof(matrix));//remember
	
    if(res==NULL)return -1;
    res->ref_cnt=1;
    res->rows=rows;
    res->cols=cols;
    res->is_1d=((rows==1)||(cols==1));
    res->parent=from;
    res->data=(double**)malloc(sizeof(double*)*rows);
    if(res->data==NULL){
    	free(res);
    	return -1;
	}
    for(int i=0;i<rows;i++){
    	res->data[i]=(from->data[i+row_offset])+col_offset;//no sizeof(double)*
	}
	*mat=res;
	from->ref_cnt++;
	return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    if(mat==NULL)return ;
    if(mat->parent!=NULL){
    	mat->parent->ref_cnt--;
    	if(!mat->parent->ref_cnt){
    		mat->parent->ref_cnt++;
    		deallocate_matrix(mat->parent);
		}
		free(mat);
	}
	else{
		mat->ref_cnt--;
		if(!mat->ref_cnt){
			for(int i=0;i<mat->rows;i++){
				free(mat->data[i]);
			}
			free(mat->data);
			free(mat);
			//remember,free struct wont free where pt point at,only free pt itself
		}
	}
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double get(matrix *mat, int row, int col) {
    return mat->data[row][col];
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void set(matrix *mat, int row, int col, double val) {
    mat->data[row][col]=val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
	int totals=mat->rows*mat->cols;
	#pragma omp parallel for if(totals>=5000)
    for(int i=0;i<mat->rows;i++){
    	for(int j=0;j<mat->cols/4*4;j+=4){
    		_mm256_storeu_pd(&mat->data[i][j],_mm256_set1_pd(val));
		}
		for(int j=mat->cols/4*4;j<mat->cols;j++){
			mat->data[i][j]=val;
		}
	}
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
	int rows=mat1->rows,cols=mat1->cols;
	if(rows!=result->rows||rows!=mat2->rows||cols!=result->cols||cols!=mat2->cols)return -1;
	int totals=mat1->rows*mat1->cols;
	#pragma omp parallel for if(totals>=5000)
    for(int i=0;i<rows;i++){
    	for(int j=0;j<cols/4*4;j+=4){
    		result->data[i][j]=mat1->data[i][j]-mat2->data[i][j];
    		__m256d vec1=_mm256_loadu_pd(mat1->data[i]+j);
    		__m256d vec2=_mm256_loadu_pd(mat2->data[i]+j);
    		__m256d vec_res=_mm256_add_pd(vec1,vec2);
    		_mm256_storeu_pd(&(result->data[i][j]),vec_res);
		}
		for(int j=cols/4*4;j<cols;j++){
			result->data[i][j]=mat1->data[i][j]+mat2->data[i][j];
		}
	}
	return 0;
}

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    int rows=mat1->rows,cols=mat1->cols;
	if(rows!=result->rows||rows!=mat2->rows||cols!=result->cols||cols!=mat2->cols)return -1;
	int totals=mat1->rows*mat1->cols;
	#pragma omp parallel for if(totals>=5000)
    for(int i=0;i<rows;i++){
    	for(int j=0;j<cols/4*4;j+=4){
    		result->data[i][j]=mat1->data[i][j]-mat2->data[i][j];
    		__m256d vec1=_mm256_loadu_pd(mat1->data[i]+j);
    		__m256d vec2=_mm256_loadu_pd(mat2->data[i]+j);
    		__m256d vec_res=_mm256_sub_pd(vec1,vec2);
    		_mm256_storeu_pd(&(result->data[i][j]),vec_res);
		}
		for(int j=cols/4*4;j<cols;j++){
			result->data[i][j]=mat1->data[i][j]-mat2->data[i][j];
		}
	}
	return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    int rows=mat1->rows,cols=mat2->cols;
    if(rows!=result->rows||cols!=result->cols||mat1->cols!=mat2->rows)return -1;
    int totals=mat1->rows*mat2->cols;
    matrix *mat2_t = NULL;
    allocate_matrix(&mat2_t, mat2->cols, mat2->rows);
    for (int i = 0; i < mat2->rows; i++) {
        for (int j = 0; j < mat2->cols; j++) {
            mat2_t->data[j][i] = mat2->data[i][j];
        }
    }
	#pragma omp parallel for if(totals>=5000)
    for(int i=0;i<rows;i++){
    	for(int j=0;j<cols;j++){
    		result->data[i][j]=0;
    		__m256d vec_res=_mm256_setzero_pd();
    		for(int k=0;k<mat1->cols/4*4;k+=4){
    			__m256d vec1=_mm256_loadu_pd(mat1->data[i]+k);
    			__m256d vec2=_mm256_loadu_pd(mat2_t->data[j]+k);
    			vec_res=_mm256_fmadd_pd(vec1,vec2,vec_res);
			}
			for(int k=mat1->cols/4*4;k<mat1->cols;k++){
				result->data[i][j]+=mat1->data[i][k]*mat2->data[k][j];
			}
			double tmp_arr[4];
			_mm256_storeu_pd(tmp_arr,vec_res);
			result->data[i][j]+=tmp_arr[0]+tmp_arr[1]+tmp_arr[2]+tmp_arr[3];
		}
	}
	deallocate_matrix(mat2_t);
	return 0;
}

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise multiplication.
 */
int pow_matrix(matrix *result, matrix *mat, int pow) {
	if(pow<0||result->rows!=result->cols||result->rows!=mat->rows||result->cols!=mat->cols)return -1;
    for(int i=0;i<result->rows;i++){
    	for(int j=0;j<result->cols;j++){
    		if(i==j)result->data[i][j]=1;
    		else result->data[i][j]=0;
		}
	}
	matrix* tmp=NULL,*base=NULL;
	if(allocate_matrix(&tmp,result->rows,result->cols)!=0){
		return -1;
	}
	if(allocate_matrix(&base,result->rows,result->cols)!=0){
		deallocate_matrix(tmp);
		return -1;
	}
	for(int i=0;i<result->rows;i++){
    	for(int j=0;j<result->cols/4*4;j+=4){
    		__m256d vec=_mm256_loadu_pd(&mat->data[i][j]);
			_mm256_storeu_pd(&base->data[i][j],vec);
		}
		for(int j=result->cols/4*4;j<result->cols;j++){
			base->data[i][j]=mat->data[i][j];
		}
	}
	while(pow){
		if(pow&1){
			for(int j=0;j<result->rows;j++){
				for(int k=0;k<result->cols/4*4;k+=4){
					__m256d vec=_mm256_loadu_pd(&result->data[j][k]);
					_mm256_storeu_pd(&tmp->data[j][k],vec);
				}
				for(int k=result->cols/4*4;k<result->cols;k++){
					tmp->data[j][k]=result->data[j][k];
				}
			}
			int rtn=mul_matrix(result,tmp,base);
			if(rtn==-1){
				deallocate_matrix(tmp);
				deallocate_matrix(base); 
				return -1;
			}
		}
		pow>>=1;
		if(pow==0)break;
		for(int j=0;j<result->rows;j++){
			for(int k=0;k<result->cols/4*4;k+=4){
				__m256d vec=_mm256_loadu_pd(&base->data[j][k]);
				_mm256_storeu_pd(&tmp->data[j][k],vec);
			}
			for(int k=result->cols/4*4;k<result->cols;k++){
				tmp->data[j][k]=base->data[j][k];
			}
		}
		int rtn=mul_matrix(base,tmp,tmp);
		if(rtn==-1){
			deallocate_matrix(tmp);
			deallocate_matrix(base);
			return -1;
		}
	}
	deallocate_matrix(tmp);
	deallocate_matrix(base);
	return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    if(result->rows!=mat->rows||result->cols!=mat->cols)return -1;
    int totals=mat->rows*mat->cols;
	#pragma omp parallel for if(totals>=5000)
    for(int i=0;i<mat->rows;i++){
    	for(int j=0;j<mat->cols;j++){
    		result->data[i][j]=-mat->data[i][j];
		}
	}
	return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    if(result->rows!=mat->rows||result->cols!=mat->cols)return -1;
    int totals=mat->rows*mat->cols;
	#pragma omp parallel for if(totals>=5000)
    for(int i=0;i<mat->rows;i++){
    	for(int j=0;j<mat->cols;j++){
    		result->data[i][j]=fabs(mat->data[i][j]);
		}
	}
	return 0;
}
