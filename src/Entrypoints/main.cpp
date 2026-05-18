//
// Created by wesley on 5/17/26.
//

#include "main.h"

#include "VTTLib/Helpers/SDLErrors.h"

int main(int argc, char* argv[]) {
    const auto listener = new NestVTT::Logging::ConsoleLog();
    REGISTER_LOG_LISTENER(listener);

    application->run();

    delete listener;
    delete application;
    return 0;
}
