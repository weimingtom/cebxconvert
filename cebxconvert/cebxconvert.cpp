// cebxconvert.cpp : 定义控制台应用程序的入口点。
//

//http://www.programlife.net/createprocess-command-line.html
//http://stackoverflow.com/questions/340356/making-createprocess-inherit-the-console-of-the-calling-process
//https://github.com/kaktos/payout/blob/a34225d09b6753742e8b2758dd1b8c2f9693f74b/javaLauncher/javaLauncher/main.cpp

//参数传入错误，请按以下三种方式传入参数：
//XML数据文件
//源文件 目标文件 模板名称 工作目录 应用程序所在目录
//源文件 目标文件 模板名称 工作目录 应用程序所在目录 文档结构XML文件 封面 红头 JobID

//-v D:/a.ceb D:/a.cebx

#include "stdafx.h"

#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>
#include <malloc.h>
#include <locale.h>

static int _enable_dprintf = 0;
static _TCHAR *arg_input_name = NULL, *arg_output_name = NULL; 

void dprintf(_TCHAR *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    if (_enable_dprintf) {
        _vftprintf(stderr, msg, ap);
    }

    va_end(ap);
}

void display_error(TCHAR* description) {
    DWORD err = GetLastError();
    _TCHAR *s, *s2;

    _ftprintf(stderr, _T("%s, error %ld\n"), description, err);

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | /* dwFlags */
                      FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,                             /* lpSource */
                      err,                              /* dwMessageId */
                      0,                                /* dwLanguageId */
                      (_TCHAR *)&s,                        /* lpBuffer */
                      0,                                /* nSize */
                      NULL) != 0) {                     /* va_list args */
        _ftprintf(stderr, _T("%s"), s);

        s2 = (_TCHAR *) malloc((_tcslen(description) + _tcslen(s) + 5) * sizeof(_TCHAR));
        _stprintf(s2, _T("%s\r\n%s"), description, s);
        //MessageBox(NULL, s2, _T("Android SDK Manager - Error"), MB_OK);
        free(s2);
        LocalFree(s);
    }
}

void replace_escape(TCHAR *s1, TCHAR *s2) {
	TCHAR *p2 = s2;
	TCHAR *p1 = s1;
	while(*p2 != _T('\0')) {
		if (*p2 == _T('\\')) {
			/*
			*p1 = _T('\\');
			p1++;
			*p1 = _T('\\');
			p1++;
			*/
			*p1 = _T('\\');
			p1++;
		} else if (*p2 == _T('\"')) {
			*p1 = _T('\\');
			p1++;
			*p1 = _T('\\');
			p1++;
			*p1 = _T('\\');
			p1++;
		}
		*p1 = *p2;
		p1++;
		p2++;
	}
	*p1 = _T('\0');
}

