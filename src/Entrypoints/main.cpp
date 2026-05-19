//
// Created by wesley on 5/17/26.
//

#include "main.h"

#include "VTTLib/Helpers/SDLErrors.h"

int main(int argc, char* argv[]) {
    const auto listener = new NestVTT::Logging::ConsoleLog();
    REGISTER_LOG_LISTENER(listener);

    try {
        application->run();
    } catch (std::runtime_error& e) {
        std::string msg(e.what());
        LOG_FATAL("{}", msg);
    }

    delete listener;
    delete application;
    return 0;
}
