#include "define.h"
#include "disparity.h"
#include "tb_disparity.h"

int sc_main(int argc, char** argv)
{
	sc_clock                  clk("clk", 25, SC_NS, 0.5, 12.5, SC_NS, true);
	sc_signal<bool>           rst;
	sc_signal<sc_uint<8> >    data_in_r[WIN_SIZE];
	sc_signal<sc_uint<8> >    data_in_l[WIN_SIZE];
	sc_signal<sc_uint<16> >   width;
	sc_signal<sc_uint<8> >    data_out;
	sc_signal<bool>           ready;
	
	int i;
	
	disparity u_disparity("disparity");
	test_disparity test("test_disparity");
	
	// connect to disparity
	u_disparity.clk( clk );
	u_disparity.rst( rst );
	for( i=0; i<WIN_SIZE; i++ ){
		u_disparity.data_in_r[i]( data_in_r[i] );
		u_disparity.data_in_l[i]( data_in_l[i] );
	}
	u_disparity.width( width );
	u_disparity.data_out( data_out );
	u_disparity.ready( ready );
	
	// connect to test bench
	test.clk( clk );
	test.rst( rst );
	test.data_out( data_out );
	test.ready( ready );
	for( i=0; i<WIN_SIZE; i++ ){
		test.data_in_r[i]( data_in_r[i] );
		test.data_in_l[i]( data_in_l[i] );
	}
	test.image_width( width );

	sc_start( 25, SC_NS );
	rst.write(0);

	sc_start( 25, SC_NS );
	rst.write(1);

	sc_start();
	
	return 0;
	
};