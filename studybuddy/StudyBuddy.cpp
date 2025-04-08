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
    cin.ignore(); // Clear the newline

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
    cout << "\nOptions:\n";
    cout << "1. Ask for location (Where?)\n";
    cout << "2. Ask for courses (What?)\n";
    cout << "3. Ask for members (Members?)\n";
    cout << "4. Join this group (Join=yourname)\n";
    cout << "5. Exit\n";
    cout << "Enter your choice: ";

    int choice;
    cin >> choice;
    cin.ignore(); // Clear the newline
    bool joinedGroup = false;
    while (!joinedGroup) {
        // Copy contents of sendbuf into sendbuf_ and convert to Uppercase
        char sendbuf_[DEFAULT_BUFLEN];

        switch (choice) {
        case 1: // Where?
            strcpy_s(sendbuf, DEFAULT_BUFLEN, "Where?");
            break;
        case 2: // What?
            strcpy_s(sendbuf, DEFAULT_BUFLEN, "What?");
            break;
        case 3: // Members?
            strcpy_s(sendbuf, DEFAULT_BUFLEN, "Members?");
            break;
        case 4: // Join
            strcpy_s(sendbuf, DEFAULT_BUFLEN, "Join=");
            strcat_s(sendbuf, DEFAULT_BUFLEN, clientName);
            break;
        case 5: // Exit
            strcpy_s(sendbuf, DEFAULT_BUFLEN, "Exit");
            break;
        default:
            cout << "Invalid choice!\n";
            continue;
        }
       

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

                // Wait for response
                int waitResponse = wait(StudySocket, 2, 0);

                if (waitResponse == 1) { // check wait failure

                    // Receive response from server
                    iResult = recvfrom(StudySocket, recvbuf, DEFAULT_BUFLEN, 0, (struct sockaddr*)&senderAddr, &senderAddrSize);


                    // If we joined the group successfully
                    if (choice == 4 && strcmp(recvbuf, "Join!") == 0) {
                        cout << "Successfully joined the study group!\n";
                        joinedGroup = true;
                        break;
                    }


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
    cin.ignore(); // Clear any leftover newlines
    cin.getline(clientName, sizeof(clientName));



      
    strcpy_s(group.name, sizeof(group.name), clientName);

    cout << "Enter Location: " << "\n";
    cin.getline(group.loc, sizeof(group.loc));

    char course[100] = "";

    cout << "Enter course(s) in PREFIX XXXX format (empty line to finish):\n";

    while (cin.getline(course, sizeof(course)) && group.coursesCounter < 100) {
        if (course[0] == '\0') {
            break;
        }
        strcpy_s(group.courses[group.coursesCounter], sizeof(group.courses[group.coursesCounter]), course);

        group.coursesCounter++;

    }
    
    // Add host as the first member
    strcpy_s(group.members[group.membersCounter], sizeof(group.members[group.membersCounter]), group.name);
    group.membersCounter++;

    group.serverTaken = true;
    

    cout << "Study group hosted succesfully! Waiting for queries...\n";
    
    while (true) {
        // Receive response from server
        iResult = recvfrom(StudySocket, recvbuf, DEFAULT_BUFLEN-1, 0, 
            (struct sockaddr*)&senderAddr, &senderAddrSize);

        if (iResult == SOCKET_ERROR) {  // Check send failure
            cout << "Recvfrom failed: " << WSAGetLastError() << endl;
            continue;
        }



        if (iResult > 0) {  // Success
            recvbuf[iResult] = '\0'; // Ensure null termination
            cout << "Received: " << recvbuf << "\n";

            if (strcmp(recvbuf, "Who?")==0) { 
                
                // Format: "Name=servername"
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Name=");
                strcat_s(sendbuf, DEFAULT_BUFLEN, group.name);

               

            }
            else if (strcmp(recvbuf, "Where?") == 0) {
                // Format: "Loc=location"
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Loc=");
                strcat_s(sendbuf, DEFAULT_BUFLEN, group.loc);
            }
            else if (strcmp(recvbuf, "What?") == 0) {
                // Format: "Courses=course1\ncourse2\n..."
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Courses=");

                // Add all courses with newline delimiters
                int pos = strlen(sendbuf);
                for (int i = 0; i < group.coursesCounter; i++) {
                    strcat_s(sendbuf, DEFAULT_BUFLEN, group.courses[i]);
                    strcat_s(sendbuf, DEFAULT_BUFLEN, "\n");
                }
            }
            else if (strcmp(recvbuf, "Members?")==0) {
                // Format: "Members=member1\nmember2\n..."
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Members=");

                // Add all members with newline delimiters
                for (int i = 0; i < group.membersCounter; i++) {
                    strcat_s(sendbuf, DEFAULT_BUFLEN, group.members[i]);
                    strcat_s(sendbuf, DEFAULT_BUFLEN, "\n");
                }

            }
            else if (strncmp(recvbuf, "Join=", 5)==0) {
                char* clientName = recvbuf + 5; // skips the first 5 characters

              

                // Add client to members list
                strcpy_s(group.members[group.membersCounter], sizeof(group.members[group.membersCounter]), clientName);
                group.membersCounter++;

                cout << "User '" << clientName << "' joined the group\n";
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Join!");
            }
            else if (strcmp(recvbuf, "Exit")==0) {
                cout << "Client requested to exit\n";
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Program Exited");

            }
            else {
                cout << "Unknow request...\n";
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Unknown request");

            }
            // Send message to server (+1 to include null terminator)
            iResult = sendto(StudySocket, sendbuf, strlen(sendbuf) + 1, 0, (struct sockaddr*)&senderAddr, senderAddrSize);

            if (iResult == SOCKET_ERROR) {  // Check send failure
                cout << "Send failed: " << WSAGetLastError() << endl;
                closesocket(StudySocket);
                WSACleanup();
                return 1;
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
    int action = 0;

    do {
        cout << "Choose an option: (1) Join | (2) Host | (3) Exit\n";
        cin >> action;

        if (action == 1) {
            mainClient();

        }
        else if (action == 2) {
            mainHost();

        }
        else if(action == 3){
            cout << "Program Exited";
        }
        else {
            cout << "Invalid";
        }

    } while (action != 3);
    
    return 0;

}
