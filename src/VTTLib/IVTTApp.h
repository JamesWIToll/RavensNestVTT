//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_VTTAPP_H
#define RAVENSNESTVTT_VTTAPP_H

#include "Config/ConfigParser.h"

namespace NestVTT {
    class IVTTApp {
        Config::Configuration config;
        std::vector<Net::Socket> sockets;

    public:
        explicit IVTTApp(const std::string& configFile) : config(configFile) {
            sockets = config.getAndInitSockets();
        }

        virtual ~IVTTApp() = default;
        virtual void run() = 0;

    };
}


#endif //RAVENSNESTVTT_VTTAPP_H
