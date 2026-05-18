//
// Created by wesley on 5/17/26.
//

#include "ClientApp.h"

#include "VTTLib/Net/Socket.h"


void ClientApp::run() {

    NestVTT::Net::Socket socket {9000, "ClientSocket1", "192.168.122.1"};
    socket.beginListening();
    socket.initiateConnection("192.168.122.1", 8000);

    char nextInput = 0x00;
    do {
        std::cout << "(press q to quit)" << std::endl;
        std::cin >> nextInput;
    } while (nextInput != 'q');
    socket.endListening();
}


NestVTT::IVTTApp *application = new ClientApp();