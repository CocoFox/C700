/*
 *  PGChunk.h
 *  Chip700
 *
 *  Created by osoumen on 12/10/17.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Chip700defines.h"
#include "FileAccess.h"

class PGChunk : public FileAccess {
public:
	PGChunk( int allocMemSize );
	virtual ~PGChunk();
	
	bool				SetDataFromVP( int pgnum, const VoiceParams *vp );
	bool				WriteDataToVP( int pgnum, VoiceParams *vp );
	const unsigned char	*GetDataPtr() const { return m_pData; }
	int					GetDataSize() const { return mDataUsed; }
	
private:
	typedef struct {
		int		type;
		long	size;	//�w�b�_���������f�[�^�T�C�Y
	};
	
	bool			writeChunk( int type, const void* data, int byte );	//�e�ʕs���őS�����������߂Ȃ��Ƃ���false

	unsigned char	*m_pData;
	int				mDataSize;
	int				mDataUsed;
	int				mDataPos;
};
