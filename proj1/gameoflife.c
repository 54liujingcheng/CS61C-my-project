/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row as adjacent to the bottom row
//and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule)
{
	Color* res=(Color*)malloc(sizeof(Color));
	int dir[8][2]={{0,1},{1,0},{-1,0},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
	uint8_t r=0,g=0,b=0;
	for(int k=0;k<8;k++){
		int total=0;
		int live=((image->image[row][col].R>>k)?9:0);
		for(int i=0;i<8;i++){
			int dx=row+dir[i][0],dy=col+dir[i][1];
			if(dx<0)dx+=image->rows;
			if(dx>=image->rows)dx-=image->rows;
			if(dy<0)dy+=image->cols;
			if(dy>=image->cols)dy-=image->cols;
			if((image->image[dx][dy].R)>>k)total++;
		}
		r+=(rule>>(live+total))&1;
		r*=2;
	}
	res->R=r;
	for(int k=0;k<8;k++){
		int total=0;
		int live=((image->image[row][col].G>>k)?9:0);
		for(int i=0;i<8;i++){
			int dx=row+dir[i][0],dy=col+dir[i][1];
			if(dx<0)dx+=image->rows;
			if(dx>=image->rows)dx-=image->rows;
			if(dy<0)dy+=image->cols;
			if(dy>=image->cols)dy-=image->cols;
			if((image->image[dx][dy].G)>>k)total++;
		}
		g+=(rule>>(live+total))&1;
		g*=2;
	}
	res->G=g;
	for(int k=0;k<8;k++){
		int total=0;
		int live=((image->image[row][col].B>>k)?9:0);
		for(int i=0;i<8;i++){
			int dx=row+dir[i][0],dy=col+dir[i][1];
			if(dx<0)dx+=image->rows;
			if(dx>=image->rows)dx-=image->rows;
			if(dy<0)dy+=image->cols;
			if(dy>=image->cols)dy-=image->cols;
			if((image->image[dx][dy].B)>>k)total++;
		}
		b+=(rule>>(live+total))&1;
		b*=2;
	}
	res->B=b;
	return res;
}

//The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
//You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule)
{
	if(image==NULL)return NULL;
	Image *ans=(Image*)malloc(sizeof(Image));
	if(ans==NULL)return NULL;
	ans->rows=image->rows,ans->cols=image->cols;
	ans->image=(Color**)malloc(sizeof(Color*)*ans->rows);
	if(ans->image==NULL)return NULL;
	for(int i=0;i<image->rows;i++){
		ans->image[i]=(Color*)malloc(sizeof(Color)*ans->cols);
		if(ans->image[i]==NULL){
			for(int j=0;j<i;j++)free(ans->image[j]);
			free(ans->image);
			free(ans);
			return NULL;
		}
	}
	for(int i=0;i<image->rows;i++){
		for(int j=0;j<image->cols;j++){
			Color* res=evaluateOneCell(image,i,j,rule);
			ans->image[i][j].R=res->R;
			ans->image[i][j].G=res->G;
			ans->image[i][j].B=res->B;
			free(res);
		}
	}
	freeImage(image);
	return ans;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv)
{
	if(argc!=3){
		printf("usage: %s filename rule\nfilename is an ASCII PPM file (type P3) with maximum value 255.\nrule is a hex number beginning with 0x; Life is 0x1808.",argv[0]);
		return -1;
	}
	Image* im=readData(argv[1]);
	if(im==NULL)return -1;
	uint32_t rule=strtoul(argv[2],NULL,16);
	if(rule>0x3ffff){
		return -1;
	}
	im=life(im,rule);
	if(im==NULL)return -1;
	writeData(im);
	freeImage(im);
	return 0;
}//mine
