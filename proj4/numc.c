#include "numc.h"
#include <structmember.h>

PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
    if (rows == 1 || cols == 1) {
        PyObject *val = PyLong_FromLong(rows * cols); // Ref = 1
        PyObject *tuple = PyTuple_Pack(1, val);       // Tuple Ref=1, Val Ref=2
        Py_DECREF(val);                               // Val Ref=1 (Tuple ¶ÀÕ¼)
        return tuple;
    } else {
        PyObject *r = PyLong_FromLong(rows);
        PyObject *c = PyLong_FromLong(cols);
        PyObject *tuple = PyTuple_Pack(2, r, c);
        Py_DECREF(r);
        Py_DECREF(c);
        return tuple;
    }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    matrix* rtn;
    if(!PyObject_TypeCheck(args, &Matrix61cType)){
    	PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
	}
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    if(allocate_matrix(&rtn,rows,cols)!=0){
        return NULL;
	}
    int tmp=add_matrix(rtn,self->mat,((Matrix61c*)(args))->mat);
	if(tmp!=0){
		PyErr_SetString(PyExc_ValueError, "Values not valid");
		deallocate_matrix(rtn);
		return NULL;
	}
	Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
	if(ans==NULL){
		deallocate_matrix(rtn);
		return NULL;
	}
	ans->shape=get_shape(rtn->rows,rtn->cols);
	ans->mat=rtn;
	return (PyObject*)ans;
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    matrix* rtn;
    if(!PyObject_TypeCheck(args, &Matrix61cType)){
    	PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
	}
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    if(allocate_matrix(&rtn,rows,cols)!=0){
        return NULL;
	}
    int tmp=sub_matrix(rtn,self->mat,((Matrix61c*)(args))->mat);
	if(tmp!=0){
		PyErr_SetString(PyExc_ValueError, "Values not valid");
		deallocate_matrix(rtn);
		return NULL;
	}
	Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
	if(ans==NULL){
		deallocate_matrix(rtn);
		return NULL;
	}
	ans->shape=get_shape(rtn->rows,rtn->cols);
	ans->mat=rtn;
	return (PyObject*)ans;
}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    matrix* rtn;
    if(!PyObject_TypeCheck(args, &Matrix61cType)){
    	PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
	}
    int rows = self->mat->rows;
    int cols = ((Matrix61c*)(args))->mat->cols;
    if(allocate_matrix(&rtn,rows,cols)!=0){
        return NULL;
	}
    int tmp=mul_matrix(rtn,self->mat,((Matrix61c*)(args))->mat);
	if(tmp!=0){
		PyErr_SetString(PyExc_ValueError, "Values not valid");
		deallocate_matrix(rtn);
		return NULL;
	}
	Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
	if(ans==NULL){
		deallocate_matrix(rtn);
		return NULL;
	}
	ans->shape=get_shape(rtn->rows,rtn->cols);
	ans->mat=rtn;
	return (PyObject*)ans;
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    matrix* rtn;
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    if(allocate_matrix(&rtn,rows,cols)!=0){
        return NULL;
	}
    int tmp=neg_matrix(rtn,self->mat);
	if(tmp!=0){
		PyErr_SetString(PyExc_ValueError, "Values not valid");
		deallocate_matrix(rtn);
		return NULL;
	}
	Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
	if(ans==NULL){
		deallocate_matrix(rtn);
		return NULL;
	}
	ans->shape=get_shape(rtn->rows,rtn->cols);
	ans->mat=rtn;
	return (PyObject*)ans;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    matrix* rtn;
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    if(allocate_matrix(&rtn,rows,cols)!=0){
        return NULL;
	}
    int tmp=abs_matrix(rtn,self->mat);
	if(tmp!=0){
		PyErr_SetString(PyExc_ValueError, "Values not valid");
		deallocate_matrix(rtn);
		return NULL;
	}
	Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
	if(ans==NULL){
		deallocate_matrix(rtn);
		return NULL;
	}
	ans->shape=get_shape(rtn->rows,rtn->cols);
	ans->mat=rtn;
	return (PyObject*)ans;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    matrix* rtn;
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    if(!PyLong_Check(pow)){
    	PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
	}
    if(allocate_matrix(&rtn,rows,cols)!=0){
        return NULL;
	}
    int tmp=pow_matrix(rtn,self->mat,PyLong_AsLong(pow));
	if(tmp!=0){
		PyErr_SetString(PyExc_ValueError, "Values not valid");
		deallocate_matrix(rtn);
		return NULL;
	}
	Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
	if(ans==NULL){
		deallocate_matrix(rtn);
		return NULL;
	}
	ans->shape=get_shape(rtn->rows,rtn->cols);
	ans->mat=rtn;
	return (PyObject*)ans;
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */

