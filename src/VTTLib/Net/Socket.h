//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_SOCKET_H
#define RAVENSNESTVTT_SOCKET_H

#include <string>
#include <utility>
#include <vector>
#include <queue>
#include <SDL3_net/SDL_net.h>

#include "Packet.h"
#include "VTTLib/Helpers/SDLErrors.h"

namespace NestVTT::Net {
    extern bool netInitialized;
    extern int numSockets;

    class Socket {
        std::string socketName;
        std::string localAddrStr;
        Connection selfConnection{};

        NET_DatagramSocket *datagramSocket;
        SDL_Mutex *socketLock;

        std::vector<Connection> connections {};
        SDL_Mutex *connectionsLock;

        std::queue<Packet> outgoingPackets {};
        SDL_Mutex *outgoingLock;

        std::queue<Packet> receivedPackets {};
        SDL_Mutex *receivedLock;

        bool listening = false;
        SDL_Mutex *listeningLock;
        SDL_Thread *listenThread = nullptr;

        static int SDLCALL listen(void *arg) {
            bool quit = false;
            auto *self = static_cast<Socket *>(arg);

            while (!quit) {

                NET_Datagram *datagram = nullptr;

                do {
                    SDL_LockMutex(self->socketLock);
                    const auto receivedData = SDL_ERR_CHECK(NET_ReceiveDatagram(self->datagramSocket, &datagram), "Couldn't query datagrams");
                    SDL_UnlockMutex(self->socketLock);


                    if (!receivedData || datagram == nullptr) {
                        continue;
                    }

                    Packet packet = Packet::unpackDatagram(datagram);

                    if (packet.type == PacketType::TEARDOWN) {
                        auto addrStr = NET_GetAddressString(packet.sourceConnection.address);
                        LOG_INFO("Socket {} received teardown notification from {}:{} ({})", self->socketName, addrStr, packet.sourceConnection.port, packet.sourceConnection.name);

                        self->removeConnection(packet.sourceConnection);
                        continue;
                    }

                    if (self->receiveInitPacket(packet)) {
                        continue;
                    }

                    if (!self->isConnected(packet.sourceConnection)) {
                        self->initiateConnection(packet.sourceConnection);
                    }

                    SDL_LockMutex(self->receivedLock);
                    self->receivedPackets.push(packet);
                    SDL_UnlockMutex(self->receivedLock);
                } while (datagram != nullptr);

                NET_DestroyDatagram(datagram);

                SDL_LockMutex(self->listeningLock);
                quit = !self->listening;
                SDL_UnlockMutex(self->listeningLock);
            }
            return 0;
        }

        bool initiateConnection(Connection &conn) {
            auto data = std::vector<Uint8>{};
            data.push_back(InitCodes::REQUEST);

            Packet packet {
                .type = PacketType::INIT,
                .contents = data,
                .sourceConnection = selfConnection
            };

            auto addrResolved = NET_WaitUntilResolved(conn.address, 2000);
            auto addrStr = NET_GetAddressString(conn.address);
            if (addrStr == nullptr || addrResolved != NET_SUCCESS) {
                SDL_ERR_CHECK(false, "Couldn't get address.");
                addrStr = "";
                return false;
            }

            sendPacketDirect(packet, conn);


            LOG_INFO("Socket {} connection requested with {}:{}", socketName, addrStr, conn.port);
            return true;
        }

        bool sendConfirm(Connection &conn, bool request = false) {
            auto data = std::vector<Uint8>{};
            data.push_back( request ? InitCodes::REQUEST | InitCodes::CONFIRM : InitCodes::CONFIRM);

            Packet packet {
                .type = PacketType::INIT,
                .contents = data,
                .sourceConnection = selfConnection
            };

            sendPacketDirect(packet, conn);

            auto addrStr = std::string(NET_GetAddressString(conn.address));
            LOG_INFO("Socket {} sent confirmation to {}:{} ({})", socketName, addrStr, conn.port, conn.name);
            if (request) {
                LOG_INFO("Socket {} connection requested with {}:{} ({})", socketName, addrStr, conn.port, conn.name);
            }
            return true;
        }


