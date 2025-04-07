// Include necessary libraries
#pragma once
#include <iostream>       
#include <ws2tcpip.h>     // Windows sockets (TCP/IP functions)
#include <WinSock2.h>     // Windows sockets main header
#include<cctype>
#include <cstring>

#define _CRT_SECURE_NO_WARNINGS

#include "StudyBuddy.h"



using std::cin;
using std::endl;
using std::cout;

// Link to Windows socket library (needed for compiler)
#pragma comment(lib,"Ws2_32.lib")

struct StudyGroup {
    char name[100] = "";
    char loc[100] = "";


   
    char courses[100][100];
    char members[100][100];

    int coursesCounter = 0;
    int membersCounter = 0;

    bool serverTaken = false;


};

char* getClientName(char str[]) {
    static char client[100];
    int iCounter = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '=') {
            for (int index = i+1; index < strlen(str); index++) {
                client[iCounter] = str[index];
                iCounter++;
            }
            client[iCounter] = '\0'; // null-terminate

            break;
        }
    }


    return client;

}

void toUpper(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        str[i] = toupper(str[i]);
    }
}


int mainClient() {

    // ---------------------------
    // 1. Initialize Winsock (Windows Socket API)
    // ---------------------------
    WSADATA wsaData;  // Structure to store socket implementation details
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);      // Variable to store function return values
    // Check if initialization failed
    if (iResult != 0) {
        cout << "WSAStartup failed: " << iResult << endl;
        WSACleanup();  // Cleanup resources (typo fixed from original code)
        return 1;      // Exit with error
    }

    // ---------------------------
    // 2. Create connection socket
    // ---------------------------

    SOCKET StudySocket = INVALID_SOCKET;
    StudySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (StudySocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }


    // -------------------------
    // 3. Get input from user and discover
    // --------------------------

    char clientName[100];
    ServerStruct servers[100];
 
    cout << "Enter your name: ";
    cin.getline(clientName, sizeof(clientName));

    // Broadcast "Who?"
    int availableServers = getServers(StudySocket, servers);
    cout << "\nServers available: \n";

    if (availableServers > 0) {
        for (int i = 0; i < availableServers; i++) {
            cout << (i+1) << " - " << servers[i].name << "\n";
        }
    }
    else {
        cout << "No servers available\n";
        closesocket(StudySocket);
        WSACleanup();
        return 1;
    }

    int serverInt;
    cout << "Choose Server #\n> ";
    cin >> serverInt;

    serverInt = serverInt - 1;
    if (serverInt < 0 || serverInt >= availableServers) {
        cout << "Invalid server number!\n";
        closesocket(StudySocket);
        WSACleanup();
        return 1;
    }

    // Sender Address
    struct sockaddr_in senderAddr = servers[(serverInt)].addr;
    int senderAddrSize = sizeof(senderAddr);
    

    // ---------------------------
    // 4. Send & Receive Data
    // ---------------------------

    char recvbuf[DEFAULT_BUFLEN];  // Buffer for received data
    char sendbuf[DEFAULT_BUFLEN];  // Buffer for data to send
    
    cout << "\nType your message (or 'Exit' to quit):\n";
    cin.getline(sendbuf, DEFAULT_BUFLEN);

    // Copy contents of sendbuf into sendbuf_ and convert to Uppercase
    char sendbuf_[DEFAULT_BUFLEN];
    strcpy_s(sendbuf_, DEFAULT_BUFLEN, sendbuf);
    toUpper(sendbuf_);


    while (strcmp(sendbuf_, "EXIT") != 0) {

        // Send message to server (+1 to include null terminator)
        iResult = sendto(StudySocket, sendbuf, 
            strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

        if (iResult == SOCKET_ERROR) {  // Check send failure
            cout << "Send failed: " << WSAGetLastError() << endl;
            closesocket(StudySocket);
            WSACleanup();
            return 1;
        }




            int counter = 0;
            while (true) {
                // Wait for response
                int waitResponse = wait(StudySocket, 2, 0);

                if (waitResponse == 1) { // check wait failure
                
                    // Receive response from server
                    iResult = recvfrom(StudySocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr*)&senderAddr, &senderAddrSize);

                    if (iResult > 0) {  // Success
                        cout << recvbuf << '\0';
                    }
                    else if (iResult == 0) {  // Connection closed
                        cout << "Connection closed by server" << endl;
                    }
                    else {  // Error
                        cout << "Receive failed: " << WSAGetLastError() << endl;
                    }
                }
                else if (waitResponse == 0) {
                    // Timeout
                    
                    break;
                }
                else {
                    break;
                }
            
        }

            cout << "\nType your message (or 'Exit' to quit):\n";
            cin.getline(sendbuf, DEFAULT_BUFLEN);

            // Copy contents of sendbuf into sendbuf_ and convert to Uppercase
            char sendbuf_[DEFAULT_BUFLEN];
            strcpy_s(sendbuf_, DEFAULT_BUFLEN, sendbuf);
            toUpper(sendbuf_);
    }


    // ---------------------------
    // 5. Cleanup & Disconnect
    // ---------------------------

    closesocket(StudySocket);  // Close socket

    WSACleanup();                // Cleanup Winsock

    return 0;  // Exit successfully
}


