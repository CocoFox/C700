/*
**	fft.c -- Fast Fourier Tramsform (FFT)
**	���Ԏ��Ԉ���FFT (Cooley--Tukey�̃A���S���Y��)
**
**	�w�b����ɂ��ŐV�A���S���Y�����T�x�i�������F�E���j�Ɍf�ڂ�
**	�v���O���������ς��ė��p�����Ē����܂���.
**
**	Revised by MIYASAKA Masaru <alkaid@coral.ocn.ne.jp> (Sep 15, 2003)
**
**	[TAB4]
*/

#include <stdlib.h>
#include <math.h>

#include "fft_czt.h"

#undef PI
#define PI	3.1415926535897932384626433832795L	/* �~����(��) */


/*
**	�O�p�֐��\�����.
*/
static void make_sintbl(int n, REAL sintbl[])
{
	/* �����x���d�v�łȂ��Ȃ� double �ł��������� */
	long double r, d = (2 * PI) / n;
	int i, n2 = n / 2, n4 = n / 4, n8 = n / 8;

	for (i = 0; i <= n8; i++) {
		r = d * i;
		sintbl[i]      = sin(r);			/* sin(2*PI*i/n) */
		sintbl[n4 - i] = cos(r);
	}
	for (i = 0; i < n4; i++) {
		sintbl[n2 - i] =  sintbl[i];
		sintbl[i + n2] = -sintbl[i];
	}
}


/*
**	�r�b�g���]�\�����.
*/
static void make_bitrev(int n, int bitrev[])
{
	int i = 0, j = 0, k;
	int n2 = n / 2;

	for (;;) {
		bitrev[i] = j;
		if (++i >= n) break;
		k = n2;
		while (k <= j) { j -= k; k /= 2; }
		j += k;
	}
}


/*
**	FFT�v�Z�p�\���̂ɑ΂��A�W�{�� n �p�̐��\�f�[�^���쐬����B
**
**	fftp	= FFT�v�Z�p�\���̂ւ̃|�C���^
**	n		= �W�{�_�̐� ( 4�ȏ�� 2�̐�����Ɍ���)
**	return	= 0:����I�� 1:n�������Ȑ� 2:�������s��
*/
int fft_init(fft_struct *fftp, int n)
{
	int i;

	for (i = 4; i < n; i *= 2) ;
	if (i != n) return 1;

	fftp->samples = n;
	fftp->sintbl  = (REAL*)malloc((n / 4 * 3) * sizeof(REAL));
	fftp->bitrev  = (int*)malloc(n * sizeof(int));
	if (fftp->sintbl == NULL || fftp->bitrev == NULL) {
		fft_end(fftp);
		return 2;
	}
	make_sintbl(n, fftp->sintbl);
	make_bitrev(n, fftp->bitrev);

	return 0;
}


/*
**	FFT�v�Z�p�\���̂̐��\�f�[�^���������Ă��̃������̈���J������B
**
**	fftp	= FFT�v�Z�p�\���̂ւ̃|�C���^
*/
void fft_end(fft_struct *fftp)
{
	if (fftp->sintbl != NULL) { free(fftp->sintbl); fftp->sintbl = NULL; }
	if (fftp->bitrev != NULL) { free(fftp->bitrev); fftp->bitrev = NULL; }

	fftp->samples = 0;
}


/*
**	����Fourier�ϊ� (FFT) (Cooley--Tukey�̃A���S���Y��).
**	re[] ������, im[] ������. ���ʂ� re[],im[] �ɏ㏑�������.
**	inv!=0 (=TRUE) �Ȃ�t�ϊ����s��. fftp �ɂ�, �v�Z�p�f�[�^��
**	�����Ă���\���̂��w�肷��.
*/
void fft(fft_struct *fftp, int inv, REAL re[], REAL im[])
{
	REAL si, co, dr, di, t;
	int i, j, k, ik, h, d, k2, n, n4;

	n  = fftp->samples;
	n4 = n / 4;

	for (i = 0; i < n; i++) {		/* �r�b�g���] */
		j = fftp->bitrev[i];
		if (i < j) {
			t = re[i];  re[i] = re[j];  re[j] = t;
			t = im[i];  im[i] = im[j];  im[j] = t;
		}
	}
	for (k = 1; k < n; k = k2) {	/* �ϊ� */
		h = 0;  k2 = k + k;  d = n / k2;
		for (j = 0; j < k; j++) {
			co = fftp->sintbl[h + n4];
			si = fftp->sintbl[h];
			if (inv) si = -si;
			for (i = j; i < n; i += k2) {
				ik = i + k;
				dr = si * im[ik] + co * re[ik];
				di = co * im[ik] - si * re[ik];
				re[ik] = re[i] - dr;  re[i] += dr;
				im[ik] = im[i] - di;  im[i] += di;
			}
			h += d;
		}
	}
	if (!inv) {						/* �t�ϊ��łȂ��Ȃ�n�Ŋ��� */
		t = 1.0 / n;			/* �t����������(���Z�͒x���̂�) */
		for (i = 0; i < n; i++) {
			re[i] *= t;
			im[i] *= t;
		}
	}
}

