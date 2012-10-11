/*
 *  CFBRRFile.cpp
 *  Chip700
 *
 *  Created by ���c ���F on 12/10/11.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "Chip700defines.h"
#include "CFBRRFile.h"

//-----------------------------------------------------------------------------
CFBRRFile::CFBRRFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
{
}

//-----------------------------------------------------------------------------
CFBRRFile::~CFBRRFile()
{
	if ( mIsLoaded ) {
		CFRelease(mPropertydata);
	}
}

//-----------------------------------------------------------------------------
bool CFBRRFile::Load()
{
	if ( mPath == NULL ) return false;
	
	//CFURL���쐬
	CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);
}

//-----------------------------------------------------------------------------
bool CFBRRFile::Write()
{
	if ( IsLoaded() == false ) return false;
	
	//CFURL���쐬
	CFURLRef	savefile = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)mPath, strlen(mPath), false);

	//�o�C�i���`���ɕϊ�
	CFWriteStreamRef	filestream=CFWriteStreamCreateWithFile(NULL,savefile);
	if (CFWriteStreamOpen(filestream)) {
		CFPropertyListWriteToStream(mPropertydata,filestream,kCFPropertyListBinaryFormat_v1_0,NULL);
		CFWriteStreamClose(filestream);
	}
	CFRelease(filestream);
	CFRelease(savefile);
	
	return true;
}

//-----------------------------------------------------------------------------
void CFBRRFile::SetDictionaryData( CFDictionaryRef propertydata )
{
#if MAC
	if ( mIsLoaded ) {
		CFRelease(mPropertydata);
	}
	mPropertydata = propertydata;
	CFRetain(mPropertydata);
	mIsLoaded = true;
#endif
}
