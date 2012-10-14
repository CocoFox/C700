/*
 *  EchoKernel.cpp
 *  Chip700
 *
 *  Created by ���c ���F on 12/10/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "EchoKernel.h"
#include <string.h>

//-----------------------------------------------------------------------------
EchoKernel::EchoKernel()
{
	mEchoBuffer = new int[ECHO_BUFFER_SIZE];
	mFIRbuf = new int[FIR_LENGTH];
	memset(mEchoBuffer, 0, ECHO_BUFFER_SIZE*sizeof(int));
	memset(mFIRbuf, 0, FIR_LENGTH*sizeof(int));
	mEchoIndex	= 0;
	mFIRIndex	= 0;
	m_echo_vol	= 0;
	m_input		= 0;
};

//-----------------------------------------------------------------------------
EchoKernel::~EchoKernel()
{
	delete [] mEchoBuffer;
	delete [] mFIRbuf;
}

//-----------------------------------------------------------------------------
void EchoKernel::Input(int samp)
{
	m_input += samp;
}

//-----------------------------------------------------------------------------
int EchoKernel::GetFxOut()
{
	int echo = m_input;
	
	mEchoIndex &= 0x7fff;
	
	//�f�B���C�M����FIR�t�B���^���|���Ă���o�͂ɉ��Z����
	mFIRbuf[mFIRIndex] = mEchoBuffer[mEchoIndex];
	int i;
	int sum = 0;
	for (i=0; i<FIR_LENGTH-1; i++) {
		sum += mFIRbuf[mFIRIndex] * m_fir_taps[i];
		mFIRIndex = (mFIRIndex + 1)%FIR_LENGTH;
	}
	sum += mFIRbuf[mFIRIndex] * m_fir_taps[i];
	sum >>= 7;
	//mFIRbuf�ւ̏������݂́A�E���獶�ւƍs����
	int output = ( sum * m_echo_vol ) >> 7;
	
	//���͂Ƀt�B�[�h�o�b�N�����Z�������̂��o�b�t�@�L���[�ɓ����
	echo += ( sum * m_fb_lev ) >> 7;
	mEchoBuffer[mEchoIndex++] = echo;
	if (mEchoIndex >= m_delay_samples) {
		mEchoIndex=0;
	}
	m_input = 0;
	
	return output;
}

//-----------------------------------------------------------------------------
void EchoKernel::Reset()
{
	memset(mEchoBuffer, 0, ECHO_BUFFER_SIZE*sizeof(int));
	memset(mFIRbuf, 0, FIR_LENGTH*sizeof(int));
	mEchoIndex	= 0;
	mFIRIndex	= 0;
	m_input		= 0;
}
