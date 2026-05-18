//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_APPCLOCK_H
#define RAVENSNESTVTT_APPCLOCK_H
#include <chrono>

#include "SDL3/SDL_mutex.h"

namespace NestVTT::Clock {

    class AppClock {
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastUpdateTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastRenderTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastSyncTime;

        //delta in seconds
        float _lastUpdateDelta {0.0f};
        float _lastRenderDelta {0.0f};
        float _lastSyncDelta {0.0f};

        SDL_Mutex *updateLock;
        SDL_Mutex *renderLock;
        SDL_Mutex *syncLock;

    public:
        AppClock(): _lastUpdateTime(std::chrono::high_resolution_clock::now()),
                    _lastRenderTime(std::chrono::high_resolution_clock::now()),
                    _lastSyncTime(std::chrono::high_resolution_clock::now()),
                    updateLock(SDL_CreateMutex()),
                    renderLock(SDL_CreateMutex()),
                    syncLock(SDL_CreateMutex()) {}

        ~AppClock() = default;

        void OnUpdate() {
            if (SDL_TryLockMutex(updateLock)) {
                const auto now = std::chrono::high_resolution_clock::now();
                _lastUpdateDelta = std::chrono::duration_cast<std::chrono::duration<float>>(now - _lastUpdateTime).count();
                _lastUpdateTime = now;
                SDL_UnlockMutex(updateLock);
            }
        }

        void OnRender() {
            if (SDL_TryLockMutex(renderLock)) {
                const auto now = std::chrono::high_resolution_clock::now();
                _lastRenderDelta = std::chrono::duration_cast<std::chrono::duration<float>>(now - _lastRenderTime).count();
                _lastRenderTime = now;
                SDL_UnlockMutex(renderLock);
            }
        }

        void OnSync() {
            if (SDL_TryLockMutex(syncLock)) {
                const auto now = std::chrono::high_resolution_clock::now();
                _lastSyncDelta = std::chrono::duration_cast<std::chrono::duration<float>>(now - _lastSyncTime).count();
                _lastSyncTime = now;
                SDL_UnlockMutex(syncLock);
            }
        }

        [[nodiscard]] float getUpdateTimeDelta() const {
            return _lastUpdateDelta;
        }

        [[nodiscard]] float getRenderTimeDelta() const {
            return _lastRenderDelta;
        }

        [[nodiscard]] std::chrono::time_point<std::chrono::high_resolution_clock> getLastUpdateTime() const {
            return _lastUpdateTime;
        }

        [[nodiscard]] std::chrono::time_point<std::chrono::high_resolution_clock> getLastRenderTime() const {
            return _lastRenderTime;
        }

        static std::chrono::time_point<std::chrono::high_resolution_clock> getNow() {
            return std::chrono::high_resolution_clock::now();
        }
    };

    extern AppClock globalAppClock;

}


#define GLOBAL_CLOCK            NestVTT::Clock::globalAppClock
#define GLOBAL_NOW()            NestVTT::Clock::globalAppClock.getNow()
#define UPDATE_GLOBAL_CLOCK()   NestVTT::Clock::globalAppClock.onUpdate()
#define UPDATE_RENDER_CLOCK()   NestVTT::Clock::globalAppClock.onRender()
#define UPDATE_SYNC_CLOCK()     NestVTT::Clock::globalAppClock.onSync()

#endif //RAVENSNESTVTT_APPCLOCK_H
