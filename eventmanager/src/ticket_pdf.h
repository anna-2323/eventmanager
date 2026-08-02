#pragma once
#include <windows.h>
#include <stdio.h>

int start_pdf_process(const char* token) {
    char html_path[128], pdf_path[128], cmd[512];
    snprintf(html_path, sizeof(html_path), "tickets/ticket_%s.html", token);
    snprintf(pdf_path, sizeof(pdf_path), "tickets/ticket_%s.pdf", token);
    snprintf(cmd, sizeof(cmd), "wkhtmltopdf \"./%s\" \"%s\"", html_path, pdf_path);

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (exitCode != 0) {
        fprintf(stderr, "wkhtmltopdf failed (exit code %lu)\n", exitCode);
        return 1;
    }

    printf("PDF generated successfully.\n");
    return 0;
}