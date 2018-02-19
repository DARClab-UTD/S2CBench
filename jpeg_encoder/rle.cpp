//========================================================================================
// 
// File Name    : rle.cpp
// Description  : JPEG RLE 
// Release Date : 30/05/2017
// Author       : UT Dallas DARClab
//                Jianqi Chen, Benjamin Carrion Schafer
// 
//
// Revision History
//---------------------------------------------------------------------------------------
// Date         Version         Author          Description
//----------------------------------------------------------------------------------------
// 30/05/2017        1.0        UTD DARClab     JPEG RLE
//=======================================================================================


#include "rle.h"

void rle::jpeg_rle()
{
  sc_int<QUAN_OUT_WIDTH> quantized_block[8][8];
  sc_int<QUAN_OUT_WIDTH> rl[RLE_BUFF_LEN];
  sc_uint<8> len; //length of RL code
  int x,y,i,k;
  previous_dc = 0;
  int j = -2;

  wait();

  while(1)
  {
    j++;
    if(j==8){
      j = 0;
      i = 1;
      k = 0;
      len = 1;
      // read inputs
      for(x=0;x<8;x++)
        for(y=0;y<8;y++)
          quantized_block[x][y] = rle_in[x*8+y].read();

      // zigzag
      jpeg_zigzag(quantized_block);
  
      // run length encoding
      // DC component
      rl[0] = zz[0] - previous_dc;
      previous_dc = zz[0];
  
      // AC components 
      while(i<64)
      {
        k=0;
        while((i<64)&&(zz[i]==0)&&(k<15))
        {
          i++;
          k++;
        }
        if(i==64)  //end of block
        {
          rl[len++] = 0;
          rl[len++] = 0;
        }
        else
        {
          rl[len++] = k; //number of zeros before a non-zero number
          rl[len++] = zz[i++];  //non-zero number
        }
      }
      if(!(rl[len-1]==0 && rl[len-2]==0))
      {
        rl[len++] = 0;
        rl[len++] = 0;
      }
      while((rl[len-4]==15)&&(rl[len-3]==0))
      {
          rl[len-4]=0;
          len-=2;
      }
      //write outputs
      for(x=0;x<RLE_BUFF_LEN;x++)
          rle_out[x].write(rl[x]);
  
      rle_length.write(len);
    }
    wait();
  }

}

void rle::jpeg_zigzag(sc_int<QUAN_OUT_WIDTH> input[8][8])
{
  int i=0,j=0,k=0,d=0;
  
  //zigzag for the top left half of the input block
  while(k<36)
  {
    zz[k++] = input[i][j];
    if((i==0)&&(j%2==0))
    {
      j++;
      d = 1;
    }
    else if((j==0)&&(i%2==1))
    {
      i++;
      d = 0;
    }
    else if(d==0)
    {
      i--;
      j++;
    }
    else
    {
      i++;
      j--;
    }
  }

  i = 7;
  j = 1;
  d = 0;

  //zigzag for the bottom right half of the input block
  while(k<64)
  {
    zz[k++] = input[i][j];
    if((i==7)&&(j%2==0))
    {
      j++;
      d = 0;
    }
    else if((j==7)&&(i%2==1))
    {
      i++;
      d = 1;
    }
    else if(d==0)
    {
      i--;
      j++;
    }
    else
    {
      i++;
      j--;
    }
  }
}




