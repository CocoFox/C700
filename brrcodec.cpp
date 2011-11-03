/*
 *  brrcodec.c
 *  Chip700
 *
 *  Created by �J���p on 06/11/06.
 *  Copyright 2006 Vermicelli Magic. All rights reserved.
 *
 */

#include "brrcodec.h"

#define filter1(a1)	(( a1 >> 1 ) + ( ( -a1 ) >> 5 ))
#define filter2(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 >> 1 ) ) ) >> 5 ) \
						 - ( a2 >> 1 ) + ( a2 >> 5 ))
#define filter3(a1,a2)	(a1 \
						 + ( ( -( a1 + ( a1 << 2 ) \
								  + ( a1 << 3 ) ) ) >> 7 ) \
						 - ( a2 >> 1 ) \
						 + ( ( a2 + ( a2 >> 1 ) ) >> 4 ))


//���̓f�[�^�F�l�C�e�B�u�G���f�B�A���P�Ubit�A���m�����A�R�QkHz
//inputframe�� 16*n �łȂ��Ƃ��A�擪�ɖ�����t�������܂�
int brrencode(short *input_data, unsigned char *output_data, long inputframes)
{
	short			*input = input_data;
	unsigned char	*output = output_data;
	int				r,f,frm;
	long			frames_remain,frame_offset,outbytes=0;
	unsigned int	filter, range, half;
	long			blockdata[13][4][16];
	long			out1[13][4], out2[13][4], err_sum[13][4];
	
	input[0]=0;
	frame_offset = (16-(inputframes % 16))%16;
	frames_remain = inputframes;
	
	//��ƕϐ��̏�����
	for (f=0; f<4; f++) {
		for (r=0; r<13; r++) {
			out1[r][f] = 0;
			out2[r][f] = 0;
			err_sum[r][f] = 0;
		}
	}
	
	while (frames_remain > 0) {
		for (frm=0; frm<16; frm++) {
			for (f=0; f<4; f++) {
				for (r=0; r<13; r++) {
					long	nbl,c1,c2,out;
					
					if (frame_offset > 0)
						nbl = 0;
					else
						nbl = *input;
					
					c1=out1[r][f];
					c2=out2[r][f];
					
					//f�l���Ƃ̗\���덷�����߂�
					if (f==1)
						nbl -= filter1(c1);
					else if (f==2)
						nbl -= filter2(c1,c2);
					else if (f==3)
						nbl -= filter3(c1,c2);
					
					//4bit�ɗʎq������
					if (r != 0)
					{
						nbl += 1<<(r-1);
					}
					nbl >>= r;
					if ( nbl > 7 )
					{
						nbl = 7;
					}
					else if ( nbl < -8 )
					{
						nbl = -8;
					}
					blockdata[r][f][frm] = nbl;
					
					//�덷�����߂邽�߃f�R�[�h����
					out=nbl<<r;
					if(f==1)
						out+=filter1(c1);
					else if(f==2)
						out+=filter2(c1,c2);
					else if(f==3)
						out+=filter3(c1,c2);
					if( out < -32768 )
					{
						out = -32768;
					}
					else if( out > 32767 )
					{
						out = 32767;
					}
					out2[r][f]=c1;
					out1[r][f]=out<<1;
					
					//���M���Ƃ̍��̐�Βl�����߂�
					nbl=out-*input;
					err_sum[r][f] += nbl<0 ? -nbl:nbl;
				}
			}
			input++;
			frames_remain--;
			if (frame_offset-- > 0) {
				input--;
				frames_remain++;
			}
		}
		
		//�덷�̍��v���ŏ��ȃt�B���^�������W��I��
		filter = 0;
		range = 0;
		for (f=0; f<4; f++) {
			for (r=0; r<13; r++) {
				if (err_sum[12-r][f] < err_sum[range][filter]) {
					filter = f;
					range = 12-r;
				}
			}
		}
		//�I�񂾃t�B���^�̉ߋ��M���ɍX�V�A�덷�l�̏�����
		for (f=0; f<4; f++) {
			for (r=0; r<13; r++) {
				out1[r][f]=out1[range][filter];
				out2[r][f]=out2[range][filter];
				err_sum[r][f] = 0;
			}
		}
		
		//Header�o�C�g�̐ݒ�
		*output = range<<4;
		*output |= filter<<2;
		if (frames_remain <= 0)
			*output |= 1;	//ENDbit�̕t��
		output++;
		outbytes++;
		
		//�f�[�^�o�C�g�̏�������
		half = 0;
		for (frm=0; frm<16; frm++) {
			if (half == 0) {
				*output = (blockdata[range][filter][frm] << 4) & 0xf0;
				half = 1;
			}
			else {
				*output |= blockdata[range][filter][frm] & 0xf;
				half = 0;
				output++;
				outbytes++;
			}
		}
	}
	return outbytes;
}

