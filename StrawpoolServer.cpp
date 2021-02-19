// StrawpoolServer.cpp : Jest aplikacją servera, posiada czat z klientami, tworzenie zapytania Strawpool

#include "framework.h"
#include "StrawpoolServer.h"
#include <Processthreadsapi.h>
#pragma comment(lib, "Ws2_32.lib")
#include "Funkcje.h"

//STAŁE ZMIENNE:
const int MAX_LOADSTRING = 16;

   
//GLOBALNE ZMIENNE:
HWND Okno;
HWND hStatic;
HWND hText;
HWND hButton;
HWND CreatePool;

std::vector <HWND> SendANDdestroy;

std::string czat;
SERVER server; 
//DODAĆ MUTEX
HINSTANCE hInst;   //Bieżące wystąpienie

HANDLE mainThread;
DWORD mainThreadId;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    mainThread = GetCurrentThread();
    mainThreadId = GetCurrentThreadId();
    UNREFERENCED_PARAMETER(hPrevInstance);                  //Nie korzystam z parametrów hPrevInstance i lpCmdLine
    UNREFERENCED_PARAMETER(lpCmdLine);                      
    
    WCHAR * mainTitle = new WCHAR[MAX_LOADSTRING];          //Tekst paska tytułu
    WCHAR * mainWindowClass = new WCHAR[MAX_LOADSTRING];    //Nazwa klasy okna głównego

    // Inicjuj ciągi   
    LoadStringW(hInstance, IDS_APP_TITLE, mainTitle, MAX_LOADSTRING);               //Załadowanie IDS_APP_TITLE do mainTitle
    LoadStringW(hInstance, IDC_STRAWPOOLSERVER, mainWindowClass, MAX_LOADSTRING);   //Załadowanie IDC_STRAWPOOLSERVER
    MyRegisterClass(hInstance, mainWindowClass);
    
    // Wykonaj inicjowanie aplikacji: Tutaj tworzone jest okno główne aplikacji  
    Okno = CreateWindowW(mainWindowClass, mainTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    delete[] mainTitle; delete[] mainWindowClass; mainTitle = nullptr; mainWindowClass = nullptr;   //Opróżniam pamięć i ustawiam  wskaźnik na nullptr(Nie będzie on już używany)
    if (!Okno)
        return 0;
    
    hStatic = CreateWindowEx(0, L"STATIC", NULL, WM_SETTEXT | WS_CHILD | WS_VISIBLE | WS_BORDER | SS_EDITCONTROL |
        WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | SS_LEFT, 51, 50, 600, 500, Okno, NULL, hInstance, NULL);

    hText = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER
        , 50, 550, 500, 30, Okno, NULL, hInstance, NULL);
    hButton = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Wyślij", WS_CHILD | WS_VISIBLE,
        550, 550, 100, 30, Okno, NULL, hInstance, NULL);
    CreatePool = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Create Poll", WS_CHILD | WS_VISIBLE,
        50, 600, 600, 30, Okno, NULL, hInstance, NULL);
    hInst = hInstance;                                      //Korzystam z globalnego uchwytu programu
    ShowWindow(Okno, nCmdShow);
    UpdateWindow(Okno);
    
    HANDLE hThread;
    DWORD ThreadID;
    hThread = CreateThread(NULL, 0, Listen_Connections, NULL, 0, &ThreadID);    //Wątek zawierający Server  

    // Główna pętla komunikatów:
    HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_STRAWPOOLSERVER)); //Skróty klawiszowe do elementów menu(W przyszłosci dodam)
    MSG msg;
    while(GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
    }
    CloseHandle(hThread);
    SetWindowText(hStatic, L"Cholera zamknięto Socket");
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance, LPCWSTR mainWindowClass)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STRAWPOOLSERVER));   //MAKEINTRESOURCEA Przekształca typ int na łańcuch znaków 
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_STRAWPOOLSERVER);
    wcex.lpszClassName  = mainWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //SENDTEXT msg1;
    switch(message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            if((HWND)lParam == hButton)    //Kiedy wciśnięty przycisk "Wyślij"
            {

                //server.SEND_TEXT(czat, hStatic, hText);
                break;
            }
            if((HWND)lParam == CreatePool) //Jeżeli wciśnięty przycisk, tworzy okna do tworzenia Pytania Strawpool
            {
                
                Font font("Kristen ITC");
                const int x1 = 800;
                int y1 = 50;

                SendANDdestroy.push_back(CreateWindowEx(0, L"STATIC", L"Create Poll", WM_SETTEXT | WS_CHILD | WS_VISIBLE | SS_LEFT, x1, y1, 150, 20, Okno, NULL, hInst, NULL));
                y1 += 50;
                font.Change(SendANDdestroy[0]);
                SendANDdestroy.push_back(CreateWindowEx(0, L"EDIT", L"Type your question here", WM_SETTEXT | WS_CHILD | WS_BORDER | WS_VISIBLE | SS_LEFT, x1, y1, 250, 20, Okno, NULL, hInst, NULL));
                y1 += 50;
                SendANDdestroy.push_back(CreateWindowEx(0, L"EDIT", L"Enter poll option", WM_SETTEXT | WS_CHILD | WS_VISIBLE | SS_LEFT, x1, y1, 300, 20, Okno, NULL, hInst, NULL));
                y1 += 50;
                SendANDdestroy.push_back(CreateWindowEx(0, L"EDIT", L"Enter poll option", WM_SETTEXT | WS_CHILD | WS_BORDER | WS_VISIBLE | SS_LEFT, x1, y1, 350, 200, Okno, NULL, hInst, NULL));
                y1 += 250;
                SendANDdestroy.push_back(CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Create Pool", WS_CHILD | WS_VISIBLE | SS_LEFT, x1, y1, 350, 50, Okno, NULL, hInst, NULL));
                
                UpdateWindow(Okno);
                
                break;
            }
            if(SendANDdestroy.size() != 0)
            {  
                if ((HWND)lParam == SendANDdestroy[4])
                {
                    server.SendToAll(SendANDdestroy, czat, &hStatic);
                    czat += "Pomyślnie wysłano Poola, aby sprawdzić wyniki, kliknij \"stop voting\"\r\n";
                    SetWindowTextA(hStatic, czat.c_str());
                    break;
                }
            }
        switch (wmId)
            case IDM_ABOUT:
                SetWindowTextA(hStatic, "ENTER");
            break;
            /*
            switch(wmId)
            {
            case IDM_ABOUT:
              //  DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), Okno, About);//IDD_ABOUTBOX
                break;
            case IDM_EXIT:
                DestroyWindow(Okno);
                break;
            default:
                return DefWindowProc(Okno, message, wParam, lParam);
            }*/
        }
        break;
        case WM_CTLCOLORSTATIC: 
        { 
            HDC hdcStatic = (HDC)wParam; 
            static HBRUSH hbrBkgnd;
            SetTextColor(hdcStatic, RGB(0, 0, 0)); 
            SetBkColor(hdcStatic, RGB(255, 255, 255)); 
            if (hbrBkgnd == NULL) 
            { 
                hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255)); 
            } 
            return (INT_PTR)hbrBkgnd; 
        }
        case WM_PAINT:
        {
            /*
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Tutaj dodaj kod rysujący używający elementu hdc...
            EndPaint(hWnd, &ps);*/
        }
        break;
        case WM_SIZE:
        {
            UpdateWindow(Okno);
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


/*
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
}*/

DWORD WINAPI Listen_Connections(LPVOID lpParam)
{
    server.WaitForClients(&hStatic, czat); 
    return 0;
}
