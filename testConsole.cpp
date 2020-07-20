// testC++CHAR.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Windows.h"//SendInput();
#include <iostream>
#include <tlhelp32.h>//FindProcessId()
#include <vector>

HWND g_HWND = NULL;

std::vector<int> FindProcessId(const std::wstring& processName)
{
	//set a new reference vector to carry
	std::vector<int> result;
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return result;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		result.push_back(processInfo.th32ProcessID);
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			result.push_back(processInfo.th32ProcessID);
		}
	}
	CloseHandle(processesSnapshot);
	return result;
}
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam)
	{
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}
void GetAllWindowsFromProcessID(DWORD dwProcessID, std::vector <HWND>& vhWnds)
{
	// find all hWnds (vhWnds) associated with a process id (dwProcessID)
	HWND hCurWnd = nullptr;
	do
	{
		hCurWnd = FindWindowEx(nullptr, hCurWnd, nullptr, nullptr);
		DWORD checkProcessID = 0;
		GetWindowThreadProcessId(hCurWnd, &checkProcessID);
		if (checkProcessID == dwProcessID)
		{
			vhWnds.push_back(hCurWnd);  // add the found hCurWnd to the vector
			//wprintf(L"Found hWnd %d\n", hCurWnd);
		}
	} while (hCurWnd != nullptr);
}
bool SetPDFSize(DWORD dwPid, int X, int Y, int cx, int cy)
{
	bool ret = true;
	int a = 0;
	EnumWindows(EnumWindowsProcMy, dwPid);
	int style = GetWindowLong(g_HWND, GWL_STYLE); //gets current style
	a = SetWindowLong(g_HWND, GWL_STYLE, (style & ~(WS_CAPTION | WS_SIZEBOX | WS_SYSMENU))); //Calling SetWindowLong to set the style on a progressbar will reset its position. , removes caption and the sizebox from current style, HideWindowBorders WS_CAPTION | WS_SIZEBOX | WS_SYSMENU
	if (a == 0) ret = false;
	Sleep(500);
	a = ::SetWindowPos(g_HWND, HWND_NOTOPMOST, X, Y, cx, cy, SWP_HIDEWINDOW); //210, 297 => a4 size,
	if (a == 0) ret = false;
	Sleep(1000);
	a = ::SetWindowPos(g_HWND, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED); //If you have changed certain window data using SetWindowLong, you must call SetWindowPos for the changes to take effect. Use the following combination for uFlags: SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED.
	if (a == 0) ret = false;
	Sleep(1000);
	a = ::ShowWindow(g_HWND, SW_SHOW); //show and activate the window

	Sleep(100); // wait to confirm it is activated

	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;

	input.ki.wVk = VK_F8;	//VK_CONTROL
	input.ki.dwFlags = 0; //  KEYDOWN
	SendInput(1, &input, sizeof(INPUT));
	input.ki.wVk = VK_F9;	// L
	input.ki.dwFlags = 0; //  KEYDOWN
	SendInput(1, &input, sizeof(INPUT));

	Sleep(100); // wait to confirm it is activated

	input.ki.wVk = VK_F8;	//L
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
	input.ki.wVk = VK_F9;	//VK_CONTROL
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
	return ret;
}
void ChangePage(int timeMs) {

	//timeMs = page number 
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = VK_RIGHT;	//Right

	int i = 0;
	std::vector<int>v1 = FindProcessId(L"AcroRd32.exe"); //case sensitive
	while (timeMs > 0)
	{
		for (int i = 0; i < v1.size(); i++) {
			EnumWindows(EnumWindowsProcMy, v1.at(i));
			//::SetFocus(g_HWND);
			SetForegroundWindow(g_HWND);
			Sleep(100);
			//sent a right click to all pdf windows
			input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN
			SendInput(1, &input, sizeof(INPUT));
			input.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
		}
		i = 0;
		timeMs--;
		Sleep(5000);
	}
}
DWORD OpenPDF(std::string PDFName) {

	std::string pdfName = PDFName;
	std::string system_str1 = "acrord32.exe /N \"C:\\ContentsAndLog\\media\\" + pdfName + ".pdf\"";
	LPSTR cmd = const_cast<char*>(system_str1.c_str());
	PROCESS_INFORMATION processInfo;
	STARTUPINFOA startupinfo = { sizeof(startupinfo) };
	if (!CreateProcessA("C:\\Program Files (x86)\\Adobe\\Acrobat Reader DC\\Reader\\acrord32.exe",
		cmd, NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &processInfo))
	{
		WaitForSingleObject(processInfo.hProcess, 0);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	DWORD dwPid = GetProcessId(processInfo.hProcess);
	EnumWindows(EnumWindowsProcMy, dwPid);
	int style = GetWindowLong(g_HWND, GWL_STYLE); //gets current style
	SetWindowLong(g_HWND, GWL_STYLE, (style & ~(WS_CAPTION | WS_SIZEBOX | WS_SYSMENU))); //Calling SetWindowLong to set the style on a progressbar will reset its position. , removes caption and the sizebox from current style, HideWindowBorders WS_CAPTION | WS_SIZEBOX | WS_SYSMENU
	::SetWindowPos(g_HWND, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	return dwPid;
}

int main()
{
	//init to open a hidden pdf
	//if (FindProcessId(L"AcroRd32.exe").size() == 0) 
	//{
	//	STARTUPINFOA si;
	//	PROCESS_INFORMATION pi;
	//	ZeroMemory(&si, sizeof(si));
	//	si.cb = sizeof(si);
	//	ZeroMemory(&pi, sizeof(pi));
	//	if (!CreateProcessA("C:\\Program Files (x86)\\Adobe\\Acrobat Reader DC\\Reader\\acrord32.exe",
	//		NULL, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
	//		printf("CreateProcess failed (%d).\n", GetLastError());
	//		//return;
	//	}
	//	// Wait until child process exits.
	//	WaitForSingleObject(pi.hProcess, 0);
	//	// Close process and thread handles. 
	//	CloseHandle(pi.hProcess);
	//	CloseHandle(pi.hThread);
	//}
	////end of init 
	DWORD dwPid{};
	DWORD dwPid2{};
	//https://stackoverflow.com/questions/7219063/win32-how-to-hide-3rd-party-windows-in-taskbar-by-hwnd
	dwPid = OpenPDF("test");
	Sleep(500);
	std::vector<int>v1 = FindProcessId(L"AcroRd32.exe"); //case sensitive
	std::cout << "PIDs:";
	for (unsigned i = 0; i < v1.size(); i++) {
		std::cout << ' ' << v1.at(i);
		if (dwPid != v1.at(i)) dwPid2 = v1.at(i);
	}
	SetPDFSize(dwPid2, 1920, 0, 800, 500);
	//ChangePage(10);
	Sleep(3000);

	//open second PDF
	dwPid = OpenPDF("test");
	Sleep(500);
	std::vector<int>v2 = FindProcessId(L"AcroRd32.exe"); //case sensitive, now find 4 instances
	std::vector<int>realv2;
	for (std::vector<int>::iterator i = v2.begin(); i != v2.end(); ++i)
	{
		if (std::find(v1.begin(), v1.end(), *i) != v1.end())
		{
			//realv2.push_back(*i);
		}
		else {
			realv2.push_back(*i); //get different elements
		}
	}
	std::cout << "PIDs:";
	for (unsigned i = 0; i < realv2.size(); i++) {
		std::cout << ' ' << realv2.at(i);
		if (dwPid != realv2.at(i)) dwPid2 = realv2.at(i);
	}
	SetPDFSize(dwPid2, 1620, 100, 800, 500);
	ChangePage(10);
	system("taskkill /IM AcroRd32.exe");
}