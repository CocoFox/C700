/*
**	fft_czt.h -- header file for fft.c and czt.c
**
**	Public domain by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#ifndef FFT_CZT_H
#define FFT_CZT_H

	/* �f�[�^�����^ REAL �̒�` (float or double) */
	/* ���̌^�� long double �ɂ���ꍇ�́Aczt.c / fft.c �Ŏg���Ă���
	 * �O�p�֐� sin() / cos() �� long double �� (C99 �Ή��R���p�C���ł�
	 * sinl / cosl) �ɕύX����K�v���� */
typedef double REAL;

	/* FFT�v�Z�p ���\�ێ��\���� */
typedef struct {
	int  samples;		/* �W�{�_�̐�( 4�ȏ�� 2�̐�����Ɍ���) */
	int  *bitrev;		/* �r�b�g���]�\ - �v�f����(samples) */
	REAL *sintbl;		/* �O�p�֐��\ - �v�f����(samples*3/4) */
} fft_struct;

	/* CZT�v�Z�p ���\�ێ��\���� */
typedef struct {
	fft_struct fft;		/* ������ FFT�v�Z�p */
	int  samples;		/* �W�{�_�̐� */
	int  samples_out;	/* �o�͂���W�{�_�̐� */
	int  samples_ex;	/* 2�̐�����ŁA(samples + samples_out) <= samples_ex
						 * �ł���ŏ��̐� */
	REAL *wr, *wi;		/* �d�݃f�[�^ - �v�f����(samples) */
	REAL *vr, *vi;		/* �C���p���X�����f�[�^ - �v�f����(samples_ex) */
	REAL *tr, *ti;		/* ��Ɨp�̈� - �v�f����(samples_ex) */
} czt_struct;

	/* fft.c */
int fft_init(fft_struct *fftp, int n);
void fft_end(fft_struct *fftp);
void fft(fft_struct *fftp, int inv, REAL re[], REAL im[]);

	/* czt.c */
int czt_init(czt_struct *cztp, int n, int no);
void czt_end(czt_struct *cztp);
void czt(czt_struct *cztp, int inv, REAL re[], REAL im[]);

int estimatebasefreq(short *src, int length);

#endif	/* FFT_CZT_H */
