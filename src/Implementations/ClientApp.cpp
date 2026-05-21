//
// Created by wesley on 5/17/26.
//

#include "ClientApp.h"

#include "VTTLib/Net/Socket.h"


void ClientApp::run() {
    char nextInput = 0x00;
    do {
        std::cout << "(press q to quit)" << std::endl;
        std::cin >> nextInput;
    } while (nextInput != 'q');
}


NestVTT::IVTTApp *application = new ClientApp();