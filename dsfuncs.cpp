#include "dsfuncs.h"

// can't be arsed to actually write code for dsound so im just copying from here
// https://github.com/open-watcom/open-watcom-v2/blob/master/bld/src/directx/cpp/dsound/dsound.cpp

int init_dsound(void)
{
        HRESULT err;
        DSCAPS dsoundcaps;
        DSBUFFERDESC dsbdesc;
        WAVEFORMATEX wfx;

        err = DirectSoundCreate8(NULL,  &dsound, NULL);
        if (err != DS_OK)
                return 1;

        err = dsound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
        if (err != DS_OK)
                return 1;

        memset(&dsoundcaps, 0, sizeof dsoundcaps);
        dsoundcaps.dwSize = sizeof dsoundcaps;

        err = IDirectSound8_GetCaps(dsound, &dsoundcaps);
        if (err != DS_OK)
                return 1;

        // Create and set primary buffer format
        memset(&dsbdesc, 0, sizeof dsbdesc);

        dsbdesc.dwSize = sizeof dsbdesc;
        dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

        err = dsound->CreateSoundBuffer(&dsbdesc, &dsprimarybuffer, NULL);
        if (err != DS_OK)
                return 1;

        memset(&wfx, 0, sizeof wfx);
        wfx.cbSize = sizeof wfx;
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        err = dsprimarybuffer->SetFormat(&wfx);
        if (err != DS_OK)
                return 1;

        //force dsound to ALWAYS keep DMA active
        err = dsprimarybuffer->Play(0, 0, DSBPLAY_LOOPING);
        if (err != DS_OK)
                return 1;

        return 0;
}

void shutdown_dsound()
{
        if (dsprimarybuffer)
        {
                dsprimarybuffer->Stop();
                dsprimarybuffer->Release();
        }
        if (dsound) dsound->Release();
}

int vol_to_db(int vol)
{
        if (vol)
                return (int)(20.0 * 100.0 * log10((double)vol/127.0));
        return -10000;
}

IDirectSoundBuffer8* load_wav(const char *filename)
{
    unsigned int *riff;
    FILE *f;
    long len;
    void *sfx;

    // Read in the supplied file
    f = fopen(filename, "rb");
    if (f == NULL){
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    sfx = malloc(len);
    if (sfx == NULL){
        fclose(f);
        return NULL;
    }

    fread(sfx, 1, len, f);
    fclose(f);

    riff = (unsigned int *)sfx;

    // Check it's a RIFF WAV file
    IDirectSoundBuffer8 *sfxbuffer8=NULL;
    
    if (*riff == MRIFFID('R','I','F','F'))
    {
        IDirectSoundBuffer *sfxbuffer;
        DSBUFFERDESC dsbufferdesc;
        void *data1, *data2;
        DWORD size1, size2;
        unsigned char *wav_data;
        unsigned int wav_size;
        WAVEFORMATEX *wav_hdr;
        HRESULT err;

        // Strip through the WAV header looking for the WAVEFORMAT data

        riff += 3;//skip length and id
        while (*riff != MRIFFID('f','m','t',' '))
            riff = (unsigned int *)((unsigned char *)riff + 8 + *(riff+1));

        wav_hdr = (WAVEFORMATEX *)(riff+2);

        // Strip through the WAV header looking for the sample data

        riff = (unsigned int *)((unsigned char *)riff + 8 + *(riff+1));
        while (*riff != MRIFFID('d','a','t','a'))
            riff = (unsigned int *)((unsigned char *)riff + 8 + *(riff+1));

        wav_data = (unsigned char *)(riff+2);
        wav_size = *(riff+1);

        // Create a suitable buffer
        memset(&dsbufferdesc, 0, sizeof dsbufferdesc);
        dsbufferdesc.dwSize = sizeof dsbufferdesc;
        dsbufferdesc.dwFlags = DSBCAPS_GLOBALFOCUS|DSBCAPS_STATIC|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRLPAN;
        dsbufferdesc.dwBufferBytes = wav_size;
        dsbufferdesc.lpwfxFormat = wav_hdr;

        err = dsound->CreateSoundBuffer(&dsbufferdesc, &sfxbuffer, NULL);
        if (err != DS_OK)
        {
            free(sfx);
            return NULL;
        }
        // Get the DirectSound8 interface to the buffer
        err = sfxbuffer->QueryInterface(IID_IDirectSoundBuffer8, (void **)&sfxbuffer8);
        if (sfxbuffer8 == NULL)
        {
            free(sfx);
            return NULL;
        }
        // Release the base object
        err = sfxbuffer->Release();

        // Lock the buffer
        err = sfxbuffer8->Lock(0, 0, (void **)&data1, &size1, (void **)&data2, &size2, DSBLOCK_ENTIREBUFFER);
        if (err != DS_OK)
        {
            free(sfx);
            return NULL;
        }

        // Copy wav data from file into DirectSound buffer
        memcpy(data1, wav_data, wav_size);

        err = sfxbuffer8->Unlock(data1, size1, data2, size2);

        // Set the sfx volume
        err = sfxbuffer8->SetVolume(vol_to_db(127));
    }

    // Tidy up
    free(sfx);
    return sfxbuffer8;
}

HRESULT play_wav(IDirectSoundBuffer8* sound){
    if(sound == NULL){
        return 0;
    }
    HRESULT err = sound->Play(0, 0, 0);
    return err;
}