bool checkbrrsize(unsigned char *src, int *size)
{
	int		count=9;
	while((*src & 1)==0) {
		src += 9;
		count += 9;
	}
	*size = count;
	return (*src & 2)?true:false;
}

int brrdecode(unsigned char *src, short *output, int looppoint, int loops)
{
	int range, end=0, loop, filter, counter, temp;
	short input;
	long	out=0,out1=0,out2=0, temp2;
	long	outb1=0,outb2=0;
	int		now=0;
	int		remainloop=loops-1;
	unsigned char *loopaddr = src+looppoint;
	
	while(end==0)
	{
        range=*(src++);
        end=range&1;
        loop=range&2;
        filter=(range>>2)&3;
        range>>=4;
		
        for(counter=0;counter<8;counter++)
        {
			temp=*(src++);
			input=temp>>4;
			input&=0xF;
			if(input>7)
				input |= ~0xF;
			
			out2=out1;
			out1=out<<1;
			out=input<<range;
			
			if(filter==1)
				out+=filter1(out1);
			else if(filter==2)
				out+=filter2(out1,out2);
			else if(filter==3)
				out+=filter3(out1,out2);
			
			temp2 = out;
			if( temp2 < -32768 )
			{
				temp2 = -32768;
			}
			else if( temp2 > 32767 )
			{
				temp2 = 32767;
			}
			output[now] = temp2;
			outb2=outb1;
			outb1=temp2;
			
			now++;
			
			input=temp&0xF;
			if(input>7)
				input |= ~0xF;
			
			out2=out1;
			out1=out<<1;
			out=input<<range;
			
			if(filter==1)
				out+=filter1(out1);
			else if(filter==2)
				out+=filter2(out1,out2);
			else if(filter==3)
				out+=filter3(out1,out2);
			
			temp2 = out;
			if( temp2 < -32768 )
			{
				temp2 = -32768;
			}
			else if( temp2 > 32767 )
			{
				temp2 = 32767;
			}
			output[now] = temp2;
			outb2=outb1;
			outb1=temp2;
			
			now++;
        }
		if (end) {
			if (remainloop > 0) {
				remainloop--;
				end = 0;
				src = loopaddr;
			}
		}
	}
	return now;
}

int emphasis(short *data, unsigned int length)
{
	unsigned int	i;
	double	now=.0,a1=.0,a2=.0,*buf,max=.0;
	
	buf=(double*)malloc(length*sizeof(double));
	
	for (i=0; i<length; i++) {
		now=data[i];
		buf[i] = now*1.72713771313805 - a1*0.63418337904287769 - a2*0.10054429474861125;	//�����t�B���^�̋t�X�y�N�g����LPC�W�����
		a2=a1;
		a1=buf[i];
		if (max < abs(buf[i]))
			max=abs(buf[i]);
	}
	if (max > 32767) {
		for (i=0; i<length; i++)
			data[i]=buf[i]*32767/max;
	} else {
		for (i=0; i<length; i++)
			data[i]=buf[i];
	}
	
	free(buf);
	return 0;
}

