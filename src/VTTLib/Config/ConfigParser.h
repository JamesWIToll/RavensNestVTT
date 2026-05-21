//
// Created by wesley on 5/20/26.
//

#ifndef RAVENSNESTVTT_CONFIGPARSER_H
#define RAVENSNESTVTT_CONFIGPARSER_H
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "VTTLib/Net/Socket.h"

using namespace nlohmann;

namespace NestVTT::Config {
    class Configuration {
        json _configJSON;

    public:
        explicit Configuration(const std::string& filename) {
            std::ifstream ifs(filename.c_str());
            _configJSON = json::parse(ifs);
        }

        std::vector<Net::Socket> getAndInitSockets() {
            std::vector<Net::Socket> sockets;
            for (auto socketsJSON = _configJSON["sockets"]; auto socketJSON: socketsJSON) {
                Net::Socket &sock = sockets.emplace_back(
                    socketJSON["listenPort"].get<Uint16>(),
                    socketJSON["name"].get<std::string>(),
                    socketJSON["listenIP"].get<std::string>()
                );

                sock.beginListening();

                if (socketJSON.contains("connections")) {
                    for (const auto connections = socketJSON["connections"]; auto connection: connections) {
                        const auto address = connection["address"].get<std::string>();
                        const auto port = connection["port"].get<Uint16>();
                        sock.initiateConnection(address, port);
                    }
                }
            }
            return sockets;
        }
    };
}

#endif //RAVENSNESTVTT_CONFIGPARSER_H
