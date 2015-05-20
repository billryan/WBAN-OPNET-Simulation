#ifndef BAN_MATH_H
#define BAN_MATH_H

/*--------------------------------------------------------------------------------
 * Function:	exponent
 *
 * Description:	return n ^ e
 *
 *           n : integer,  e: integer
 *--------------------------------------------------------------------------------*/

static int exponent (int n, int e){
	int i ;
	long res;

	res = 1;
	
	for (i=1; i<=e; i++){
		res = res*n;
		}
	return (res);
}


/*--------------------------------------------------------------------------------
 * Function:	Symbols2Bits
 *
 * Description:	Converts the size of a frame from Symbols to Bits
 *
 *           SizeInSymbols : integer
 *--------------------------------------------------------------------------------*/

static int Symbols2Bits (int SizeInSymbols){
	return (SizeInSymbols*4);
}


/*--------------------------------------------------------------------------------
 * Function:	Bits2Symbols
 *
 * Description:	Converts the size of a frame from Bits to Symbols
 *
 *           SizeInSymbols : integer
 *--------------------------------------------------------------------------------*/

static int Bits2Symbols (int SizeInBits){
	
	int res;
	res = (SizeInBits/4)+1;
	if (SizeInBits%4 == 0)
		res = (SizeInBits/4);
	return (res);
}


/*--------------------------------------------------------------------------------
 * Function:	Symbols2ms
 *
 * Description:	Converts the size of a frame from Symbols to milliseconds
 *
 *           SizeInSymbols : integer,   datarateinbits : double
 *--------------------------------------------------------------------------------*/

static double Symbols2ms (int SizeInSymbols, double datarateinbits){
	double sizeinbits;
	double res;
	
	sizeinbits = (double)(SizeInSymbols)*4.0;
	res = (sizeinbits/datarateinbits)*1000;
	return (res);
}

/*--------------------------------------------------------------------------------
 * Function:	allocation_slot_length2ms
 *
 * Description:	Calculate the size of a frame from Allocation Slot Length to milliseconds
 *
 *           allocation_slot_length : integer
 *--------------------------------------------------------------------------------*/

// static double allocation_slot_length2ms (int allocation_slot_length){
// 	return double((pAllocationSlotMin + allocation_slot_length * pAllocationSlotResolution)/1000.0);
// }

/*--------------------------------------------------------------------------------
 * Function:	Symbols2Sec
 *
 * Description:	Converts the size of a frame from Symbols to seconds
 *
 *           SizeInSymbols : integer, datarateinbits : double
 *--------------------------------------------------------------------------------*/

static double Symbols2Sec (int SizeInSymbols, double datarateinbits){
	double sizeinbits;
	double res;
	
	sizeinbits = (double)(SizeInSymbols)*4.0;
	res = (sizeinbits/datarateinbits);
	return (res);
}


/*--------------------------------------------------------------------------------
 * Function:	Bits2Sec
 *
 * Description:	Converts the size of a frame from bits to seconds
 *
 *           SizeInBits : double, datarateinbits : double
 *--------------------------------------------------------------------------------*/

static double Bits2Sec (double sizeinbits, double datarateinbits){
	
	double res;
	res = (sizeinbits/datarateinbits);
	return (res);
}

/*--------------------------------------------------------------------------------
 * Function:	TX_TIME
 *
 * Description:	Calculate the transmission time of a packet
 *
 *           SizeInBits : double, datarateinKbits : double
 *--------------------------------------------------------------------------------*/

static double TX_TIME (double sizeinbits, double datarateinKbits){
	return sizeinbits/(1000 * datarateinKbits);
}


/*--------------------------------------------------------------------------------
 * Function:	min_int
 *
 * Description:	return the minimum of two interger
 *
 *				a : integer, b : integer           
 *--------------------------------------------------------------------------------*/

static int min_int (int a, int b){
	int mini;
	mini = a;
	if (b<a)
		mini = b;
	return (mini);
}

/*--------------------------------------------------------------------------------
 * Function:	max_int
 *
 * Description:	return the maximum of two interger
 *
 *				a : integer, b : integer           
 *--------------------------------------------------------------------------------*/

static int max_int (int a, int b){
	int mini;
	mini = a;
	if (b>a)
		mini = b;
	return (mini);
}

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
 * Function:	min_double
 *
 * Description:	return the minimum of two double
 *
 *   		a : double, b : double        
 *--------------------------------------------------------------------------------*/

static double min_double (double a, double b){
	double min;
	min = a;
	if (b<a)
		min = b;
	return (min);
}

/*--------------------------------------------------------------------------------
 * Function:	boolean2string
 *
 * Description:	return TRUE or FALSE
 *
 *           a: Boolean
 *--------------------------------------------------------------------------------*/

static char* boolean2string (Boolean a){
	
	if(a)
		return "TRUE";
	else
		return "FALSE";
}

/*--------------------------------------------------------------------------------
 * Function:	dabs
 *
 * Description:	return absolute value
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
 * Function:	rand_int(int seed)
 *
 * Description:	return random integer from given seed
 *
 *           seed integer
 *--------------------------------------------------------------------------------*/
 static int rand_int (int module) {
	// int new_seed, rand_int;
	// PrgT_Random_Gen *my_rng;

	// new_seed = (int)time(0) + (int)op_dist_uniform (256);
	// /* create a new random number generator */
	// my_rng = op_prg_random_gen_create (new_seed);
	// printf("new_seed=%d\n", new_seed);

	//  generate a random integer in the interval [0,module-1] 
	// rand_int = (op_prg_random_integer_gen (my_rng) % module);

	// /* destroy the random number generator */
	// op_prg_random_gen_destroy (my_rng);
	return (int)op_dist_uniform(module);
 }
#endif