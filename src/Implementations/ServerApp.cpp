//
// Created by wesley on 5/17/26.
//

#include "ServerApp.h"

#include "VTTLib/Net/Socket.h"

void ServerApp::run() {
    NestVTT::Net::Socket socket {8000, "ServerSocket1", "192.168.122.1"};
    socket.beginListening();

    char nextInput = 0x00;
    do {
        std::cout << "(press q to quit)" << std::endl;
        std::cin >> nextInput;
    } while (nextInput != 'q');
    socket.endListening();
}

NestVTT::IVTTApp *application = new ServerApp();