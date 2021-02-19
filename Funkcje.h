#pragma once
#include <memory>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include "resource1.h"
enum { TEXT, SEND_POOL, REGISTER, LOGIN, END = -1 };

ATOM                MyRegisterClass();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Client(LPVOID lpParam);
//DWORD WINAPI Thread_recv_from_server(LPVOID lpParam);
void SHOW_POOL(std::vector <HWND> & windows_show, const char* recvbuf, int* CMD); //HWND* windows_show
void SEND_TEXT(SOCKET* ConnectSocket, HWND hText);
INT_PTR SetBkgrndColor(int r, int g, int b, HDC * hdcStatic);
void DrawQuestPool(HDC * BeginPaint, HWND * MainWindow);
HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent);


void SHOW_POOL(std::vector <HWND> & windows_show, const char * recvbuf, int * CMD)
{
    //for(HWND window : windows_show)
        //ShowWindow(window, *CMD);
    ShowWindow(windows_show[0], *CMD);//
    const int length = strlen(recvbuf);
    std::string recvbuf1(recvbuf, 1, length + 1);
    std::string tmp[3];
    int i = 0;
     
        for (char & c : recvbuf1) 
        {
            
            if (c != '+' && c != -52)
            {
                tmp[i].push_back(c);
            }
            else
            { 
                if (i < 3)
                {
                    SetWindowTextA(windows_show[i], tmp[i].c_str());
                    ++i;
                }
                else
                    break;
            }
        }
}

INT_PTR SetBkgrndColor(int r, int g, int b, HDC * hdcStatic)
{
    static HBRUSH hbrBkgnd;
    SetTextColor(*hdcStatic, RGB(0, 0, 0));
    SetBkColor(*hdcStatic, RGB(r, g, b));
    if (hbrBkgnd == NULL)
    {
        hbrBkgnd = CreateSolidBrush(RGB(r, g, b));
    }
    return (INT_PTR)hbrBkgnd;
}

void SEND_TEXT(SOCKET * ConnectSocket, HWND hText)
{
    DWORD Length = GetWindowTextLength(hText);
    LPSTR txt_send = (LPSTR)GlobalAlloc(GPTR, Length);
    char command = TEXT;
    LPSTR TXT = &command;

    GetWindowTextA(hText, txt_send, Length);

   // send(*ConnectSocket, TXT, TEXT + 1, 0);
    send(*ConnectSocket, txt_send, Length, 0); 

    SetWindowTextA(hText, " ");
    GlobalFree(txt_send);
}

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask, hbmOld, hbmOld2;
    BITMAP bm;

    GetObject(hbmColour, sizeof(BITMAP), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

    hdcMem = CreateCompatibleDC(NULL);
    hdcMem2 = CreateCompatibleDC(NULL);

    hbmOld = (HBITMAP)SelectObject(hdcMem, hbmColour);
    hbmOld2 = (HBITMAP)SelectObject(hdcMem2, hbmMask);

    SetBkColor(hdcMem, crTransparent);

    BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

    SelectObject(hdcMem, hbmOld);
    SelectObject(hdcMem2, hbmOld2);
    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);

    return hbmMask;
}

void DrawQuestPool(HDC * BeginPaint, HWND * MainWindow)
{
    //POINT pt = { 1000, 1000 };
    HBITMAP hbmObraz, hbmOld, hbmMaska;
    hbmObraz = (HBITMAP)LoadImage(NULL, L"obrazek.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hbmMaska = CreateBitmapMask(hbmObraz, RGB(0, 255, 0));
    HDC hdcNowy = CreateCompatibleDC(*BeginPaint);
    BITMAP bmInfo;

    GetObject(hbmObraz, sizeof(bmInfo), &bmInfo);
    hbmOld = (HBITMAP)SelectObject(hdcNowy, hbmMaska);
    //PlgBlt(hdc, &pt, hdcNowy, bmInfo.bmWidth, bmInfo.bmHeight, 0, 0, hbmMaska, 0, 0);   // Nowe współrzędene bitmapy
    BitBlt(*BeginPaint, 50, 50, bmInfo.bmWidth, bmInfo.bmHeight, hdcNowy, 0, 0, SRCAND);
    SelectObject(hdcNowy, hbmObraz);
    BitBlt(*BeginPaint, 50, 50, bmInfo.bmWidth, bmInfo.bmHeight, hdcNowy, 0, 0, SRCPAINT);

    ReleaseDC(*MainWindow, *BeginPaint);
    SelectObject(hdcNowy, hbmOld);
    DeleteDC(hdcNowy);
    DeleteObject(hbmMaska);
    DeleteObject(hbmObraz);
}

//POŁĄCZENIE SERWER
class CONNECT
{
private:
    WSADATA wsaData;
    struct addrinfo * result = NULL;
    struct addrinfo * ptr = NULL;
    struct addrinfo  hints;
public:
    CONNECT()
    {
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;        //Zezwalanie na standard IPv4 lub IPv6 
        hints.ai_socktype = SOCK_STREAM;    //Preferowany typ Socketu(Można uzyć SOCK_DGRAM lub 0 dla dowolnego typu zwracanego gniazda
        hints.ai_protocol = IPPROTO_TCP;    //Protokół TCP(Można użyć 0 lub UDP)
    }
    void TryConnect(const char * IP, PCSTR Port, SOCKET * ConnectSocket)
    {
        getaddrinfo(IP, Port, &hints, &result);
        //Będzie próbować się połączyć, używając pętli for, dopóki się nie połączy 
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
        {
            // Utwórz Socket do połączenia się z serwerem
            *ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

            if (connect(*ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
            {
                closesocket(*ConnectSocket);
                *ConnectSocket = INVALID_SOCKET;
                continue;
            }
        }
        freeaddrinfo(result); //Zwolnij pamięć addrinfo
    }
};
