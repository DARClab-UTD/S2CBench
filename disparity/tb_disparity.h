#ifndef TB_DISPARITY_H_
#define TB_DISPARITY_H_
#include "define.h"

SC_MODULE (test_disparity){
	
	//inputs
	sc_in<bool>          clk;
	sc_in<bool>          rst;

	sc_in <sc_uint<8> >  data_out;
	sc_in <bool> ready;
	
	//outputs
	sc_out <sc_uint<8> > data_in_r[WIN_SIZE];
	sc_out <sc_uint<8> > data_in_l[WIN_SIZE];
	sc_out <sc_uint<16> > image_width;
	
	U8 *image_disparity;

	// functions
	void pad_image(U8 **image_out, U8 *image_in,
          int width, int height, int margin_width, int margin_height,
          int padded_width, int padded_height);

	U8 *load_bmp(const char *fname, BmpFileHeader *Header, BmpInfoHeader *InfoHeader,
         U8 **palette);

	void write_bmp(const char *fname, U8 *image,
          BmpFileHeader *Header, BmpInfoHeader *InfoHeader, U8 *palette);

	void histogram_equalization(U8 **image_out, U8 *disparity, int height, int width);
	
	void recv();
	void send();
	
	SC_CTOR (test_disparity){
		
		SC_CTHREAD(send, clk.pos());
		reset_signal_is(rst,false);
		
		SC_CTHREAD(recv, clk.pos());
		reset_signal_is(rst,false);
	}
	
	~test_disparity(){}

};

#endif