//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_SDLERRORS_H
#define RAVENSNESTVTT_SDLERRORS_H
#include "SDL3/SDL_error.h"
#include "VTTLib/Logging/Logger.h"

namespace NestVTT::Helpers {

    template<typename... Params>
    static bool SDLCheck(const bool success, Params... params) {
        if (!success) {
            std::string format = "SDL Error: {}";
            std::string msg = SDL_GetError();
            int count = sizeof...(params);
            for (int i = 0; i < count; i++) {
                format += ", {}";
            }

            LOG_ERROR(format, msg, params...);
        }
        return success;
    }



}

#define SDL_ERR_CHECK(success, ...) NestVTT::Helpers::SDLCheck<std::string>(success, __VA_ARGS__)


#endif //RAVENSNESTVTT_SDLERRORS_H