int mainHost()   {

    // ---------------------------
    // 1. Initialize Winsock (Windows Socket API)
    // ---------------------------
        WSADATA wsaData;  // Structure to store socket implementation details
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);      // Variable to store function return values
        // Check if initialization failed
        if (iResult != 0) {
            cout << "WSAStartup failed: " << iResult << endl;
            WSACleanup();  // Cleanup resources (typo fixed from original code)
            return 1;      // Exit with error
        }

    
    StudyGroup group;
    char clientName[100];

    SOCKET StudySocket = INVALID_SOCKET;
    StudySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (StudySocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    // Data in the sin_port and sin_addr members of the sockaddr_in struct need to be in network byte order.
    // Use htons to convert the port number and htonl to convert the IP address. (Port numbers
    // and addresses returned by getaddrinfo are already in network byte order.)

    struct sockaddr_in myAddr;
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(DEFAULT_PORT);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(StudySocket, (SOCKADDR*)&myAddr, sizeof(myAddr));
    if (iResult == SOCKET_ERROR) {
        cout << "bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(StudySocket);
        WSACleanup();
        return 1;
    }

    // --------------------------------

    char recvbuf[DEFAULT_BUFLEN];  // Buffer for received data
    char sendbuf[DEFAULT_BUFLEN];  // Buffer for data to send

    // Sender Address
    struct sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    cout << "Enter Name: " << "\n";
    cin.getline(clientName, sizeof(clientName));


    if (!group.serverTaken) {
      
        strcpy_s(group.name, sizeof(group.name), clientName);

        cout << "Enter Location: " << "\n";
        cin.getline(group.loc, sizeof(group.loc));

        char course[100] = "";

        cout << "Enter Course(s): " << "\n";

        while (cin.getline(course, sizeof(course)) && group.coursesCounter < 100) {
            if (course[0] == '\0') {
                break;
            }
            strcpy_s(group.courses[group.coursesCounter], sizeof(group.courses[group.coursesCounter]), course);

            group.coursesCounter++;

        }
        group.serverTaken = true;
    }


    while (true) {
        cout << "Options:\nWho? | Where? | What? | Members? | Join=servername \n";
        // Receive response from server
        iResult = recvfrom(StudySocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr*)&senderAddr, &senderAddrSize);

        if (iResult > 0) {  // Success

            if (strcmp(recvbuf, "Who?")==0) { 
                
                strcpy_s(sendbuf, DEFAULT_BUFLEN, group.name);

                // Send message to server (+1 to include null terminator)
                iResult = sendto(StudySocket, sendbuf, strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

                if (iResult == SOCKET_ERROR) {  // Check send failure
                    cout << "Send failed: " << WSAGetLastError() << endl;
                    closesocket(StudySocket);
                    WSACleanup();
                    return 1;
                }

            }
            else if (strcmp(recvbuf, "Where?")==0) {
                strcpy_s(sendbuf, DEFAULT_BUFLEN, group.loc);

                // Send message to server (+1 to include null terminator)
                iResult = sendto(StudySocket, sendbuf, strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

                if (iResult == SOCKET_ERROR) {  // Check send failure
                    cout << "Send failed: " << WSAGetLastError() << endl;
                    closesocket(StudySocket);
                    WSACleanup();
                    return 1;
                }
            }
            else if (strcmp(recvbuf,"What?")==0) {
                for (int i = 0; i < group.coursesCounter; i++) {
                    strcpy_s(sendbuf, DEFAULT_BUFLEN, group.courses[i]);

                    // Send message to server (+1 to include null terminator)
                    iResult = sendto(StudySocket, sendbuf, strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

                    if (iResult == SOCKET_ERROR) {  // Check send failure
                        cout << "Send failed: " << WSAGetLastError() << endl;
                        closesocket(StudySocket);
                        WSACleanup();
                        return 1;
                    }
                }

            }
            else if (strcmp(recvbuf, "Members?")==0) {
                for (int i = 0; i < group.membersCounter; i++) {
                    strcpy_s(sendbuf, DEFAULT_BUFLEN, group.members[i]);

                    // Send message to server (+1 to include null terminator)
                    iResult = sendto(StudySocket, sendbuf, strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

                    if (iResult == SOCKET_ERROR) {  // Check send failure
                        cout << "Send failed: " << WSAGetLastError() << endl;
                        closesocket(StudySocket);
                        WSACleanup();
                        return 1;
                    }
                }

            }
            else if (strncmp(recvbuf, "Join=", 5)==0) {
                char* result = recvbuf + 5; // skips the first 5 characters

                strcpy_s(group.members[group.membersCounter], sizeof(group.members[group.membersCounter]), result);
                group.membersCounter++;

                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Join!");

                // Send message to server (+1 to include null terminator)
                iResult = sendto(StudySocket, sendbuf, strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

                if (iResult == SOCKET_ERROR) {  // Check send failure
                    cout << "Send failed: " << WSAGetLastError() << endl;
                    closesocket(StudySocket);
                    WSACleanup();
                    return 1;
                }
            }
            else if (strcmp(recvbuf, "Exit")==0) {
                break;
            }


        }
        else if (iResult == 0) {  // Connection closed
            cout << "Connection closed by server" << endl;
            break;
        }
        else {  // Error
            cout << "Receive failed: " << WSAGetLastError() << endl;
            break;
        }
    }
  
     

    // ---------------------------
    // 5. Cleanup & Disconnect
    // ---------------------------

    closesocket(StudySocket);  // Close socket

    WSACleanup();                // Cleanup Winsock

    return 0;  // Exit successfully


}


int main() {
    cout << "What do you want to do?\n";
    char action[100];
    cin.getline(action, sizeof(action));
    toUpper(action);
    if (strcmp(action,"HOST") ==0){
        mainHost();
    }
    else if (strcmp(action,"JOIN")==0) {
        mainClient();
    }
    else {
        cout << "Invalid";
    }  

    return 0;

}
