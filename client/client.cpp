#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <process.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")

// Protocol:
// 1 byte:
// MSB   LSB
// XXXX XXXX
// TTTL WASD

// Display
// d:\gstreamer\1.0\x86\bin\gst-launch-1.0 -e -v udpsrc port=5001 ! application/x-rtp, encoding-name=JPEG,payload=26 ! rtpjpegdepay ! jpegdec !  autovideosink
// MP4 File
// d:\gstreamer\1.0\x86\bin\gst-launch-1.0 -e -v udpsrc port=5001 ! application/x-rtp, encoding-name=JPEG,payload=26 ! rtpjpegdepay ! decodebin ! x264enc ! mp4mux ! filesink location=file.mp4

#define UP_POS 0
#define LT_POS 1
#define DN_POS 2
#define RT_POS 3
#define HM_POS 4

#define T_POS 7
#define L_POS 6
#define W_POS 5
#define A_POS 4
#define S_POS 3
#define D_POS 2

#define SPD_MAX					3	// [units]
#define SPD_TIME_TO_REACH_MAX	400	// [ms]
#define SPD_TIME_TO_REACH_MIN	200	// [ms]

#define DEFAULT_BUFLEN			512
#define DEFAULT_PORT			"5001" // "11999"
#define CTRL_LOOP_SEND_DELAY	50	//[ms]

SOCKET ConnectSocket = INVALID_SOCKET;

//void gst_display_startup(LPCTSTR lpApplicationName, PROCESS_INFORMATION pi) {
//	// additional information
//	STARTUPINFO si;

//	// set the size of the structures
//	ZeroMemory( &si, sizeof(si) );
//	si.cb = sizeof(si);

//	// start the program up
//	CreateProcess( lpApplicationName,	// the path
//					NULL,				// Command line
//					NULL,				// Process handle not inheritable
//					NULL,				// Thread handle not inheritable
//					FALSE,				// Set handle inheritance to FALSE
//					0,					// No creation flags
//					NULL,				// Use parent's environment block
//					NULL,				// Use parent's starting directory
//					&si,				// Pointer to STARTUPINFO structure
//					&pi					// Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
//	);//CreateProcess()

//};//gst_display_startup()

