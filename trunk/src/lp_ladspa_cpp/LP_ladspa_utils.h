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

#ifndef LP_LADSPA_UTILS_H
#define LP_LADSPA_UTILS_H

#include <stdio.h>
#include <stdlib.h>

class LP_ladspa_utils // NOTE Type: float etre LADSPA_data ? Verifier validite de channel_len
{
	public:
	/**
	Copy 2 simple arrays to a simple (but longer) array
	**/
	int LP_L_R_to_LR(const float *L, const float *R, float *LR, int LR_len);
	/**
	Copy a simple interlaced array into 2 arrays
	**/
	int LP_LR_to_L_R(float *L, float *R, float *LR, int LR_len);

	/**
	Copy left channel of a simple interlaced array into a simple array
	**/
	int LP_LR_to_L(float *L, float *LR, int LR_len);

	/**
	Copy a simple array into left channel of a simple interlaced array
	**/
	int LP_L_to_LR(float *L, float *LR, int LR_len);

	/**
	Copy a simple array into right channel of a simple interlaced array
	**/
	int LP_R_to_LR(float *R, float *LR, int LR_len);

	/**
	Copy right channel of a simple interlaced array into a simple array
	**/
	int LP_LR_to_R(float *R, float *LR, int LR_len);

	/**
	Allocate memory for a 2dim array
	**/
	float **lp_fmalloc_2d_array(int channels, int channel_len);
	
	/**
	Free a 2dim array
	**/
	int lp_ffree_2d_array(float **ptr, int channels, int channel_len);
	
	/**
	Copy a 2dim array into a simple array. 
	Simple array's size must be channels x 2d array's size
	This makes a interlaced (LRLRLR if 2 channels) buffer
	**/
	int lp_fcopy_2d_1d_array(float **dbuf, float *sbuf, int channels, int channel_len);
	
	/**
	Copy a simple array (interlaced) into double array
	channel_len is the len of each channel
	**/
	int lp_fcopy_1d_2d_array(float **dbuf, float *sbuf, int channels, int channel_len);
};

#endif // LP_LADSPA_UTILS_H
