#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <SDKDDKVer.h>  //WERSJA SYSTEMU

#define INVALID_SOCKET (SOCKET)(~0)
#define DEFAULT_PORT 27015
const int DEFAULT_BUFLEN = 512;
enum { TEXT, SEND_POOL, REGISTER, LOGIN, END = -1 };

ATOM                MyRegisterClass(HINSTANCE hInstance, LPCWSTR mainWindowClass);
std::string HWND_TEXT_Cpy1(std::vector <HWND> & string, DWORD Length, std::string& chat, HWND* hStatic);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI Listen_Connections(LPVOID lpParam);

typedef struct _SOCKET_INFORMATION 
{
    CHAR Buffer[DEFAULT_BUFLEN];
    WSABUF DataBuf;
    SOCKET Socket;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

class SERVER
{
private:
    DWORD EventTotal;
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
    SOCKET ListenSocket;
    SOCKET Accept;
    SOCKADDR_IN InternetAddr;
    LPSOCKET_INFORMATION SocketInfo;
    DWORD Event;
    WSANETWORKEVENTS NetworkEvents;
    WSADATA wsaData;
    DWORD Flags;
    DWORD RecvBytes;
    DWORD SendBytes;
public:
    SERVER() : EventTotal(0)
    {
        //ClientSocket.resize(32);
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        CreateSocketInformation(ListenSocket);
        WSAEventSelect(ListenSocket, EventArray[0], FD_ACCEPT | FD_CLOSE);  //EventTotal - 1
        InternetAddr.sin_family = AF_INET;
        InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        InternetAddr.sin_port = htons(DEFAULT_PORT);
        bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr));
        listen(ListenSocket, 30);
    }
    BOOL CreateSocketInformation(SOCKET s)
    {
        LPSOCKET_INFORMATION SI;
        EventArray[EventTotal] = WSACreateEvent();
        SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION));
        // Prepare SocketInfo structure for use
        SI->Socket = s;
        SI->BytesSEND = 0;
        SI->BytesRECV = 0;

        SocketArray[EventTotal] = SI;
        EventTotal++;
        return(TRUE);
    }
    void FreeSocketInformation(DWORD Event)
    {
        LPSOCKET_INFORMATION SI = SocketArray[Event];
        closesocket(SI->Socket);
        GlobalFree(SI);

        // Squash the socket and event arrays
        for (DWORD i = Event; i < EventTotal; i++)
        {
            EventArray[i] = EventArray[i + 1];
            SocketArray[i] = SocketArray[i + 1];
        }
        EventTotal--;
    }
    int WaitForClients(HWND* hStatic, std::string& chat)
    {
        while(TRUE)
        {
            // Wait for one of the sockets to receive I/O notification and
            Event = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);       //Network HANDLER
            WSAEnumNetworkEvents(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, EventArray[Event - WSA_WAIT_EVENT_0], &NetworkEvents);

            if(NetworkEvents.lNetworkEvents & FD_ACCEPT)
            {
                if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                    break;
                SERVER::Accept = accept(SocketArray[Event - WSA_WAIT_EVENT_0]->Socket, NULL, NULL);
                if (SERVER::Accept == INVALID_SOCKET)
                    break;
                else
                {
                    chat += "Dołączył student";
                    chat += "\r\n";
                    SetWindowTextA(*hStatic, chat.c_str());
                }

                if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
                {

                    //printf("Too many connections - closing socket...\n");
                    closesocket(SERVER::Accept);
                    break;
                }

                CreateSocketInformation(SERVER::Accept);

                if (WSAEventSelect(SERVER::Accept, EventArray[EventTotal - 1], FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                {
                    return 1;
                }

                else;
                    //printf("WSAEventSelect() is OK!\n");
               // printf("Socket %d got connected...\n", SERVER::Accept);
            }

            // Try to read and write data to and from the data buffer if read and write events occur
            if(NetworkEvents.lNetworkEvents & FD_READ || NetworkEvents.lNetworkEvents & FD_WRITE)
            {
                if(NetworkEvents.lNetworkEvents & FD_READ && NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
                {
                    break;
                }

                if(NetworkEvents.lNetworkEvents & FD_WRITE && NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
                {
                    break;
                }

                SocketInfo = SocketArray[Event - WSA_WAIT_EVENT_0];

                // Read data only if the receive buffer is empty
                if(SocketInfo->BytesRECV == 0)
                {
                    SocketInfo->DataBuf.buf = SocketInfo->Buffer;
                    SocketInfo->DataBuf.len = DEFAULT_BUFLEN;
                    Flags = 0;

                    if(WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                            return 1;
                        }
                        
                    }
                    else
                    {
                        chat += std::string(SocketInfo->DataBuf.buf);
                        chat += "\r\n";
                        SetWindowTextA(*hStatic, chat.c_str());
                        SocketInfo->BytesRECV = RecvBytes;
                    }

                }

                // Write buffer data if it is available
                if(SocketInfo->BytesRECV > SocketInfo->BytesSEND)
                {
                    SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
                    SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

                    if(WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
                            return 1;
                        }
                        // A WSAEWOULDBLOCK error has occurred. An FD_WRITE event will be posted
                        // when more buffer space becomes available
                    }
                    else
                    {
                        SocketInfo->BytesSEND += SendBytes;
                        if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
                        {
                            SocketInfo->BytesSEND = 0;
                            SocketInfo->BytesRECV = 0;
                        }
                    }
                }
            }

            if(NetworkEvents.lNetworkEvents & FD_CLOSE)
            {
                if(NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
                {
                    break;
                }

                //printf("Closing socket information %d\n", SocketArray[Event - WSA_WAIT_EVENT_0]->Socket);
                FreeSocketInformation(Event - WSA_WAIT_EVENT_0);
            }
        }
        return 0;
    }
        
        int SendToAll(std::vector <HWND> & SendANDdestroy, std::string & chat, HWND * hStatic)
        {
            std::string Pool_Send = HWND_TEXT_Cpy1(SendANDdestroy, 123, chat, hStatic);

            for (int i = 0; SocketArray[i] != nullptr; ++i)
            {
                if (SocketArray[i]->BytesSEND <= 0)
                {                   
                    send(SocketArray[i]->Socket, Pool_Send.c_str(), Pool_Send.size(), 0);
                }
                else
                {
                    --i;
                }
            }
            for (int i = 0; i < 5; i++)
                DestroyWindow(SendANDdestroy[i]);
            return 1;
        }
      
};

class Font
{
private:
	HFONT * font;
public:
	Font(const char * FontName)
	{
		font = new HFONT;
		HDC hdc = GetDC(NULL);
		long lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		*font = CreateFontA(lfHeight, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, FontName);
	}
	~Font()
	{
		delete font;
	}
	void Change(HWND Object_HWND)
	{
		SendMessage(Object_HWND, WM_SETFONT, WPARAM(*font), TRUE);
	}

};

std::string HWND_TEXT_Cpy1(std::vector <HWND> & String, DWORD Length, std::string & chat, HWND* hStatic)
{
    std::string result;
    result.push_back(1);
    bool jump = 0;

    for (HWND String1 : String)
    {
        const int length = GetWindowTextLength(String1) + 1;
        char * tmp = new char[length];

        if(jump)
        {   
            GetWindowTextA(String1, tmp, length);
            if(strcmp(tmp,"Create Pool") != 0)
            {
                result += tmp;
                result.push_back('+');
                chat += result;
                chat += "\r\n";
                SetWindowTextA(*hStatic, chat.c_str());
            }
        }
        else
        {
            jump = 1;
        }  
        delete[] tmp;
    }
    return result;
}

void Graphic()
{
    /*
    HDC hdcOkno = GetDC(Okno);
    HPEN PioroCzerw, PioroZiel, Pudelko;
    HBRUSH PedzelZiel, Pudelko1;
    POINT stary = { 800, 150 };
    SetPixel(hdcOkno, 800, 150, 0x0000FF);
    SetPixel(hdcOkno, 801, 150, 0x0000FF);
    SetPixel(hdcOkno, 801, 151, 0x0000FF);
    PioroCzerw = CreatePen(PS_SOLID, 5, 0x0000FF);
    PioroZiel = CreatePen(PS_SOLID, 1, 0x00FF00);
    PedzelZiel = CreateSolidBrush(0x00FF00);
    Pudelko1 = (HBRUSH)SelectObject(hdcOkno, PedzelZiel);
    Pudelko = (HPEN)SelectObject(hdcOkno, PioroCzerw);
    MoveToEx(hdcOkno, 0, 0, &stary);

    Rectangle(hdcOkno, 10, 10, 100, 100);
    SelectObject(hdcOkno, Pudelko1);
    LineTo(hdcOkno, 21, 10);
    SelectObject(hdcOkno, PioroZiel);
    LineTo(hdcOkno, 31, 10);

    SelectObject(hdcOkno, Pudelko);

    DeleteObject(PioroCzerw);
    DeleteObject(PioroZiel);
    ReleaseDC(Okno, hdcOkno);
    */
}
//int a = AddFontResourceA("PatrickHand.ttf");
  //Kristen ITC
