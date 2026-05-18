//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_VTTAPP_H
#define RAVENSNESTVTT_VTTAPP_H

namespace NestVTT {
    class IVTTApp {

    public:
        virtual ~IVTTApp() = default;
        virtual void run() = 0;

    };
}


#endif //RAVENSNESTVTT_VTTAPP_H
