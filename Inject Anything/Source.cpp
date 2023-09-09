#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <string>
#include <vector>

bool InjectDLL(const std::string& processName, const std::string& dllPath) {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    DWORD targetProcessId = 0;

    do {
        if (processName.compare(pe32.szExeFile) == 0) {
            targetProcessId = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    if (targetProcessId == 0) {
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);
    if (hProcess == NULL) {
        return false;
    }

    LPVOID remoteMemory = VirtualAllocEx(hProcess, NULL, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteMemory == NULL) {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, remoteMemory, dllPath.c_str(), dllPath.length() + 1, NULL)) {
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32 == NULL) {
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPVOID loadLibraryAddr = reinterpret_cast<LPVOID>(GetProcAddress(hKernel32, "LoadLibraryA"));
    if (loadLibraryAddr == NULL) {
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryAddr), remoteMemory, 0, NULL);
    if (hRemoteThread == NULL) {
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hRemoteThread, INFINITE);

    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);

    return true;
}


int main() {
 
  
    std::string processName = "vlc.exe";    // Replace with the name of the target process
    std::string dllPath = "E:\\Code\\Inject Anything\\x64\\Debug\\Inline.dll";    // Replace with the path to your DLL
    
    
    if (InjectDLL(processName, dllPath)) {
        ExitProcess(0);
    }
    else {
        ExitProcess(0);
    }

    return 0;
}
