//
//  C700_CocoaViewFactory.m
//  Chip700
//
//  Created by osoumen on 12/10/01.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <CAAUParameter.h>
#import "C700_CocoaViewFactory.h"
#import "C700Edit.h"

@implementation C700_CocoaViewFactory

// version 0
- (unsigned) interfaceVersion {
	return 0;
}

// string description of the Cocoa UI
- (NSString *) description {
	return @" Cocoa View";
}

- (void)dealloc {
	[super dealloc];
}

- (NSView *)uiViewForAudioUnit:(AudioUnit)inAU withSize:(NSSize)inPreferredSize
{
	NSRect r;
	r.origin.x = 0;
	r.origin.y = 0;
	r.size.width = 200;
	r.size.height = 200;
	
    C700_CocoaView *returnView = [[C700_CocoaView alloc] initWithFrame:r audioUnit:inAU];
	[returnView setEditorFrame];
	
    return [returnView autorelease];
}

@end


void ParameterListenerDispatcher(void *inRefCon, void *inObject, const AudioUnitParameter *inParameter, Float32 inValue)
{
	C700Edit *edit = (C700Edit *)inRefCon;
    edit->setParameter(inParameter->mParameterID, inValue);
}

@implementation C700_CocoaView

- (C700_CocoaView*)initWithFrame:(NSRect)frameRect audioUnit:(AudioUnit)inAU
{
	C700_CocoaView	*view = [super initWithFrame:frameRect];
	
	mAU = inAU;
#ifndef __LP64__
	CFrame::setCocoaMode(true);
#endif
	editor = new C700Edit(inAU);
	
	if ( editor ) {
		editor->open(self);
	}
	
	//�^�C�}�[�̐ݒ�
	timer = [NSTimer scheduledTimerWithTimeInterval:1.0/24
											 target:self
										   selector:@selector(respondToEventTimer:)
										   userInfo:nil
											repeats:YES];	
	
	//�p�����[�^�[���X�i�[�̓o�^
	[self _addListeners];
	editor->SetParameterListener(mParameterListener);
	
	//�ݒ�l�̔��f
	[self _synchronizeUIWithParameterValues];
	
	return view;
}

- (void)_addListeners
{
	NSAssert(	AUListenerCreate(	ParameterListenerDispatcher, editor, 
								 CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0.010, // 10 ms
								 &mParameterListener	) == noErr,
			 @"[CocoaView _addListeners] AUListenerCreate()");
	
    for (int i = 0; i < kNumberOfParameters; ++i) {
		AudioUnitParameter parameter = { 0, 0, kAudioUnitScope_Global, 0 };
        parameter.mAudioUnit = mAU;
		parameter.mParameterID = i;
        NSAssert (	AUListenerAddParameter (mParameterListener, NULL, &parameter) == noErr,
				  @"[CocoaView _addListeners] AUListenerAddParameter()");
    }
}

- (void)_removeListeners
{
	for (int i = 0; i < kNumberOfParameters; ++i) {
		AudioUnitParameter parameter = { mAU, i, kAudioUnitScope_Global, 0 };
        NSAssert (	AUListenerRemoveParameter(mParameterListener, NULL, &parameter) == noErr,
				  @"[CocoaView _removeListeners] AUListenerRemoveParameter()");
    }
	NSAssert (	AUListenerDispose(mParameterListener) == noErr,
			  @"[CocoaView _removeListeners] AUListenerDispose()");
}

- (void)_synchronizeUIWithParameterValues
{
	Float32 value;
    
    for (int i = 0; i < kNumberOfParameters; ++i)
	{
		AudioUnitParameter parameter = { mAU, i, kAudioUnitScope_Global, 0 };
        // only has global parameters
        NSAssert (	AudioUnitGetParameter(mAU, parameter.mParameterID, kAudioUnitScope_Global, 0, &value) == noErr,
				  @"[CocoaView synchronizeUIWithParameterValues] (x.1)");
        NSAssert (	AUParameterSet (mParameterListener, self, &parameter, value, 0) == noErr,
				  @"[CocoaView synchronizeUIWithParameterValues] (x.2)");
        NSAssert (	AUParameterListenerNotify (mParameterListener, self, &parameter) == noErr,
				  @"[CocoaView synchronizeUIWithParameterValues] (x.3)");
		
		//�ő�l�A�ŏ��l�A�f�t�H���g�l���R���g���[���ɔ��f
		CAAUParameter tParam(mAU, i, kAudioUnitScope_Global, 0);
		editor->SetParameterInfo( i, tParam.ParamInfo().minValue, tParam.ParamInfo().maxValue, tParam.ParamInfo().defaultValue );
    }
}

- (void)respondToEventTimer:(NSTimer*)inTimer
{
	if (editor) {
		editor->doIdleStuff();
	}
}

- (void)setEditorFrame
{
	CRect fsize;
	if ( editor ) {
		fsize = editor->getFrame()->getViewSize(fsize);
	}
	NSRect r;
	r.origin.x = fsize.left;
	r.origin.y = fsize.top;
	r.size.width = fsize.getWidth();
	r.size.height = fsize.getHeight();
	[self setFrame:r];	
}

- (void)dealloc
{
	//�p�����[�^���X�i�[�̍폜
	[self _removeListeners];
	
	//�^�C�}�[�̒�~
	if ( timer ) {
		[timer invalidate];
	}
	
	if (editor) {
		editor->close();
		delete editor;
		editor = NULL;
	}
	[super dealloc];
}

@end
