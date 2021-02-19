// StrawpoolClient.cpp : Definiuje punkt wejścia dla aplikacji.
//
#define WIN32_LEAN_AND_MEAN

#include "Funkcje.h"
#include <Processthreadsapi.h>
#include "framework.h"
#include "StrawpoolClient.h"
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "27015"
#define IPv4 "127.0.0.1"
const int MAX_LOADSTRING = 16;
const int DEFAULT_BUFLEN = 8192;

std::string fullname;
// Zmienne globalne:
SOCKET ConnectSocket = INVALID_SOCKET;
HWND Okno;
HWND hStatic;
HWND hText;
HWND hButton;
//HWND * SendANDdestroy = new HWND[3];
std::vector <HWND> SendANDdestroy;
int * CMD = nullptr;
int IsDraw = 0;
std::string czat;
HINSTANCE hInstance;                                // bieżące wystąpienie

HANDLE mainThread;
DWORD mainThreadId;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    mainThread = GetCurrentThread();
    mainThreadId = GetCurrentThreadId();
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    CMD = &nCmdShow;
    hInstance = hInstance;                                      //Korzystam z globalnego uchwytu programu
    
    MyRegisterClass();

    // Wykonaj inicjowanie aplikacji:
    
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), Okno, Login);
    hStatic = CreateWindowEx(0, _T("STATIC"), NULL, WM_SETTEXT | WS_CHILD | WS_VISIBLE | WS_BORDER | SS_EDITCONTROL |
        WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | SS_LEFT, 51, 50, 600, 500, Okno, NULL, hInstance, NULL);

    hText = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER
        , 50, 550, 500, 30, Okno, NULL, hInstance, NULL);
    hButton = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"), _T("Wyślij"), WS_CHILD | WS_VISIBLE,
        550, 550, 100, 30, Okno, NULL, hInstance, NULL);

    ShowWindow(Okno, nCmdShow);
    UpdateWindow(Okno);


    SendANDdestroy.push_back(CreateWindowEx(0, L"STATIC", L"Checkbox", WM_SETTEXT | WS_CHILD | SS_EDITCONTROL | WS_VISIBLE,
        800, 140, 250, 15, Okno, NULL, hInstance, NULL));
    SendANDdestroy.push_back(CreateWindowEx(0, L"BUTTON", L"Checkbox", WS_CHILD | BS_AUTOCHECKBOX ,
        800, 160, 250, 30, Okno, NULL, hInstance, NULL));
    SendANDdestroy.push_back(CreateWindowEx(0, L"Button", L"Checkbox", WS_CHILD | BS_AUTOCHECKBOX ,
        800, 180, 650, 30, Okno, NULL, hInstance, NULL));
    HANDLE hThreadClient;
    DWORD ThreadID;
    hThreadClient = CreateThread(NULL, 0, Client, NULL, 0, &ThreadID);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STRAWPOOLCLIENT));
    MSG msg;
    // Główna pętla komunikatów:
    
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    CloseHandle(hThreadClient);
    return (int) msg.wParam;
}

//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass() //LPCWSTR mainWindowClass
{
    WCHAR mainTitle[MAX_LOADSTRING], mainWindowClass[MAX_LOADSTRING];   //Tekst paska tytułu i nazwa klasy okna głównego

    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, mainTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_STRAWPOOLCLIENT, mainWindowClass, MAX_LOADSTRING);

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STRAWPOOLCLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_STRAWPOOLCLIENT);
    wcex.lpszClassName = mainWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    //RegisterClassExW(&wcex);
    if (!RegisterClassExW(&wcex))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"), TEXT("..."), MB_ICONERROR);
        return 0;
    }
    Okno = CreateWindowW(mainWindowClass, mainTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!Okno)
        return 0;
    return (ATOM)true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_COMMAND:
    {
            int wmId = LOWORD(wParam);

            if((HWND)lParam == hButton)
            {
                SEND_TEXT(&ConnectSocket, hText);
                break;
            }
            /*
            if ((HWND)lParam == SendANDdestroy[1])
            {
                int ID_CHECKBOX1 = GetDlgCtrlID(SendANDdestroy[1]);
                //CheckDlgButton(Okno, ID_CHECKBOX1, BST_CHECKED); //ustaw "fajeczkę"
                //CheckDlgButton(Okno, ID_CHECKBOX1, BST_INDETERMINATE);
                //CheckDlgButton(hwnd, ID_CHECKBOX1, BST_UNCHECKED); //usuń "fajeczkę"
                czat += "Przyciśnięto checkbox\r\n";
                SetWindowTextA(hStatic, czat.c_str());
                break;
            }*/
            // Analizuj zaznaczenia menu:6
            
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
    }
    break;
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        if (SendANDdestroy.size() && SendANDdestroy[0] == (HWND)lParam)
            return SetBkgrndColor(255, 215, 86, &hdcStatic);
        return SetBkgrndColor(255, 255, 255, &hdcStatic);
    }
    break;
    case WM_PAINT:
    {
        
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(Okno, &ps);
        if (IsDraw)
        {
            DrawQuestPool(&hdc, &Okno);
            IsDraw = 0;    
        }
        EndPaint(Okno, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
DWORD WINAPI Client(LPVOID lpParam)
{
    CONNECT Server;
    Server.TryConnect(IPv4, DEFAULT_PORT, &ConnectSocket);

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;
    std::string tmp;
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);  //Odbierz od serwera
        if (iResult > 0)
        {
            char MessageKind = recvbuf[0];  //Pierwszy element zawiera żądanie np 1 - SEND_POOL

            switch (MessageKind)
            {
            case TEXT:
                //iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

                czat += std::string{recvbuf};
                czat += "\r\n";
                SetWindowTextA(hStatic, czat.c_str());
                break;

            case SEND_POOL:
                //iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
                IsDraw = 1;
                PostMessage(Okno, WM_PAINT, 0, 0);
                PostThreadMessageA(mainThreadId, WM_PAINT, 0, 0);
                InvalidateRect(Okno, NULL, TRUE); //Aktualizuje WM_PAINT
                
                //czat += recvbuf;
                //czat += "\r\n";
                //tmp = std::string{ recvbuf };
                //SetWindowTextA(hStatic, tmp.c_str());
                SHOW_POOL(SendANDdestroy, recvbuf, CMD);
                break;

            default:
                break;
            }
        }
        else if (iResult == 0)
        {
            SetWindowText(hStatic, _T("Zamknięto połączenie"));
        }
        else
        {
            //czat += "recv failed";
            //czat += "\r\n";
            //SetWindowTextA(hStatic, czat.c_str());
            //closesocket(ConnectSocket);
            //WSACleanup();
        }
        //Sleep(500);
    } while (iResult < 2000);
    /*
    DWORD ThreadID2;
    HANDLE hThread = CreateThread(NULL, 0, Thread_recv_from_server, NULL, 0, &ThreadID2);
    while (hThread); //TUTAJ DODAC SEMAFOR*/
    SetWindowText(hStatic, L"Cholera zamknięto Socket");

    closesocket(ConnectSocket);
    WSACleanup();
   // ExitThread(ThreadID2);
   // CloseHandle(hThread);
    return 0;
}