PyNumberMethods Matrix61c_as_number = {
    .nb_add      = (binaryfunc)Matrix61c_add,
    .nb_subtract = (binaryfunc)Matrix61c_sub,
    .nb_multiply = (binaryfunc)Matrix61c_multiply,
    .nb_negative = (unaryfunc)Matrix61c_neg,
    .nb_absolute = (unaryfunc)Matrix61c_abs,
    .nb_power    = (ternaryfunc)Matrix61c_pow,
};

/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
	PyObject *rows,*cols,*val;
    if (PyArg_UnpackTuple(args, "args",3, 3, &rows, &cols,&val)) {
        if ((PyFloat_Check(val)||PyLong_Check(val)) && PyLong_Check(rows) && PyLong_Check(cols)) {
            long rows_c=PyLong_AsLong(rows),cols_c=PyLong_AsLong(cols);
			double val_c=PyFloat_AsDouble(val);
            if(rows_c<0||rows_c>=self->mat->rows||cols_c<0||cols_c>=self->mat->cols){
            	PyErr_SetString(PyExc_ValueError, "Values not valid");
            	return NULL;
			}
			else{
				set(self->mat,rows_c,cols_c,val_c);
			}
        }
        else{
        	PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        	return NULL;
		}
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    PyObject *rows,*cols;
    if (PyArg_UnpackTuple(args, "args",2, 2, &rows, &cols)) {
        if (PyLong_Check(rows) && PyLong_Check(cols)) {
            long rows_c=PyLong_AsLong(rows),cols_c=PyLong_AsLong(cols);
            if(rows_c<0||rows_c>=self->mat->rows||cols_c<0||cols_c>=self->mat->cols){
            	PyErr_SetString(PyExc_ValueError, "Values not valid");
            	return NULL;
			}
			else{
				return PyFloat_FromDouble(get(self->mat,rows_c,cols_c));
			}
        }
        else{
        	PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        	return NULL;
		}
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    {"set", (PyCFunction)Matrix61c_set_value, METH_VARARGS, "set(rows,cols,val),set mal[rows][cols]->val"},
    {"get", (PyCFunction)Matrix61c_get_value, METH_VARARGS, "get(rows,cols),get mal[rows][cols]"},
    {NULL, NULL, 0, NULL}
};

/* INDEXING */

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
static matrix *Matrix61c_helper(Matrix61c* self,PyObject* key,int* is_float){
	matrix* res;
	*is_float=0;
	int rows=self->mat->rows,cols=self->mat->cols;
	if(self->mat->is_1d){
		if(PyLong_Check(key)){
			int index=PyLong_AsLong(key);
			if(allocate_matrix_ref(&res,self->mat,index/cols,index%cols,1,1)<0){
				PyErr_SetString(PyExc_IndexError, "Index not valid");
            	return NULL;
			}
			*is_float=1;
			return res;
		}
		else if(PySlice_Check(key)){
			Py_ssize_t start, stop, step, slicelength;
			if(PySlice_GetIndicesEx(key, rows*cols, &start, &stop, &step, &slicelength)<0){
				PyErr_SetString(PyExc_TypeError, "Invalid arguments");
    			return NULL;
			}
			if(step!=1||slicelength<1){
				PyErr_SetString(PyExc_ValueError, "Values not valid");
    			return NULL;
			}
			if(allocate_matrix_ref(&res,self->mat,(int)start/cols,(int)start%cols,(rows==1?1:(int)slicelength),(cols==1?1:(int)slicelength))<0){
				PyErr_SetString(PyExc_IndexError, "Index not valid");
        		return NULL;
			}
			*is_float=0;
			return res;
		}
		else{
			PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        	return NULL;
		}
	}
	else{
		if(PyLong_Check(key)){
			if(allocate_matrix_ref(&res,self->mat,PyLong_AsLong(key),0,1,self->mat->cols)<0){
				PyErr_SetString(PyExc_IndexError, "Index not valid");
            	return NULL;
			}
			if(res->rows==1&&res->cols==1)*is_float=1;
			return res;
		}
		else if(PySlice_Check(key)){
			Py_ssize_t start, stop, step, slicelength;
			Py_ssize_t total_length = self->mat->rows;
			if(PySlice_GetIndicesEx(key, total_length, &start, &stop, &step, &slicelength)<0){
				PyErr_SetString(PyExc_TypeError, "Invalid arguments");
    			return NULL;
			}
			if(step!=1||slicelength<1){
				PyErr_SetString(PyExc_ValueError, "Values not valid");
    			return NULL;
			}
			if(allocate_matrix_ref(&res,self->mat,(int)start,0,(int)slicelength,self->mat->cols)<0){
				PyErr_SetString(PyExc_IndexError, "Index not valid");
        		return NULL;
			}
			if(res->rows==1&&res->cols==1)*is_float=1;
			return res;
		}
		else if (PyTuple_Check(key)){
			    if (PyTuple_Size(key) != 2) {
			        PyErr_SetString(PyExc_TypeError, "Indexing requires a tuple of size 2.");
			        return NULL;
			    }
			    PyObject *row_key = PyTuple_GetItem(key, 0); 
			    PyObject *col_key = PyTuple_GetItem(key, 1); 
			    if (PyLong_Check(row_key)) {
			        long row_index = PyLong_AsLong(row_key);
				        if (PyLong_Check(col_key)) {
					        long col_index = PyLong_AsLong(col_key);
					        if(allocate_matrix_ref(&res,self->mat,row_index,col_index,1,1)){
					        	PyErr_SetString(PyExc_IndexError, "Index Error.");
				        		return NULL;
							}
					        *is_float=1;
					        return res;
				    	} 
						else if (PySlice_Check(col_key)) {
					        Py_ssize_t col_start, col_stop, col_step, col_slicelength;
					        PySlice_GetIndicesEx(col_key, self->mat->cols, &col_start, &col_stop, &col_step, &col_slicelength);
					        if(col_step!=1||col_slicelength<1){
								PyErr_SetString(PyExc_ValueError, "Values not valid");
				    			return NULL;
							}
					        if(allocate_matrix_ref(&res,self->mat,row_index,(int)col_start,1,(int)col_slicelength)<0){
								PyErr_SetString(PyExc_IndexError, "Index not valid");
				        		return NULL;
							}
							if(res->rows==1&&res->cols==1)*is_float=1;
							return res;
				    	} 
						else {
					        PyErr_SetString(PyExc_TypeError, "Column index must be an integer or a slice.");
					        return NULL;
				    	}
			    } 
				else if (PySlice_Check(row_key)) {
			        Py_ssize_t row_start, row_stop, row_step, row_slicelength;
			        PySlice_GetIndicesEx(row_key, self->mat->rows, &row_start, &row_stop, &row_step, &row_slicelength);
			        if(row_step!=1||row_slicelength<1){
						PyErr_SetString(PyExc_ValueError, "Values not valid");
		    			return NULL;
					}
		        	if (PyLong_Check(col_key)) {
				        long col_index = PyLong_AsLong(col_key);
				        if(allocate_matrix_ref(&res,self->mat,(int)row_start,col_index,(int)row_slicelength,1)<0){
				        	PyErr_SetString(PyExc_IndexError, "Index Error.");
		        			return NULL;
						}
						if(res->rows==1&&res->cols==1)*is_float=1;
						return res;
				    }
					else if (PySlice_Check(col_key)) {
				        Py_ssize_t col_start, col_stop, col_step, col_slicelength;
				        PySlice_GetIndicesEx(col_key, self->mat->cols, &col_start, &col_stop, &col_step, &col_slicelength);
				        if(col_step!=1||col_slicelength<1){
							PyErr_SetString(PyExc_ValueError, "Values not valid");
			    			return NULL;
						}
		    			if(allocate_matrix_ref(&res,self->mat,(int)row_start,(int)col_start,(int)row_slicelength,(int)col_slicelength)<0){
				        	PyErr_SetString(PyExc_IndexError, "Index Error.");
		        			return NULL;
						}
				        if(res->rows==1&&res->cols==1)*is_float=1;
						return res;
				    } 
					else {
				        PyErr_SetString(PyExc_TypeError, "Column index must be an integer or a slice.");
				        return NULL;
				    }
		    	} 
				else {
			        PyErr_SetString(PyExc_TypeError, "Row index must be an integer or a slice.");
			        return NULL;
		    	}
		}
		else{
			PyErr_SetString(PyExc_TypeError, "Type Error.");
		    return NULL;
		}
	}
}
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
	matrix* res;
	int is_float=0;
    res=Matrix61c_helper(self,key,&is_float);
    if(res==NULL){
    	return NULL;
	}
    if(is_float){
    	double ans=res->data[0][0];
    	deallocate_matrix(res);
    	return PyFloat_FromDouble(ans);
	}
	else{
		Matrix61c* ans=(Matrix61c*)Matrix61c_new(&Matrix61cType,NULL,NULL);
		if(ans==NULL){
			deallocate_matrix(res);
			return NULL;
		}
		ans->shape=get_shape(res->rows,res->cols);
		ans->mat=res;
		return (PyObject*)ans;
	}
}
/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject *key, PyObject *v) {
	int is_float=0;
	matrix* res=Matrix61c_helper(self,key,&is_float);
	if(res==NULL)return -1;
	if(res->rows==1&&res->cols==1){
		if(PyLong_Check(v)||PyFloat_Check(v)){
			res->data[0][0]=PyFloat_AsDouble(v);
			deallocate_matrix(res);
			return 0;
		}
		else{
			deallocate_matrix(res);
			PyErr_SetString(PyExc_TypeError, "Type Error.");
		    return -1;
		}
	}
	else{
		if(res->is_1d){
			if(!PyList_Check(v)){
				deallocate_matrix(res);
				PyErr_SetString(PyExc_TypeError, "Type Error.");
		    	return -1;
			}
			int rows=res->rows,cols=res->cols;
			if (rows * cols != PyList_Size(v)) {
				deallocate_matrix(res);
		        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
		        return -1;
		    }
			int count = 0;
		    for (int i = 0; i < rows; i++) {
		        for (int j = 0; j < cols; j++) {
		        	if(PyLong_Check(PyList_GetItem(v, count)) || PyFloat_Check(PyList_GetItem(v, count)))
		        		res->data[i][j]=PyFloat_AsDouble(PyList_GetItem(v, count));
		        	else{
		        		deallocate_matrix(res);
		        		PyErr_SetString(PyExc_TypeError, "List contains non-number elements");
                    	return -1;
					}
		            count++;
		        }
		    }
		    deallocate_matrix(res);
			return 0;
		}
		else{
			int rows = PyList_Size(v);
		    if (rows == 0) {
		    	deallocate_matrix(res);
		        PyErr_SetString(PyExc_ValueError,
		                        "Cannot initialize numc.Matrix with an empty list");
		        return -1;
		    }
		    int cols;
		    if (!PyList_Check(PyList_GetItem(v, 0))) {
		    	deallocate_matrix(res);
		        PyErr_SetString(PyExc_ValueError, "List values not valid");
		        return -1;
		    } else {
		        cols = PyList_Size(PyList_GetItem(v, 0));
		    }
		    for (int i = 0; i < rows; i++) {
		        if (!PyList_Check(PyList_GetItem(v, i)) ||
		                PyList_Size(PyList_GetItem(v, i)) != cols) {
		            deallocate_matrix(res);
		            PyErr_SetString(PyExc_ValueError, "List values not valid");
		            return -1;
		        }
		    }
		    if(rows!=res->rows||cols!=res->cols){
		    	deallocate_matrix(res);
		    	PyErr_SetString(PyExc_ValueError, "List values not valid");
		    	return -1;
			}
		    for (int i = 0; i < rows; i++) {
		    	PyObject* itemi=PyList_GetItem(v, i);
		        for (int j = 0; j < cols; j++) {
		        	PyObject* itemj=PyList_GetItem(itemi, j);
		        	if(PyLong_Check(itemj)||PyFloat_Check(itemj)){
		        		res->data[i][j]=PyFloat_AsDouble(itemj);
					}
					else{
						deallocate_matrix(res);
		        		PyErr_SetString(PyExc_TypeError, "List contains non-number elements");
                    	return -1;
					}
		        }
		    }
		    deallocate_matrix(res);
		    return 0;
		}
	}
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}
