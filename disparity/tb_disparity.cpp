#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "define.h"
#include "tb_disparity.h"


U8 * test_disparity::load_bmp(const char *fname, BmpFileHeader *Header,
                              BmpInfoHeader *InfoHeader, U8 **palette)
{
	FILE *fp     = NULL;
	U8 *image    = NULL;

	// Open image file
	fp = fopen(fname, "rb");
	if (!fp)
	{
		printf("Coundn't open the image.\n");
		exit(1);
	}

	// Read Header information
	fread(Header, sizeof(BmpFileHeader), 1, fp);

	// Check if the image is in BMP format
	if (Header->type != 0x4D42)
	{
		fclose(fp);
		printf("Image is not in BMP format.\n");
		exit(1);
	}
	

	// Read the image details
	fread(InfoHeader, sizeof(BmpInfoHeader), 1, fp);

	// Print the information
	// printf("\n---- %s ----\n", fname);
	// print_bmp(Header, InfoHeader);

	// if the image is in gray-scale, the palette has to be read
	if (InfoHeader->bits == 8)
	{
		int palette_size = Header->offset - sizeof(BmpFileHeader) - sizeof(BmpInfoHeader);
		int mem_palette_size = sizeof(U8) * palette_size;
		*palette = (U8 *)malloc(mem_palette_size);
		fread(*palette, mem_palette_size, 1, fp);
	}

	// allocate memory for image data
	image = (U8 *)malloc(InfoHeader->image_size);
	
	// Read the image data (B,G,R)
	fread(image, sizeof(U8), InfoHeader->image_size, fp);

	// Close file
	fclose(fp);
	
	// Return image data
	return image;
}

void test_disparity::write_bmp(const char *fname, U8 *image,
          BmpFileHeader *Header, BmpInfoHeader *InfoHeader, U8 *palette)
{
	FILE *fp = NULL;

	// Open an empty image file
	fp = fopen(fname, "wb");
	if (!fp)
	{
		perror("Coundn't open the output image.");
		exit(1);
	}

	// write headers to image file
	fwrite(Header, sizeof(BmpFileHeader), 1, fp);
	fwrite(InfoHeader, sizeof(BmpInfoHeader), 1, fp);

	// if the image is in gray-scale, it needs palette
	if (InfoHeader->bits == 8)
	{
		int mem_palette_size = Header->offset - sizeof(BmpFileHeader)
							   - sizeof(BmpInfoHeader);
		fwrite(palette, mem_palette_size, 1, fp);
	}

	// Write image data to file
	fwrite(image, InfoHeader->image_size, 1, fp);

	fclose(fp);
}

void test_disparity::pad_image(U8 **image_out, U8 *image_in,
			int width, int height, int margin_width, int margin_height,
			int padded_width, int padded_height)
{
	int i         = 0;
	int j         = 0;
	int index_old = 0;
	int index_new = 0;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			index_old = i * width + j;
			index_new = (margin_height + i) * padded_width + margin_width + j;

			(*image_out)[index_new] = image_in[index_old];
		}
	}
}

void test_disparity::histogram_equalization(U8 **image_out, U8 *image_in,
                                            int height, int width)
{
	int hist[HIST_BIN] = {0};

	int trans[HIST_BIN] = {0};

	int i;
	int j;

	int size = height * width;

	for (i = 0; i < HIST_BIN; i++)
	{
		hist[i] = 0;
		trans[i] = 0;
	}

	for (i = 0; i < size; i++)
	{
		hist[image_in[i]] += 1;
	}


	for (i = 0; i < HIST_BIN; i++)
	{
		for (j = 0; j < i + 1; j++)
		{
			trans[i] += HIST_BIN * ((float)hist[j]) / size;
		}
	}

	for (i = 0; i < size; i++)
	{
		(*image_out)[i] = 255 - trans[image_in[i]];
	}
}


void test_disparity::send()
{
	// Load left and right images
	BmpFileHeader HeaderLeft;
	BmpFileHeader HeaderRight;
	BmpInfoHeader InfoHeaderLeft;
	BmpInfoHeader InfoHeaderRight;
	U8 *palette_l   = NULL;
	U8 *palette_r   = NULL;
	U8 *image_l     = NULL;
	U8 *image_r     = NULL;

	clock_t start = 0;
	clock_t end   = 0;
	double elapsed = 0.0;

	int i,j,k;

	wait();
	
	while(1){
		image_l = load_bmp(IMAGE_L, &HeaderLeft, &InfoHeaderLeft, &palette_l);
		image_r = load_bmp(IMAGE_R, &HeaderRight, &InfoHeaderRight, &palette_r);

		// Check if two images are equal size
		U32 width       = 0;
		U32 height      = 0;
		U32 size        = 0;
		U32 mem_size    = 0;
		if ((InfoHeaderLeft.width == InfoHeaderRight.width)
			&& (InfoHeaderLeft.height == InfoHeaderRight.height))
		{
			width = InfoHeaderLeft.width;
			height = InfoHeaderLeft.height;
			size = width * height;
			mem_size = sizeof(U8) * size;
		}
		else
		{
			perror("ERROR: two images with different size.");
			exit(1);
		}

		// Variables for padding images
		U32 margin_width              = WIN_SIZE / 2;
		U32 margin_height             = WIN_SIZE / 2;
		U32 padded_width              = width + margin_width * 2;
		U32 padded_height             = height + margin_height * 2;
		U32 padded_size               = padded_width * padded_height;
		U32 mem_padded_size           = sizeof(U8) * padded_size;
		U8 *padded_image_l            = (U8 *)malloc(mem_padded_size);
		U8 *padded_image_r            = (U8 *)malloc(mem_padded_size);

		image_disparity = (U8 *)malloc(mem_size);
		memset(image_disparity, 0, mem_size);

		// Padding images
		pad_image(&padded_image_l, image_l, width, height,
				  margin_width, margin_height, padded_width, padded_height);
		pad_image(&padded_image_r, image_r, width, height,
				  margin_width, margin_height, padded_width, padded_height);
		start = clock();
		image_width.write(padded_width);

		for (i=0; i<padded_height-WIN_SIZE+1; i++){
			for (j=0; j<padded_width; j++){
				for (k=0; k<WIN_SIZE; k++){
					data_in_r[k].write(padded_image_r[(i+k)*padded_width + j]);
					data_in_l[k].write(padded_image_l[(i+k)*padded_width + j]);
				}
				wait();
			}
			wait();

			while( ready.read()!=0 )
				wait();
		}

		
		end = clock();
		elapsed = (double)(end - start) / CLOCKS_PER_SEC;
		printf("CPU stereo vision time: %0.3f ms\n", elapsed * 1000.0);

		// Use histogram equalization to enhance the contrast
		U8 *image_out = (U8 *)malloc(mem_size);
		memset(image_out, 0, mem_size);
		histogram_equalization(&image_out, image_disparity, height, width);

		// Generate the final image with distance information
		write_bmp(IMAGE_DIS, image_out, &HeaderLeft, &InfoHeaderLeft, palette_l);

		// Deallocate the objects
		free(image_l);
		free(image_r);
		free(padded_image_l);
		free(padded_image_r);
		free(image_disparity);
		free(image_out);
		
		sc_stop();
		
	}
}

void test_disparity::recv()
{
	int k=0;
	int data_out_read;

	wait();

	while(1){
		while(ready.read()!=1){
			wait();
		}
		data_out_read = data_out.read();
		image_disparity[k] = data_out_read;
		k++;
		wait();
	}
}