void test(void *param) {
  uint8_t iResult=0;
  uint8_t ctrl_byte[2]={0};
  uint8_t end_byte=0;
  uint8_t spd_set_min=0;
  uint8_t spd_set_max=SPD_MAX;
  uint8_t spd_curr=0;
  int8_t spd_transit=0;
  uint16_t spd_inc_cnt=0;

  char ctrl_str[16]={0};
//  PROCESS_INFORMATION pi;
//	ZeroMemory( &pi, sizeof(pi) );

	printf("Control Thread started\r\n");
	//gst_display_startup("d:\\gstreamer\\1.0\\x86\\bin\\gst-launch-1.0 -e -v udpsrc port=5001 ! application/x-rtp, encoding-name=JPEG,payload=26 ! rtpjpegdepay ! jpegdec !  autovideosink", pi);

	while (end_byte == 0) {
		ctrl_byte[0]=0;
		ctrl_byte[1]=0;
		strncpy(ctrl_str,"----------------",16);
		// ======  CAR  =======
		if		(GetKeyState('1') & 0x8000) { spd_set_min = 0; spd_curr = spd_set_min; }
		else if (GetKeyState('2') & 0x8000) { spd_set_min = 1; spd_curr = spd_set_min; }
		else if (GetKeyState('3') & 0x8000) { spd_set_min = 2; spd_curr = spd_set_min; }
		else if (GetKeyState('4') & 0x8000) { spd_set_min = 3; spd_curr = spd_set_min; }

		if		(GetKeyState('7') & 0x8000) { spd_set_max = 0; }
		else if (GetKeyState('8') & 0x8000) { spd_set_max = 1; }
		else if (GetKeyState('9') & 0x8000) { spd_set_max = 2; }
		else if (GetKeyState('0') & 0x8000) { spd_set_max = 3; }

		if (((GetKeyState('W') & 0x8000) || (GetKeyState('S') & 0x8000))) {

			if (spd_transit == 0) { spd_transit = 1; spd_inc_cnt = 0;};
			if (spd_curr < spd_set_min) { spd_curr = spd_set_min; }; // we always start from the set speed
			if (spd_curr < spd_set_max) {
				if ((++spd_inc_cnt) >= (SPD_TIME_TO_REACH_MAX / CTRL_LOOP_SEND_DELAY)) {
					spd_inc_cnt = 0;
					++spd_curr;
				}//if()
			};//if(spd < max)
		} else { // else
			if (spd_transit == 1) { spd_transit = 0; spd_inc_cnt = 0;};
			if (spd_curr > spd_set_min) {
				if ((++spd_inc_cnt) >= (SPD_TIME_TO_REACH_MIN / CTRL_LOOP_SEND_DELAY)) {
					spd_inc_cnt = 0;
					--spd_curr;
				}//if()
			};//if(spd > min)
			//spd_curr = 0;
		};//else if ()

		ctrl_byte[0] |= spd_curr;
		ctrl_str[15-0] = (ctrl_byte[0]&0x01)?'1':'0';
		ctrl_str[15-1] = (ctrl_byte[0]&0x02)?'1':'0';

		if (GetKeyState('W') & 0x8000) { // Key pressed
			ctrl_byte[0] |= 1<<W_POS;
			ctrl_str[15-W_POS]='W';
		};//if()
		if (GetKeyState('A') & 0x8000) { // Key pressed
			ctrl_byte[0] |= 1<<A_POS;
			ctrl_str[15-A_POS]='A';
		};//if()
		if (GetKeyState('S') & 0x8000) { // Key pressed
			ctrl_byte[0] |= 1<<S_POS;
			ctrl_str[15-S_POS]='S';
		};//if()
		if (GetKeyState('D') & 0x8000) { // Key pressed
			ctrl_byte[0] |= 1<<D_POS;
			ctrl_str[15-D_POS]='D';
		};//if()
		if (GetKeyState('L') & 0x0001) { // Key toggled
			ctrl_byte[0] |= 1<<L_POS;
			ctrl_str[15-L_POS]='L';
		};//if()
		// ====== CAMERA =======
		if (GetKeyState(VK_UP) & 0x8000) { // Key pressed
			ctrl_byte[1] = 1<<UP_POS;
			ctrl_str[7-UP_POS]='^';
		};//if()
		if (GetKeyState(VK_DOWN) & 0x8000) { // Key pressed
			ctrl_byte[1] = 1<<DN_POS;
			ctrl_str[7-DN_POS]='v';
		};//if()
		if (GetKeyState(VK_RIGHT) & 0x8000) { // Key pressed
			ctrl_byte[1] = 1<<RT_POS;
			ctrl_str[7-RT_POS]='>';
		};//if()
		if (GetKeyState(VK_LEFT) & 0x8000) { // Key pressed
			ctrl_byte[1] = 1<<LT_POS;
			ctrl_str[7-LT_POS]='<';
		};//if()
		if (GetKeyState(VK_HOME) & 0x0001) { // Key toggled
			ctrl_byte[1] = 1<<HM_POS;
			ctrl_str[7-HM_POS]='H';
		};//if()

		if (GetKeyState(VK_ESCAPE) & 0x8000) { // ESC Key pressed
			ctrl_byte[0] = 1<<T_POS;
			strncpy(ctrl_str,"----------------",16);
			end_byte = 1; //break;
		};//if()
		if (ConnectSocket != INVALID_SOCKET) {
			iResult = send(ConnectSocket, (const char *)ctrl_byte, 2, 0);
		} else { iResult = SOCKET_ERROR; };

		if (iResult == SOCKET_ERROR) {
			printf("\r\nSend failed with error: %d\r\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			end_byte = 1; //break;
		};//if()

		printf("%.*s", 16, ctrl_str);
		Sleep(CTRL_LOOP_SEND_DELAY); // [ms]
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	};//inf loop

	// Close process and thread handles.
//	CloseHandle( pi.hProcess );
//	CloseHandle( pi.hThread );
	printf("\r\nControl Thread ended\r\n");
	_endthread();
};//test()

int __cdecl main(int argc, char **argv)
{
	HANDLE hThread;

//hThread = (HANDLE)_beginthread(test, 50, NULL);
//for(;;);

	WSADATA wsaData;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;
	const char *correct_server_resp = "Remote Control Server";

	char *sendbuf = "Remote Control Client v1";
	char recvbuf[DEFAULT_BUFLEN]={0};
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if ( iResult != 0 ) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

//	while(!key_thread_finished) {
//		Sleep(100);
//	};//while()

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection since no more data will be sent
	//iResult = shutdown(ConnectSocket, SD_SEND);
	//if (iResult == SOCKET_ERROR) {
	//	printf("shutdown failed with error: %d\n", WSAGetLastError());
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return 1;
	//};//if()

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if ( iResult > 0 ) {
			printf("Msg received: [%s] (%d Bytes)\n", recvbuf, iResult);
			if (strncmp(recvbuf, correct_server_resp, strlen(correct_server_resp)) == 0) { hThread = (HANDLE)_beginthread(test, 50, NULL); }
			else { printf("Incorrect server response!\r\n"); };
		} else if ( iResult == 0 ) {
			printf("Connection closed at server side!\n");
			//CloseHandle(hThread);
			TerminateThread(hThread,0);
			hThread = 0;
		} else {
			printf("recv failed with error: %d\n", WSAGetLastError());
		};//else()
	} while( iResult > 0 );


	if (hThread) { WaitForSingleObject(hThread, INFINITE); };

	// cleanup
	// shutdown(ConnectSocket, SD_BOTH);
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}
