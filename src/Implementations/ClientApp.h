//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_CLIENTAPP_H
#define RAVENSNESTVTT_CLIENTAPP_H
#include "VTTLib/IVTTApp.h"


class ClientApp : public NestVTT::IVTTApp {

public:

    ~ClientApp() override = default;
    void run() override;
};


#endif //RAVENSNESTVTT_CLIENTAPP_H
