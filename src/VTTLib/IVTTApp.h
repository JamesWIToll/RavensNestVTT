//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_VTTAPP_H
#define RAVENSNESTVTT_VTTAPP_H

#include "Config/ConfigParser.h"

namespace NestVTT {
    class IVTTApp {
        Config::Configuration *config = nullptr;
        std::vector<Net::Socket> sockets {};

    public:
        virtual ~IVTTApp() {
            delete config;
        }

        virtual void run() = 0;

        void loadConfig (const std::string &configFilePath) {
            config  = new Config::Configuration{configFilePath};
            sockets = config->getAndInitSockets();
        }

    };
}


#endif //RAVENSNESTVTT_VTTAPP_H
