/************************************************************************
**
** NAME:        imageloader.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**              Justin Yokota - Starter Code
**				YOUR NAME HERE
**
**
** DATE:        2020-08-15
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "imageloader.h"

//Opens a .ppm P3 image file, and constructs an Image object. 
//You may find the function fscanf useful.
//Make sure that you close the file with fclose before returning.
Image *readData(char *filename) 
{
	FILE *fin=fopen(filename,"r");
	if(fin==NULL){
		fclose(fin);
		return NULL;
	}
	char bf[20];uint32_t colors,cols,rows;
	fscanf(fin,"%s %u %u %u",bf,&cols,&rows,&colors);
	Image* image=(Image*)malloc(sizeof(Image));
	if(image==NULL)return NULL;
	image->rows=rows,image->cols=cols;
	image->image=(Color**)malloc(sizeof(Color*)*rows);
	if(image->image==NULL)return NULL;
	for(int i=0;i<rows;i++){
		image->image[i]=(Color*)malloc(sizeof(Color)*cols);
		if(image->image[i]==NULL){
			for(int j=0;j<i;j++)free(image->image[j]);
			free(image->image);
			free(image);
			return NULL;
		}
		for(int j=0;j<cols;j++){
			fscanf(fin,"%hhu %hhu %hhu",&image->image[i][j].R,&image->image[i][j].G,&image->image[i][j].B);
		}
	}
	fclose(fin);
	return image;
}

//Given an image, prints to stdout (e.g. with printf) a .ppm P3 file with the image's data.
void writeData(Image *image)
{
	printf("P3\n");
	printf("%u %u\n",image->cols,image->rows);
	printf("255\n");
	for(int i=0;i<image->rows;i++){
		for(int j=0;j<image->cols;j++){
			printf("%3hhu %3hhu %3hhu",image->image[i][j].R,image->image[i][j].G,image->image[i][j].B);
			if(j!=image->cols-1)printf("   ");
		}
		printf("\n");
	}
}
//Frees an image
void freeImage(Image *image)
{
	for(int i=0;i<image->rows;i++){
		free(image->image[i]);
	}
	free(image->image);
	free(image);
}
