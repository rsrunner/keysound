#ifndef _DSOUND_H_
#define _DSOUND_H_

#include <dsound.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>

// what kind of clinically insane person would make LPDIRECTSOUND isntead of Just A Pointer
IDirectSound8 *dsound;
IDirectSoundBuffer *dsprimarybuffer;

#define MRIFFID(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))

int init_dsound(void);
void shutdown_dsound(void);
int vol_to_db(int vol);
IDirectSoundBuffer8* load_wav(const char *filename);
HRESULT play_wav(IDirectSoundBuffer8* sound);

#endif