//
// Created by wesley on 5/18/26.
//

#ifndef RAVENSNESTVTT_ISERIALIZEABLE_H
#define RAVENSNESTVTT_ISERIALIZEABLE_H
#include <vector>
#include "SDL3/SDL_stdinc.h"


namespace NestVTT::Data {
    class ISerializable {
    public:
        virtual ~ISerializable() = default;
        virtual std::vector<Uint8> serialize() = 0;
        virtual bool loadSerialized(const std::vector<Uint8>& data) = 0;
    };
}



#endif //RAVENSNESTVTT_ISERIALIZEABLE_H
