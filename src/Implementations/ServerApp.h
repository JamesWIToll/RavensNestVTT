//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_SERVERAPP_H
#define RAVENSNESTVTT_SERVERAPP_H
#include "VTTLib/IVTTApp.h"


class ServerApp : public NestVTT::IVTTApp {

public:
    ServerApp(): IVTTApp("./config/server.net.config.json") {}

    ~ServerApp() override = default;
    void run() override;
};


#endif //RAVENSNESTVTT_SERVERAPP_H
