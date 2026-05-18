//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_PACKET_H
#define RAVENSNESTVTT_PACKET_H
#include <string>
#include <vector>

#include "SDL3/SDL_stdinc.h"
#include "SDL3_net/SDL_net.h"
#include "VTTLib/Helpers/HelperMacros.h"

namespace NestVTT::Net {
    enum class PacketType : Uint8 {
        INIT = 0x01,
        TEARDOWN = 0x02,
    };

    enum InitCodes : Uint8 {
        DENY    = 0x00,
        REQUEST = BITSHIFT_TO_POS(0),
        CONFIRM = BITSHIFT_TO_POS(1)
    };

    struct Connection {
        std::string name;
        NET_Address *address;
        Uint16 port;
    };

    struct Packet {
        PacketType type;
        std::string sourceName;
        std::vector<Uint8> contents;
        Connection sourceConnection;


        static std::vector<Uint8> getBuffer(Packet& packet) {
            std::vector<Uint8> buffer;

            const Uint8 nameLength = packet.sourceName.size();
            const auto *nameStr = reinterpret_cast<const Uint8*>(&packet.sourceName[0]);
            const auto type = static_cast<Uint8>(packet.type);

            buffer.push_back(type);
            buffer.push_back(nameLength);
            buffer.insert(buffer.end(), nameStr, nameStr + nameLength);
            buffer.insert(buffer.end(), packet.contents.begin(), packet.contents.end());

            return buffer;
        }

        static Packet unpackDatagram(const NET_Datagram* datagram) {
            std::vector<Uint8> buffer;
            auto buf = datagram->buf;
            const auto bufEnd = buf + datagram->buflen;

            PacketType type {};
            memcpy(&type, buf, sizeof(type));

            buf += sizeof(type);

            Uint8 sourceNameLength {};
            memcpy(&sourceNameLength, buf, sizeof(sourceNameLength));
            buf += sizeof(sourceNameLength);

            char sourceNameBuff[sourceNameLength];
            memcpy(sourceNameBuff, buf, sourceNameLength);
            const auto sourceName  = std::string(sourceNameBuff, sourceNameLength);
            buf += sourceNameLength;

            while (buf < bufEnd) {
                const Uint8 data = buf[0];
                buffer.push_back(data);
                buf += sizeof(data);
            }

            return Packet {
                .type = type,
                .sourceName = sourceName,
                .contents = buffer,
                .sourceConnection =  { sourceName, datagram->addr, datagram->port }
            };
        }
    };
}


#endif //RAVENSNESTVTT_PACKET_H
