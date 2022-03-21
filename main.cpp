#include <vector>
#include <string>
#include <windows.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <mmsystem.h>

#include "dsfuncs.h"

HHOOK keyhook;
HHOOK mousehook;
MSG msg;
std::vector<std::string> SOUNDS;
std::vector<IDirectSoundBuffer8*> DSOUNDS;
HANDLE hFind;
WIN32_FIND_DATA ffd;

void funnynoise(){
    /*char file[MAX_PATH];
    std::sprintf(file, "sounds/%s", SOUNDS[std::rand() % (SOUNDS.size())].c_str());
    IDirectSoundBuffer8* sound = load_wav(file);
    if(sound != NULL){
        sound->Play(0, 0, 0);
    }*/
    play_wav(DSOUNDS[std::rand() % (DSOUNDS.size())]);
}

LRESULT __stdcall handlekey(int code, WPARAM wParam, LPARAM lParam){
    if(code == 0 && wParam == WM_KEYDOWN){
        funnynoise();
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT __stdcall handlemouse(int code, WPARAM wParam, LPARAM lParam){
    if(code == 0 && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)){
        funnynoise();
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

int main(int argc, char* argv[]){
    int status = 0;
    status = init_dsound();
    if(status != 0){
        return 1;
    }
    std::srand(std::time(NULL));
    hFind = FindFirstFile("sounds\\*.*", &ffd);

    do
    {
        if(ffd.cFileName != "." && ffd.cFileName != ".." && (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            SOUNDS.push_back(ffd.cFileName);
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);

    for (int i = 0; i < SOUNDS.size(); i++)
    {
        char file[MAX_PATH];
        std::sprintf(file, "sounds/%s", SOUNDS[std::rand() % (SOUNDS.size())].c_str());
        DSOUNDS.push_back(load_wav(file));
    }

    keyhook = SetWindowsHookEx(WH_KEYBOARD_LL, handlekey, NULL, 0);
    keyhook = SetWindowsHookEx(WH_MOUSE_LL, handlemouse, NULL, 0);

    while(GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    shutdown_dsound();
    if(keyhook){
        UnhookWindowsHookEx(keyhook);
    }
    if(mousehook){
        UnhookWindowsHookEx(mousehook);
    }
    return 0;
}