int sdk_launcher() {
    int                   result = 0;
    STARTUPINFO           startup;
    PROCESS_INFORMATION   pinfo;
    TCHAR                  program_dir[MAX_PATH];
    int                   ret, pos;

    ZeroMemory(&pinfo, sizeof(pinfo));

    ZeroMemory(&startup, sizeof(startup));
    startup.cb = sizeof(startup);
	//startup.dwFlags     = STARTF_USESHOWWINDOW;
    //startup.wShowWindow = SW_HIDE|SW_MINIMIZE;
    startup.hStdError = GetStdHandle(STD_OUTPUT_HANDLE); 
    startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE); 
    //startup.hStdInput = g_hChildStd_IN_Rd;  // my outgoing pipe
    startup.dwFlags |= STARTF_USESTDHANDLES;

    /* get path of current program, to switch dirs here when executing the command. */
    ret = GetModuleFileName(NULL, program_dir, sizeof(program_dir));
    if (ret == 0) {
        display_error(_T("Failed to get program's filename:"));
        result = 1;
    } else {
        /* Remove the last segment to keep only the directory. */
        pos = ret - 1;
        while (pos > 0 && program_dir[pos] != '\\') {
            --pos;
        }
        program_dir[pos] = 0;
    }

    if (!result) {
        dprintf(_T("Program dir: %s\n"), program_dir);

		_TCHAR *ini_path = (_TCHAR *)malloc(1024 * sizeof(_TCHAR));
        ZeroMemory(ini_path, 1024 * sizeof(_TCHAR));
		_stprintf(ini_path, _T("%s\\cebxconvert.ini"), program_dir);

		_TCHAR *xcassist_buffer = (_TCHAR *)malloc(1024 * sizeof(_TCHAR));
		ZeroMemory(xcassist_buffer, 1024 * sizeof(_TCHAR));

		_TCHAR *tpl_buffer = (_TCHAR *)malloc(1024 * sizeof(_TCHAR));
		ZeroMemory(tpl_buffer, 1024 * sizeof(_TCHAR));

		_TCHAR *tmpdir_buffer = (_TCHAR *)malloc(1024 * sizeof(_TCHAR));
		ZeroMemory(tmpdir_buffer, 1024 * sizeof(_TCHAR));

		_TCHAR *appdir_buffer = (_TCHAR *)malloc(1024 * sizeof(_TCHAR));
		ZeroMemory(appdir_buffer, 1024 * sizeof(_TCHAR));

		GetPrivateProfileString(
			_T("environment"),
			_T("xcassist"),
			NULL,
			xcassist_buffer,
			1024,
			ini_path
		);
		if (_tcslen(xcassist_buffer) == 0) {
			_stprintf(xcassist_buffer, _T("%s\\xcassist.exe"), program_dir);
		}

		GetPrivateProfileString(
			_T("environment"),
			_T("tpl"),
			NULL,
			tpl_buffer,
			1024,
			ini_path
		);
		if (_tcslen(tpl_buffer) == 0) {
			_stprintf(tpl_buffer, _T("CEBX"));
		}

		GetPrivateProfileString(
			_T("environment"),
			_T("tmpdir"),
			NULL,
			tmpdir_buffer,
			1024,
			ini_path
		);
		if (_tcslen(tmpdir_buffer) == 0) {
			_stprintf(tmpdir_buffer, _T(".\\"));
		}

		GetPrivateProfileString(
			_T("environment"),
			_T("appdir"),
			NULL,
			appdir_buffer,
			1024,
			ini_path
		);
		if (_tcslen(appdir_buffer) == 0) {
			_stprintf(appdir_buffer, _T(".\\"));
		}

		dprintf(_T("xcassist dir: %s\n"), xcassist_buffer);
		dprintf(_T("tpl: %s\n"), tpl_buffer);
		dprintf(_T("tmpdir: %s\n"), tmpdir_buffer);
		dprintf(_T("appdir: %s\n"), appdir_buffer);

		_TCHAR *arg_buffer = (_TCHAR *)malloc(3 * 1024 * sizeof(_TCHAR));
		ZeroMemory(arg_buffer, 3 * 1024 * sizeof(_TCHAR));
		_stprintf(arg_buffer, _T("%s %s %s %s %s"), 
			arg_input_name, arg_output_name,
			tpl_buffer, tmpdir_buffer, appdir_buffer);

		dprintf(_T("arg_buffer string: %s\n"), arg_buffer);
		//replace_escape(arg_buffer2, arg_buffer);
		//dprintf(_T("arg_buffer2 string: %s\n"), arg_buffer2);

        // SDK Manager.exe is installed by the Windows Installer just below
        // the tools directory and needs to access tools\android.bat
        ret = CreateProcess(
                xcassist_buffer, /* program path */
                arg_buffer, /* command-line */
                NULL, /* process handle is not inheritable */
                NULL, /* thread handle is not inheritable */
                TRUE, /* yes, inherit some handles */
                0/*CREATE_NEW_CONSOLE CREATE_NO_WINDOW*/, /* we don't want a console */
                NULL, /* use parent's environment block */
                program_dir, /* use parent's starting directory */
                &startup, /* startup info, i.e. std handles */
                &pinfo);

        dprintf(_T("CreateProcess returned %d\n"), ret);

        if (!ret) {
            display_error(_T("Failed to execute xcassist.exe"));
            result = 1;
        } else {
			WaitForSingleObject(pinfo.hProcess, INFINITE);
			CloseHandle(pinfo.hThread);
			CloseHandle(pinfo.hProcess);
			//LPCSTR msg = "exit the payout";
			//MessageBox(NULL, msg, "Notice", MB_OK | MB_ICONINFORMATION);
		}

		if (arg_buffer != NULL) {
			free(arg_buffer);
			arg_buffer = NULL;
		}
		if (ini_path != NULL) {
			free(ini_path);
			ini_path = NULL;			
		}
		if (xcassist_buffer != NULL) {
			free(xcassist_buffer);
			xcassist_buffer = NULL;
		}
		if (tpl_buffer != NULL) {
			free(tpl_buffer);
			tpl_buffer = NULL;
		}
		if (tmpdir_buffer != NULL) {
			free(tmpdir_buffer);
			tmpdir_buffer = NULL;
		}
		if (appdir_buffer != NULL) {
			free(appdir_buffer);
			appdir_buffer = NULL;
		}
    }

    dprintf(_T("Cleanup.\n"));

    return result;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _UNICODE
	setlocale(LC_ALL, "");
#endif
	
    _enable_dprintf = argc > 1 && _tcscmp(argv[1], _T("-v")) == 0;
    dprintf(_T("Verbose debug mode.\n"));
	int min_argc = 2;
	if (_enable_dprintf) {
		min_argc = 3;
	}
	if (argc <= min_argc) {
		dprintf(_T("argc=%d, min_argc=%d, argc too small.\n"), argc, min_argc);
		return -1;
	}
	arg_input_name = argv[min_argc - 2 + 1];
	arg_output_name = argv[min_argc - 2 + 2];

	int ret = sdk_launcher();
    return ret;
}

