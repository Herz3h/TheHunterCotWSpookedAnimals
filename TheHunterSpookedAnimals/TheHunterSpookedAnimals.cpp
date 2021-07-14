// TheHunterSpookedAnimals.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <Psapi.h>
#include "TheHunterSpookedAnimals.h"

using namespace std;

int main()
{
    DWORD access = PROCESS_VM_READ | PROCESS_QUERY_INFORMATION;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    int pid = NULL;

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof entry;

    if (!Process32FirstW(snap, &entry)) {
        return 0;
    }

    wchar_t processName[] = L"theHunterCotW_F.exe";
    do {
        if (std::wstring(entry.szExeFile) == std::wstring(processName)) {
            pid = entry.th32ProcessID;
        }
    } while (Process32NextW(snap, &entry)); 

    HANDLE proc = OpenProcess(access, FALSE, pid);
    HMODULE hMods[1024];
    HMODULE hModule = NULL;
    DWORD cbNeeded;
    unsigned int i;

    if (EnumProcessModulesEx(proc, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_64BIT))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleFileNameEx(proc, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                wstring wstrModName = szModName;
                wstring wstrModContain = wstring(processName);
                if (wstrModName.find(wstrModContain) != string::npos)
                {
                    hModule = hMods[i];
                    break;
                }
            }
        }
    }

    if (hModule == NULL)
    {
        //std::cout << GetLastError() << endl;
        CloseHandle(proc);
        //std::cout << "Couldn't get module" << endl;
        return -1;
    }
    
    DWORD cb;
    MODULEINFO modInfo;

    GetModuleInformation(proc, hModule, &modInfo, sizeof(modInfo));
    
    SIZE_T nbBytesRead;

    DWORD64 audioOffset = 0xf7c48ed9;
    DWORD64 smellOffset = 0xe8988f6b;
    DWORD64 sightOffset = 0xf1b73639;

    LPVOID audioAddr = getSpookedOffset(modInfo, proc, audioOffset);
    LPVOID smellAddr = getSpookedOffset(modInfo, proc, smellOffset);
    LPVOID sightAddr = getSpookedOffset(modInfo, proc, sightOffset);

        DWORD previousAudioValue = -1;
        DWORD previousSmellValue = -1;
        DWORD previousSightValue = -1;
        
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        while (true)
        {
            bool changed = false;
            DWORD nbSpookedAnimals;
            ReadProcessMemory(proc, audioAddr, &nbSpookedAnimals, 4, &nbBytesRead);
            if (previousAudioValue != -1 && previousAudioValue != nbSpookedAnimals)
            {
                SetConsoleTextAttribute(hConsole, 10);

                changed = true;
            }
            else
            {
                SetConsoleTextAttribute(hConsole, 7);
            }
            previousAudioValue = nbSpookedAnimals;
            std::cout << "Spooked Audio: " << nbSpookedAnimals << endl;

            ReadProcessMemory(proc, sightAddr, &nbSpookedAnimals, 4, &nbBytesRead);
            if (previousSightValue != -1 && previousSightValue != nbSpookedAnimals)
            {
                SetConsoleTextAttribute(hConsole, 10);

                changed = true;
            }
            else
            {
                SetConsoleTextAttribute(hConsole, 7);
            }
            previousSightValue = nbSpookedAnimals;

            std::cout << "Spooked Sight: " << nbSpookedAnimals << endl;


            ReadProcessMemory(proc, smellAddr, &nbSpookedAnimals, 4, &nbBytesRead);
            if (previousSmellValue != -1 && previousSmellValue != nbSpookedAnimals)
            {
                SetConsoleTextAttribute(hConsole, 10);

                changed = true;
            }
            else
            {
                SetConsoleTextAttribute(hConsole, 7);
            }
            previousSmellValue = nbSpookedAnimals;
            std::cout << "Spooked Smell: " << nbSpookedAnimals << endl;




            if (changed)
            {
                Beep(523, 500); // 523 hertz (C5) for 500 milliseconds     
            }
            Sleep(1000);

            std::system("CLS");
        }

        char ok;
        std::cin >> ok;

    

    CloseHandle(proc);
}

LPVOID getSpookedOffset(MODULEINFO& modInfo, const HANDLE& proc, DWORD64 spookedOffset)
{
    HANDLE rcx, firstSubStructureAddress, secondSubSubStructure, statStructure, currentStatStructure, finalStatStructure, rbx, nbSpookedFromAudio;
    DWORD valueAt0x20;
    BYTE byte;
    HANDLE address;
    SIZE_T nbBytesRead;
    DWORD64 offset = 0x20B83A8;
    LPVOID baseAddr = modInfo.lpBaseOfDll;
    char* addr = static_cast<char*>(baseAddr) + offset;
    ReadProcessMemory(proc, addr, &rcx, 8, &nbBytesRead);

    addr = static_cast<char*>(rcx) + 0x30;
    ReadProcessMemory(proc, addr, &firstSubStructureAddress, 8, &nbBytesRead);

    addr = static_cast<char*>(firstSubStructureAddress) + 0x8;
    ReadProcessMemory(proc, addr, &secondSubSubStructure, 8, &nbBytesRead);

    addr = static_cast<char*>(secondSubSubStructure) + 0x19;
    ReadProcessMemory(proc, addr, &byte, 1, &nbBytesRead);

    bool found = true;
    statStructure = firstSubStructureAddress;
    if (byte == 0)
    {
        do
        {

            addr = static_cast<char*>(secondSubSubStructure) + 0x20;
            ReadProcessMemory(proc, addr, &valueAt0x20, 4, &nbBytesRead);

            if ((DWORD)valueAt0x20 < spookedOffset)
            {
                addr = static_cast<char*>(secondSubSubStructure) + 0x10;
                ReadProcessMemory(proc, addr, &currentStatStructure, 8, &nbBytesRead);
            }
            else
            {
                addr = static_cast<char*>(secondSubSubStructure);
                ReadProcessMemory(proc, addr, &currentStatStructure, 8, &nbBytesRead);

                statStructure = secondSubSubStructure;
            }

            secondSubSubStructure = currentStatStructure;

            addr = static_cast<char*>(currentStatStructure) + 0x19;
            ReadProcessMemory(proc, addr, &byte, 1, &nbBytesRead);
        } while (byte == 0);
    }
    addr = static_cast<char*>(statStructure) + 0x19;
    ReadProcessMemory(proc, addr, &byte, 1, &nbBytesRead);

    addr = static_cast<char*>(statStructure) + 0x20;
    ReadProcessMemory(proc, addr, &valueAt0x20, 4, &nbBytesRead);
    if (byte == 0 && (DWORD)valueAt0x20 <= spookedOffset)
    {
        found = true;
    }
    else
    {
        found = false;
    }

    if (!found)
    {
        statStructure = firstSubStructureAddress;
    }

    if (statStructure != firstSubStructureAddress)
    {
        addr = static_cast<char*>(statStructure) + 0x28 + 0x8;
        ReadProcessMemory(proc, addr, &byte, 1, &nbBytesRead);

        if (byte == 0)
        {
            addr = static_cast<char*>(statStructure) + 0x28 + 0x30;
        }
    }

    return addr;
}