        bool receiveInitPacket(Packet &packet) {
            if (packet.type == PacketType::INIT) {

                auto srcUrl = NET_GetAddressString(packet.sourceConnection.address);
                auto port = packet.sourceConnection.port;

                auto code = packet.contents[0];

                if (packet.contents[0] == InitCodes::DENY) {
                    LOG_ERROR("Initialization denied for connection with {}:{}", srcUrl, port);
                    return false;
                }

                const bool confirmed = packet.contents[0] & InitCodes::CONFIRM;
                const bool requested = packet.contents[0] & InitCodes::REQUEST;
                if (confirmed) {
                    SDL_LockMutex(connectionsLock);
                    connections.push_back(packet.sourceConnection);
                    SDL_UnlockMutex(connectionsLock);
                    LOG_INFO("Socket {} Connection initialized with {}:{}", socketName, srcUrl, port);
                }
                if (requested) {
                    LOG_INFO("Socket {} Connection requested with {}:{}", socketName, srcUrl, port);
                    sendConfirm(packet.sourceConnection, !confirmed);
                }

                return true;
            }
            return false;
        }

        bool teardownConnection(const Connection &conn) {
            if (!isConnected(conn)) {
                return false;
            }

            SDL_LockMutex(connectionsLock);
            std::erase_if(connections, [&conn](const Connection& curr) {
                return conn.address == curr.address &&
                       conn.port == curr.port;
            });
            SDL_UnlockMutex(connectionsLock);

            auto addrStr = NET_GetAddressString(conn.address);
            LOG_INFO("Socket {} tore down connection with {}:{} ({})", socketName, addrStr, conn.port, conn.name);

            return true;
        }

    public:
        explicit Socket(const Uint16 localPort, std::string socketName, const std::string& localAddr) :
            socketName(std::move(socketName)),
            localAddrStr(localAddr)
        {
            if (!netInitialized) {
                if (!SDL_ERR_CHECK(NET_Init(), "Couldn't initialize Net library")) {
                    throw std::runtime_error("Couldn't initialize Net library");
                }
                netInitialized = true;
            }

            NET_Address *addr;

            if (!localAddr.empty()) {
                addr = NET_ResolveHostname(localAddr.c_str());
            } else {
                addr = NET_ResolveHostname("");
            }

            auto resolved = NET_WaitUntilResolved(addr, 5000);

            SDL_ERR_CHECK(addr != nullptr && resolved == NET_SUCCESS, "couldn't resolve host name: " + localAddr);

            selfConnection = Connection(socketName, addr, localPort);

            datagramSocket = NET_CreateDatagramSocket(addr, localPort, 0);
            numSockets++;

            socketLock = SDL_CreateMutex();
            listeningLock = SDL_CreateMutex();
            receivedLock = SDL_CreateMutex();
            outgoingLock = SDL_CreateMutex();
            connectionsLock = SDL_CreateMutex();
        }

        ~Socket() {
            SDL_LockMutex(listeningLock);
            listening = false;
            SDL_UnlockMutex(listeningLock);

            SDL_WaitThread(listenThread, nullptr);

            for (const auto& connection : connections) {
                removeConnection(connection);
            }

            SDL_DestroyMutex(connectionsLock);
            SDL_DestroyMutex(socketLock);
            SDL_DestroyMutex(listeningLock);
            SDL_DestroyMutex(receivedLock);
            SDL_DestroyMutex(outgoingLock);

            if (netInitialized) {

                NET_DestroyDatagramSocket(datagramSocket);

                if (numSockets <= 1) {
                    NET_Quit();
                }
            }
        }

        [[nodiscard]] std::string getLocalAddr() const {
            return localAddrStr;
        }

        [[nodiscard]] Uint16 getPort() const {
            return selfConnection.port;
        }

        [[nodiscard]] bool isListening() const {
            return listening;
        }

        void beginListening() {
            SDL_LockMutex(listeningLock);
            listening = true;
            SDL_UnlockMutex(listeningLock);

            const auto name = "listen thread:  " + socketName;
            listenThread = SDL_CreateThread(listen, name.c_str(), this);
        }

