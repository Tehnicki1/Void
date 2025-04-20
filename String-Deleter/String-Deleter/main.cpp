#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>

void delete_strings(HANDLE hProcess, const std::vector<std::string>& stringsToDelete);

void print_header() {
    std::cout << "\033[38;2;211;211;211m[/] void.exe | https://github.com/Tehnicki1 | version 1.0\n\033[0m";
}

int main() {
    print_header();

    DWORD pid;
    std::cout << "\n\033[38;2;211;211;211m[?] Minecraft PID > \033[0m";
    std::cin >> pid;

    auto handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);

    if (!handle) {
        std::cout << "\n\033[38;2;211;211;211m[/] Invalid Process.\n\033[0m";
        return 1;
    }
    else {
        std::cin.ignore();
        std::cout << "\033[38;2;211;211;211m[?] Strings you want to delete > \033[0m";
        std::string input;
        std::getline(std::cin, input);

        std::vector<std::string> stringsToDelete;
        std::stringstream ss(input);
        std::string token;
        while (std::getline(ss, token, ',')) {
            token.erase(0, token.find_first_not_of(" \n\r\t"));
            token.erase(token.find_last_not_of(" \n\r\t") + 1);
            if (!token.empty()) {
                stringsToDelete.push_back(token);
            }
        }

        system("cls");

        delete_strings(handle, stringsToDelete);
    }

    std::cout << "\n\033[38;2;211;211;211m[/] All strings deleted successfully, press any key to exit...\033[0m";
    std::cin.ignore();
    std::cin.get();

    return 0;
}

void delete_strings(HANDLE hProcess, const std::vector<std::string>& stringsToDelete) {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    MEMORY_BASIC_INFORMATION memInfo;
    uint8_t* address = static_cast<uint8_t*>(sys_info.lpMinimumApplicationAddress);

    for (const auto& str : stringsToDelete) {
        while (address < sys_info.lpMaximumApplicationAddress && VirtualQueryEx(hProcess, address, &memInfo, sizeof(memInfo))) {
            if (memInfo.State == MEM_COMMIT && (memInfo.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)) &&
                memInfo.Type == MEM_PRIVATE) {

                std::vector<uint8_t> buffer(memInfo.RegionSize);
                SIZE_T bytesRead;
                if (ReadProcessMemory(hProcess, memInfo.BaseAddress, buffer.data(), buffer.size(), &bytesRead)) {
                    std::string_view view(reinterpret_cast<char*>(buffer.data()), bytesRead);

                    size_t pos = 0;
                    while ((pos = view.find(str, pos)) != std::string_view::npos) {
                        void* found = static_cast<uint8_t*>(memInfo.BaseAddress) + pos;
                        SIZE_T bytesWritten;
                        std::string nullString(str.size(), '\0');
                        WriteProcessMemory(hProcess, found, nullString.data(), nullString.size(), &bytesWritten);
                        std::cout << "\033[38;2;211;211;211m[-] \"" << str << "\" successfully deleted at " << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(found) << "\n\033[0m";
                        ++pos;
                    }
                }
            }
            address = static_cast<uint8_t*>(memInfo.BaseAddress) + memInfo.RegionSize;
        }
    }
}
