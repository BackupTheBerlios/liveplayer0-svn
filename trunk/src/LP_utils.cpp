/***************************************************************************
 *   Copyright (C) 2006 by Philippe   *
 *   nel230@gmail.ch   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "LP_utils.h"

/**
   Copy 2 simple arrays to a simple (but longer) interlaced array
**/
int LP_utils::LP_L_R_to_LR(const float *L, const float *R, float *LR, int LR_len)
{
	// LR_len / 2 must be integer
	unsigned int i;
	for(i=0; i<LR_len/2; i++){
		LR[i*2]   = L[i];
		LR[i*2+1] = R[i];
	}
	return 0;
}

/**
   Copy a simple interlaced array into 2 arrays
**/
int LP_utils::LP_LR_to_L_R(float *L, float *R, float *LR, int LR_len)
{
	// LR_len / 2 must be integer
	unsigned int i;
	for(i=0; i<LR_len/2; i++){
		L[i] = LR[i*2];
		R[i] = LR[i*2+1];
	}
	return 0;
}

/**
   Allocate memory for a 2dim array
**/
float **LP_utils::lp_fmalloc_2d_array(int channels, int channel_len)
{
	float **tmp = (float**)malloc(channels*sizeof(float*));
	if(tmp == NULL){ return NULL;}
	int i;
	for(i=0; i<channels; i++){
		tmp[i] = (float*)malloc(channel_len*sizeof(float));
		if(tmp[i] == NULL){ return NULL; }
	}
	return tmp;
}

/**
   Free a 2dim array
**/
int LP_utils::lp_ffree_2d_array(float **ptr, int channels, int channel_len)
{
 if(ptr == NULL){
        fprintf(stderr, "lp_free_2d_array: **ptr is NULL\n");
        return -1;
 }
 int i;
 for(i=0; i<channels; i++){
          if(ptr[i] != NULL){
                    free(ptr[i]);
                    ptr[i] = NULL;
          }else{
                fprintf(stderr, "lp_free_2d_array: ptr[%d] was NULL\n", i);
                return -1;
          }
 }
 free(ptr);
 return 0;
}

/**
   Copy a 2dim array into a simple array. 
   Simple array's size must be channels x 2d array's size
   This makes a interlaced (LRLRLR if 2 channels) buffer
**/
int LP_utils::lp_fcopy_2d_1d_array(float **dbuf, float *sbuf, int channels, int channel_len)
{
	int i, j;
	for(i=0; i<channels; i++){
		for(j=0; j<channel_len; j++){
			sbuf[i+j*channels] = dbuf[i][j];
		}
	}
	return 0;
}

/**
   Copy a simple array (interlaced) into double array
   channel_len is the len of each channel
**/
int LP_utils::lp_fcopy_1d_2d_array(float **dbuf, float *sbuf, int channels, int channel_len)
{
	int i, j;
	for(i=0; i<channels; i++){
		for(j=0; j<channel_len; j++){
			dbuf[i][j] = sbuf[i+j*channels];
		}
	}
	return 0;
}
