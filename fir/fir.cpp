//========================================================================================
// 
// File Name    : fir.cpp
// Description  : FIR filter
// Release Date : 14/02/2013
// Author       : PolyU DARC Lab
//                Benjamin Carrion Schafer, Anushree Mahapatra 
// 
// Revision History
//---------------------------------------------------------------------------------------
// Date         Version    Author      Description
//---------------------------------------------------------------------------------------
//14/02/2013      1.0       PolyU     FIR filter main description
//=======================================================================================
#define SC_INCLUDE_FX
#include "fir.h"
#define MAX 255 

//  Main thread  
void fir::fir_main ( void ) {

	// Variables declaration
	sc_uint<16> filter_output_function;
	sc_uint<8> in_data_read;
	sc_fixed<8,4,SC_RND,SC_SAT> coeff_read[FILTER_TAPS];
	sc_uint<8> data_buffer[FILTER_TAPS];
	int i,j;
	for(i=0;i<FILTER_TAPS-1;i++)
		data_buffer[i] = 0;

	// Reset state - should be executable in 1 clock cycle
	wait();
	
	// Main thread	
	while (1){		
		// read inputs
		in_data_read = in_data.read();
		for(i=0;i<FILTER_TAPS;i++){
			coeff_read[i] = coeff[i].read();
		}

		// shift
		for(j=FILTER_TAPS-1;j>0;j--){
			data_buffer[j] = data_buffer[j-1];
		}
		data_buffer[0] = in_data_read;

		// Filter function
		filter_output_function = filter(data_buffer, coeff_read);
		
		// write outputs
		filter_output.write(filter_output_function) ;
		wait();
	}
}

// Filter function
sc_uint<16> fir::filter( sc_uint<8>  *ary, sc_fixed<8,4,SC_RND,SC_SAT>  *coeff)
{
	sc_fixed<21,17,SC_RND,SC_SAT> sop=0;
	sc_uint <16> filter_result ;
	int i ;

    // Sum of product (SOP) generation 
	for(i=0;i<FILTER_TAPS;i++){
		sop += sc_fixed<21,17,SC_RND,SC_SAT>(coeff[i]) * ary[i];
	}

    // Sign adjustment and rounding to sc_unit <8>)
	if ( sop < 0 ){
		sop = 0 ;
	}

	filter_result = sc_uint<16>(sop);
	return filter_result;
}


