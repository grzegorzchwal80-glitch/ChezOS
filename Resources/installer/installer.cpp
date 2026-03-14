#include <iostream>
#include <windows.h>
int main(int argc, char const *argv[])
{
    char c;
    std::cout << "Welcome to the ChezOS installer!\nIt works, that git installs the directory as zip and unzips it.\nI recommend having git, if not, it gets automatically installed and deleted. Continue?";
    std::cin >> c;
    if (c == 'y' || c == 'Y'){
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi;
        if (CreateProcessA(NULL,"powershell git",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi) == true){
         CreateProcess(
        NULL,
        "powershell git clone https://github.com/grzegorzchwal80-glitch/ChezOS",
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    ); 
    std::cout << "done" << std::endl;
} else {
        std::cout << "YO boi! i need git! do i install it? ill uninstall it soon i swear!!" << std::endl;
        char ask;
        std::cin >> ask;
        if (ask == 'y' || ask == 'Y') {
                CreateProcessA(
                    "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe",
                    (LPSTR)"-NoProfile -ExecutionPolicy Bypass -Command \"iwr -useb get.scoop.sh | iex; scoop install git; git clone https://github.com/grzegorzchwal80-glitch/ChezOS\"",
                    NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
                );
            } else {
                std::cout << "ok then" << std::endl;
            }
        }
    }
    return 0;
}
