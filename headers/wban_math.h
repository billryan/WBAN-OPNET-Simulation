#ifndef BAN_MATH_H
#define BAN_MATH_H

/*--------------------------------------------------------------------------------
 * Function:	max_double
 *
 * Description:	return the maximum of two double
 *
 *  		a : double, b : double         
 *--------------------------------------------------------------------------------*/
static double max_double (double a, double b){
	double max;
	max = a;
	if (b>a)
		max = b;
	return (max);
}
/*--------------------------------------------------------------------------------
 * Function:   dabs
 *
 * Description:        return absolute value
 *
 *           a : double
 *--------------------------------------------------------------------------------*/
static double dabs (double a) {
       if(a<0)
               return (-1*a);
       else
               return a;
}

/*--------------------------------------------------------------------------------
 * Function:	compare_doubles
 *
 * Description:	return 0 if a==b, 1 if a>b, -1 if a<b
 *
 *           a : double, b : double 
 *--------------------------------------------------------------------------------*/
static int compare_doubles (double a, double b) {
	if (dabs(a-b) < 0.000001) {
		return 0;
	} else {
		if(a > b) {
			return 1;
		} else {
			return -1;
		}
	}
}

/*--------------------------------------------------------------------------------
 * Function:	rand_int(int module)
 *
 * Description:	return random integer from given seed
 *           seed integer
 *--------------------------------------------------------------------------------*/
static int rand_int (int module) {
	return (int)op_dist_uniform(module);
}
#endif