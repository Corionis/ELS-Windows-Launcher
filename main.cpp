#include "string"
#include <windows.h>

#define EXE_NAME_LENGTH 17  // The length of this executable name, e.g. ELS-Navigator.exe

/**
 * Windows Launcher for ELS, https://github.com/Corionis/ELS
 *
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR pCmdLine, int nCmdShow)
{
    // Declare and initialize process blocks
    PROCESS_INFORMATION processInformation;
    STARTUPINFO startupInfo;

    // Get the passed command line
    std::string args = pCmdLine;

    // Get the path to this executable
    TCHAR dest[MAX_PATH];
    DWORD length = GetModuleFileName(NULL, dest, MAX_PATH);
    std::string path = dest;

    // Remove the executable name
    path = path.substr(0, length - EXE_NAME_LENGTH);

    // Add path to ELS jar
//    path = path + "rt\\bin\\javaw.exe -jar " + path + "bin\\ELS.jar ";
    path = "rt\\bin\\javaw.exe -jar \"" + path + "bin\\ELS.jar\" ";

    // Assemble the command line with arguments
    std::string cmd = path + args;

    pCmdLine = const_cast<char *>(cmd.c_str());

    memset(&processInformation, 0, sizeof(processInformation));
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    int result = ::CreateProcess
            (
                    NULL,
                    pCmdLine,
                    NULL,
                    NULL,
                    FALSE,
                    NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                    NULL,
                    NULL,
                    &startupInfo,
                    &processInformation
            );

    // Wait until child process exits.
    //WaitForSingleObject(processInformation.hProcess, INFINITE);

    // Close process and thread handles.
    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);

    return 0;
}