        void endListening() {
            SDL_LockMutex(listeningLock);
            listening = false;
            SDL_UnlockMutex(listeningLock);
        }

        void queueOutgoing (const PacketType type, std::vector<Uint8> data) {

            SDL_LockMutex(outgoingLock);
            outgoingPackets.emplace(
                type,
                std::move(data),
                Connection{socketName, selfConnection.address, selfConnection.port }
            );
            SDL_UnlockMutex(outgoingLock);
        }

        void postOutgoingPackets () {
            auto empty = false;
            SDL_LockMutex(outgoingLock);
            empty = outgoingPackets.empty();
            SDL_UnlockMutex(outgoingLock);

            while (!empty) {
                SDL_LockMutex(outgoingLock);
                auto packet = outgoingPackets.front();
                outgoingPackets.pop();
                empty = outgoingPackets.empty();
                SDL_UnlockMutex(outgoingLock);

                auto data = Packet::getBuffer(packet);


                SDL_LockMutex(connectionsLock);

                for (const auto& [name, address, port] : connections) {

                    SDL_LockMutex(socketLock);
                    NET_SendDatagram(datagramSocket, address, port, data.data(), data.size());
                    SDL_UnlockMutex(socketLock);
                }

                SDL_UnlockMutex(connectionsLock);

            }

        }

        void sendPacketDirect (Packet packet, const Connection& conn) const {
            const auto buff = Packet::getBuffer(packet);
            const auto data = buff.data();
            const int size = buff.size();
            SDL_LockMutex(socketLock);
            NET_SendDatagram(datagramSocket, conn.address, conn.port, data, size);
            SDL_UnlockMutex(socketLock);
        }



        bool initiateConnection(std::string address, const Uint16 port) {
            const auto addr = NET_ResolveHostname(address.c_str());
            if (!addr) {
                LOG_ERROR("Couldn't resolve hostname for address {} on port {}", address, port);
                return false;
            }

            Connection conn {"", addr, port};
            return initiateConnection(conn);
        }

        const Connection *getConnection(std::string address, const Uint16 port) {
            SDL_LockMutex(connectionsLock);
            const auto found = std::ranges::find_if(connections,
              [&address, &port](const Connection& conn) {
                  return NET_GetAddressString(conn.address) == address && conn.port == port;
              });

            if (found == connections.end()) {
                return nullptr;
            }
            SDL_UnlockMutex(connectionsLock);

            return &*found;
        }

        bool removeConnection(const Connection &conn) {
            if (!isConnected(conn)) {
                return false;
            }

            const Packet packet {
                .type = PacketType::TEARDOWN,
                .contents = {},
                .sourceConnection = selfConnection
            };

            sendPacketDirect(packet, conn);

            auto addrStr = NET_GetAddressString(conn.address);
            LOG_INFO("Socket {} sent teardown notification to {}:{} ({})", socketName, addrStr, conn.port, conn.name);

            return teardownConnection(conn);
        }

        [[nodiscard]] bool isConnected (const Connection& conn)  const{
            const auto addrStr = NET_GetAddressString(conn.address);
            const auto port = conn.port;

            SDL_LockMutex(connectionsLock);

            for (const auto& [name, address2, port2] : connections) {
                auto addrStr2 = NET_GetAddressString(address2);
                if (port2 == port && std::string(addrStr2) == std::string(addrStr)) {
                    SDL_UnlockMutex(connectionsLock);
                    return true;
                }
            }

            SDL_UnlockMutex(connectionsLock);

            return false;
        }

        [[nodiscard]] bool isConnected (const std::string& address, const Uint16 port) const {
            const auto addr = NET_ResolveHostname(address.c_str());
            if (const auto result = NET_WaitUntilResolved(addr, 1000); result != NET_SUCCESS) {
                return false;
            }

            const Connection conn {"", addr, port};

            return isConnected (conn);
        }

    };
}

#endif //RAVENSNESTVTT_SOCKET_H
