/*
 *  AudioFile.cpp
 *  Chip700
 *
 *  Created by ���c ���F on 12/10/10.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "AudioFile.h"
#include "vstgui.h"

#if MAC
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioConverter.h>
#endif

//-----------------------------------------------------------------------------
AudioFile::AudioFile( const char *path, bool isWriteable )
: FileAccess(path, isWriteable)
, m_pAudioData( NULL )
, mLoadedSamples( 0 )
{
}

//-----------------------------------------------------------------------------
AudioFile::~AudioFile()
{
	if ( m_pAudioData ) {
		free(m_pAudioData);
	}
}

//-----------------------------------------------------------------------------
bool AudioFile::IsVarid()
{
	AudioFileID mAudioFileID;
	FSRef	ref;
	Boolean	isDirectory=false;
	FSPathMakeRef((const UInt8*)GetFilePath(), &ref, &isDirectory);
	OSStatus err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
	if (err == noErr) {
		AudioFileClose(mAudioFileID);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool AudioFile::Load()
{
#if MAC
	AudioFileID mAudioFileID;
    AudioStreamBasicDescription mFileDescription, outputFormat;
    SInt64 dataSize64;
    UInt32 dataSize;
	
	OSStatus err;
	UInt32 size;
	
    // �t�@�C�����J��
	FSRef	ref;
	Boolean	isDirectory=false;
	FSPathMakeRef((const UInt8*)GetFilePath(), &ref, &isDirectory);
	
	err = AudioFileOpen(&ref, fsRdPerm, 0, &mAudioFileID);
    if (err) {
        //NSLog(@"AudioFileOpen failed");
        return false;
    }
	
    // �J�����t�@�C���̊�{���� mFileDescription ��
    size = sizeof(AudioStreamBasicDescription);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyDataFormat, 
							   &size, &mFileDescription);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	
    // �J�����t�@�C���̃f�[�^���̃o�C�g���� dataSize ��
    size = sizeof(SInt64);
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, 
							   &size, &dataSize64);
    if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
    dataSize=(UInt32)dataSize64;
	
	AudioFileTypeID	fileTypeID;
	size = sizeof( AudioFileTypeID );
	err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFileFormat, &size, &fileTypeID);
	if (err) {
        //NSLog(@"AudioFileGetProperty failed");
        AudioFileClose(mAudioFileID);
        return false;
    }
	
	mInstData.basekey	= 60;
	mInstData.lowkey	= 0;
	mInstData.highkey	= 127;
	mInstData.loop		= 0;
	
	//���[�v�|�C���g�̎擾
	Float64		st_point=0.0,end_point=0.0;
	if ( fileTypeID == kAudioFileAIFFType || fileTypeID == kAudioFileAIFCType ) {
		//INST�`�����N�̎擾
		AudioFileGetUserDataSize(mAudioFileID, 'INST', 0, &size);
		if ( size > 4 ) {
			UInt8	*instChunk = (UInt8*)malloc(size);
			AudioFileGetUserData(mAudioFileID, 'INST', 0, &size, instChunk);
			
			//MIDI���̎擾
			mInstData.basekey = instChunk[0];
			mInstData.lowkey = instChunk[2];
			mInstData.highkey = instChunk[3];
			
			if ( instChunk[9] > 0 ) {	//���[�v�t���O���m�F
				//�}�[�J�[�̎擾
				UInt32	writable;
				err = AudioFileGetPropertyInfo(mAudioFileID, kAudioFilePropertyMarkerList,
											   &size, &writable);
				if (err) {
					//NSLog(@"AudioFileGetPropertyInfo failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				AudioFileMarkerList	*markers = (AudioFileMarkerList*)malloc(size);
				err = AudioFileGetProperty(mAudioFileID, kAudioFilePropertyMarkerList, 
										   &size, markers);
				if (err) {
					//NSLog(@"AudioFileGetProperty failed");
					AudioFileClose(mAudioFileID);
					return NULL;
				}
				
				//���[�v�|�C���g�̐ݒ�
				for (unsigned int i=0; i<markers->mNumberMarkers; i++) {
					if (markers->mMarkers[i].mMarkerID == instChunk[11] ) {
						st_point = markers->mMarkers[i].mFramePosition;
					}
					else if (markers->mMarkers[i].mMarkerID == instChunk[13] ) {
						end_point = markers->mMarkers[i].mFramePosition;
					}
					CFRelease(markers->mMarkers[i].mName);
				}
				if ( st_point < end_point ) {
					mInstData.loop = 1;
				}
				free( markers );
			}
			free( instChunk );
		}
		
	}
	else if ( fileTypeID == kAudioFileWAVEType ) {
		//smpl�`�����N�̎擾
		AudioFileGetUserDataSize( mAudioFileID, 'smpl', 0, &size );
		if ( size >= sizeof(WAV_smpl) ) {
			UInt8	*smplChunk = (UInt8*)malloc(size);
			AudioFileGetUserData( mAudioFileID, 'smpl', 0, &size, smplChunk );
			WAV_smpl	*smpl = (WAV_smpl *)smplChunk;
			
			smpl->loops = EndianU32_LtoN( smpl->loops );
			
			if ( smpl->loops > 0 ) {
				mInstData.loop = 1;
				mInstData.basekey = EndianU32_LtoN( smpl->note );
				st_point = EndianU32_LtoN( smpl->start );
				end_point = EndianU32_LtoN( smpl->end ) + 1;	//SoundForge���ł͍ŏI�|�C���g���܂߂����
				//end_point = EndianU32_LtoN( smpl->end );	//Peak�ł͂Ȃ���AIFF�Ɠ���
			}
			else {
				mInstData.basekey = EndianU32_LtoN( smpl->note );
			}
			free( smplChunk );
		}
	}
	
    // �g�`�ꎞ�ǂݍ��ݗp���������m��
    char *mFileBuffer;
	if (mInstData.loop) {
		mFileBuffer = (char *)calloc(dataSize+EXPAND_BUFFER*mFileDescription.mBytesPerFrame,sizeof(char));
	}
	else {
		mFileBuffer = (char *)calloc(dataSize,sizeof(char));
	}
	
	// �t�@�C������ǂݍ���
	err = AudioFileReadBytes(mAudioFileID, false, 0, &dataSize, mFileBuffer);
    if (err) {
        //NSLog(@"AudioFileReadBytes failed");
        AudioFileClose(mAudioFileID);
        free(mFileBuffer);
        return false;
    }
    AudioFileClose(mAudioFileID);
	
    //�P�Ubit���m�����̃f�[�^�ɕϊ�
    outputFormat=mFileDescription;
	if (mInstData.loop) {
		UInt32	plusalpha=0, framestocopy;
		while (plusalpha < EXPAND_BUFFER) {
			framestocopy = 
			(end_point-st_point)>(EXPAND_BUFFER-plusalpha)?(EXPAND_BUFFER-plusalpha):end_point-st_point;
			memcpy(mFileBuffer+((int)end_point+plusalpha)*mFileDescription.mBytesPerFrame,
				   mFileBuffer+(int)st_point*mFileDescription.mBytesPerFrame,
				   framestocopy*mFileDescription.mBytesPerFrame);
			plusalpha += framestocopy;
		}
		dataSize += plusalpha*mFileDescription.mBytesPerFrame;
		
		Float64	adjustment = ( (long long)((end_point-st_point)/16) ) / ((end_point-st_point)/16.0);
		outputFormat.mSampleRate *= adjustment;
		st_point *= adjustment;		//16�T���v�����E��FIX����
		end_point *= adjustment;
		
	}
	outputFormat.mFormatID = kAudioFormatLinearPCM;
    outputFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
	outputFormat.mChannelsPerFrame = 1;
	outputFormat.mBytesPerFrame = sizeof(short);
	outputFormat.mBitsPerChannel = 16;
	outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame;
	
    // �o�C�g�I�[�_�[�ϊ��p�̃R���o�[�^��p��
    AudioConverterRef converter;
	err = AudioConverterNew(&mFileDescription, &outputFormat, &converter);
    if (err) {
        //NSLog(@"AudioConverterNew failed");
        free(mFileBuffer);
        return false;
    }
	
	//�T���v�����O���[�g�ϊ��̎����ō��ɐݒ�
	if (mFileDescription.mSampleRate != outputFormat.mSampleRate) {
		size = sizeof(UInt32);
		UInt32	setProp = kAudioConverterQuality_Max;
		AudioConverterSetProperty(converter, kAudioConverterSampleRateConverterQuality, 
								  size, &setProp);
	}
	
    //�o�͂ɕK�v�\���ȃo�b�t�@�T�C�Y�𓾂�
    //if(m_pAudioData) // 2�x�ڈȍ~
    //    free(m_pAudioData);
	UInt32	outputSize = dataSize;
	size = sizeof(UInt32);
	err = AudioConverterGetProperty(converter, kAudioConverterPropertyCalculateOutputBufferSize, 
									&size, &outputSize);
	if (err) {
		//NSLog(@"AudioConverterGetProperty failed");
		free(mFileBuffer);
		AudioConverterDispose(converter);
	}
    m_pAudioData=(short *)malloc(outputSize);
    
    // �o�C�g�I�[�_�[�ϊ�
	AudioConverterConvertBuffer(converter, dataSize, mFileBuffer,
								&outputSize, m_pAudioData);
    if(outputSize == 0) {
        //NSLog(@"AudioConverterConvertBuffer failed");
        free(mFileBuffer);
        AudioConverterDispose(converter);
        return false;
    }
    
    // ��n��
    free(mFileBuffer);
    AudioConverterDispose(converter);
	
	mInstData.lp			= st_point;
	mInstData.lp_end		= end_point;
	mInstData.srcSamplerate	= outputFormat.mSampleRate;
    mLoadedSamples			= outputSize/outputFormat.mBytesPerFrame;
	
	mIsLoaded = true;
	
	return true;
#else
	//TODO : Windows�̃I�[�f�B�I�t�@�C���ǂݍ��ݏ���
	return false;
#endif
}

//-----------------------------------------------------------------------------
short *AudioFile::GetAudioData()
{
	return m_pAudioData;
}

//-----------------------------------------------------------------------------
int AudioFile::GetLoadedSamples()
{
	return mLoadedSamples;
}

//-----------------------------------------------------------------------------
bool AudioFile::GetInstData( InstData *instData )
{
	if ( IsLoaded() ) {
		*instData = mInstData;
		return true;
	}
	return false;
}
