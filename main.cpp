#include "string"
#include <windows.h>
#include <filesystem>
#include <iostream>
#include <direct.h>
#include <shlobj.h>

#define EXE_NAME_LENGTH 17  // The length of this executable name, e.g. ELS-Navigator.exe

/**
 * Windows Launcher for ELS, https://github.com/Corionis/ELS
 *
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR pCmdLine, int nCmdShow)
{
    std::cout << "Starting ELS" << std::endl;

    char cwd[PATH_MAX];

    // Declare and initialize process blocks
    PROCESS_INFORMATION processInformation;
    STARTUPINFO startupInfo;

    // Get the passed command line
    std::string args = pCmdLine;

    // Get the current working directory
    getcwd(cwd, sizeof(cwd));
    std::cout << "cwd: " << cwd << std::endl;

    // Get the path to this executable
    TCHAR dest[MAX_PATH];
    DWORD length = GetModuleFileName(NULL, dest, MAX_PATH);
    std::string path = dest;

    // Remove the executable name
    std::string search(path);
    int p = search.find_last_of("\\");
    if (p >= 0)
        path = path.substr(0, p);
    else
        path = path.substr(0, length - EXE_NAME_LENGTH);
    std::cout << "app: " << path << std::endl;

    // detect -C [configuration directory] argument
    bool dashC = true;
    std::string dashSearch(args);
    p = dashSearch.find("-C");
    if (p >= 0)
    {
        dashC = false;
        p+= 2; // past -C

        // clear cwd
        for (int i = 0; i < sizeof(cwd); ++i)
        {
            cwd[i] = '\0';
        }

        // parse working directory argument
        bool first = true;
        int index = 0;
        bool quoted = false;
        for ( ; p < args.length(); ++p)
        {
            if (args[p] == ' ') // space after -C
            {
                if (first)
                {
                    first = false;
                    continue;
                }
            }

            if (args[p] == '"')
            {
                if (quoted) // ending quote, done
                {
                    quoted = false;
                    break;
                }
                else
                {
                    if (cwd[0] == '\0') // nothing copied yet
                        quoted = true;
                    else
                    {
                        std::cout << "Parse error: Unexpected quote" << std::endl;
                        return 1;
                    }
                }
            }
            else
            {
                if (args[p] == ' ' && !quoted) // space separator, done
                    break;

                if (index < sizeof(cwd))
                    cwd[index++] = args[p];
                else
                    break;
            }
        }

        if (cwd[0] == '\0') // empty
        {
            std::cout << "Exception: -C option requires a configuration directory argument" << std::endl;
            return 1;
        }

        std::cout << "arg: " << cwd << std::endl;

        // make sure the directory exists
        std::filesystem::path directoryPath(cwd);
        if (std::filesystem::exists(directoryPath))
        {
            if (!std::filesystem::is_directory(directoryPath))
            {
                std::cout << "Exception: -C \"" << cwd << "\" exists but is not a directory" << std::endl;
                return 1;
            }
        }
        else
            _mkdir(cwd);
    }

    chdir(cwd);

    // double check working directory
    char wd[PATH_MAX];
    getcwd(wd, sizeof(cwd));
    std::cout << " wd: " << wd << std::endl;

    // Add software location path to ELS jar
    path = path + "/rt/bin/javaw.exe -jar \"" + path + "/bin/ELS.jar\" ";

    // Add -C if needed
    std::string config = "";
    if (dashC)
    {
        // see if libraries directory exists in the software location
        char check[PATH_MAX];
        std::string sd = wd;
        sd += "/libraries";
        strncpy(check, &sd[0], sd.length());

        // make sure the directory exists
        std::filesystem::path directoryPath(check);
        if (std::filesystem::exists(directoryPath)) // check if libraries is in program directory
        {
            if (!std::filesystem::is_directory(directoryPath))
            {
                std::cout << "Exception: \"" << sd << "\" exists but is not a directory" << std::endl;
                return 1;
            }
        }
        else // not in program directory, use default of "USER_HOME/.els"
        {
            char check[PATH_MAX];
            char home[PATH_MAX];

            // get user home directory
            if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home) == S_OK)
            {
                std::cout << "hom: " << home << std::endl;
                std::string sd = home;
                sd += "/.els";
                for (int i = 0; i < sizeof(check); ++i)
                    check[i] = '\0';
                strncpy(check, &sd[0], sd.length());

                // make sure the directory exists
                std::filesystem::path directoryPath(check);
                if (std::filesystem::exists(directoryPath))
                {
                    if (!std::filesystem::is_directory(directoryPath))
                    {
                        std::cout << "Exception: \"" << sd << "\" exists but is not a directory" << std::endl;
                        return 1;
                    }
                }
                else
                    _mkdir(check); // create default directory

                for (int i = 0; i < sizeof(wd); ++i)
                    wd[i] = '\0';
                strcpy(wd, check);
            }
            else
            {
                std::cerr << "Exception: Cannot get home directory" << std::endl;
                return 1;
            }
        }

        std::string copy = wd;
        config = "-C " + copy + " ";

        chdir(wd);

        // double check working directory
        char wd[PATH_MAX];
        getcwd(wd, sizeof(cwd));
        std::cout << "cfg: " << wd << std::endl;
    }

    // Assemble the command line with arguments
    std::string cmd = path + config + args;
    for (int i = 0; i < cmd.length(); ++i)
    {
        if (cmd[i] == '\\')
        {
            cmd[i] = '/';
        }
    }

    pCmdLine = const_cast<char *>(cmd.c_str());
    std::cout << "cmd: " << pCmdLine << std::endl;

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

    if (result == 0)
    {
        // Wait until child process exits.
        result = WaitForSingleObject(processInformation.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(processInformation.hProcess);
        CloseHandle(processInformation.hThread);
    }

    return result;
}
