//
// Created by wesley on 5/17/26.
//

#ifndef RAVENSNESTVTT_LOGGER_H
#define RAVENSNESTVTT_LOGGER_H

//
// Created by wesley on 8/7/25.
//

#include <string>
#include <format>
#include <iostream>
#include <vector>
#include "SDL3/SDL_stdinc.h"
#include "VTTLib/Clock/AppClock.h"
#include "../Helpers/HelperMacros.h"

namespace NestVTT::Logging {

    enum class LogLevel {
        Fatal = BITSHIFT_TO_POS(1),
        Error = BITSHIFT_TO_POS(2),
        Warning = BITSHIFT_TO_POS(3),
        Debug = BITSHIFT_TO_POS(4),
        Info = BITSHIFT_TO_POS(5),
    };

    static std::string GetLevelName(const LogLevel level) {
        switch (level) {
            case LogLevel::Fatal:
                return "Fatal";
            case LogLevel::Error:
                return "Error";
            case LogLevel::Warning:
                return "Warning";
            case LogLevel::Debug:
                return "Debug";
            case LogLevel::Info:
                return "Info";
        }
        return "";
    }

    struct LogMessage {
        LogLevel level;
        std::string message;
    };

    class ILogListener {
        Uint16 _levelMask = static_cast<Uint16>(LogLevel::Fatal)   |
                             static_cast<Uint16>(LogLevel::Error)   |
                             static_cast<Uint16>(LogLevel::Warning) |
                             static_cast<Uint16>(LogLevel::Debug)   |
                             static_cast<Uint16>(LogLevel::Info);
    public:
        ILogListener() = default;
        virtual ~ILogListener() = default;

        [[nodiscard]] bool watchesLevel(const LogLevel level) const {
            return _levelMask & static_cast<Uint16>(level);
        }

        void addLevel(const LogLevel level) {
            _levelMask |= static_cast<Uint16>(level);
        }

        void removeLevel(const LogLevel level) {
            _levelMask &= ~static_cast<Uint16>(level);
        }

        void setLevelMask(const Uint16 mask) {
            _levelMask = mask;
        }

        virtual void SubmitMessage(LogLevel level, const std::string& message) = 0;

        virtual void flush() = 0;
    };

    class ConsoleLog final : public ILogListener {
        static std::string GetMessageColor(const LogLevel level) {
            switch (level) {
                case LogLevel::Fatal:
                    return "\x1b[31m\x1b[40m";
                case LogLevel::Error:
                    return "\033[31m";
                case LogLevel::Warning:
                    return "\033[33m";
                case LogLevel::Debug:
                    return "\033[36m";
                case LogLevel::Info:
                    return "\x1b[37m";
            }
            return "\x1b[37m";
        }

        std::string _lastMessage;
        bool _repeated{};
        Uint64 _repeatCount = 0;
        SDL_Mutex *writeLock;
    public:
        ConsoleLog() {
            writeLock = SDL_CreateMutex();
        }

        ~ConsoleLog() override {
            SDL_DestroyMutex(writeLock);
        }

        void SubmitMessage(const LogLevel level, const std::string &message) override {
            SDL_LockMutex(writeLock);
            if (watchesLevel(level)) {
                std::string output = std::format("{}Log: [{}] ({}) : {}\x1b[0m", GetMessageColor(level), GetLevelName(level), GLOBAL_NOW(), message);
                if (_lastMessage == message && !_repeated) {
                    _repeated = true;
                    output = " [repeated]";
                    _repeatCount++;
                } else if (_lastMessage == message){
                    output = "";
                    _repeatCount++;
                } else if (_repeated){
                    output = std::format("<{} times>\n", _repeatCount) + output;
                    _repeated = false;
                    _repeatCount = 1;
                } else {
                    _repeated = false;
                    _repeatCount = 1;
                }
                _lastMessage = message;
                if (output.empty()) {
                    return;
                }
                switch (level) {
                    case LogLevel::Fatal:
                    case LogLevel::Error:
                        std::cerr << output << "\n";
                        break;
                    case LogLevel::Warning:
                    case LogLevel::Debug:
                    case LogLevel::Info:
                        std::cout << output << "\n";
                        break;
                }
            }
            std::flush(std::cout);
            std::flush(std::cerr);
            SDL_UnlockMutex(writeLock);
        }

        void flush() override {
            std::cout.flush();
            std::cerr.flush();
        }
    };


    extern std::vector<ILogListener*> listeners;

    static void RegisterListener(ILogListener* listener) {
        listeners.push_back(listener);
    }

    static void SubmitMessage(const LogLevel level, const std::string& message) {
        for (const auto& listener : listeners) {
            listener->SubmitMessage(level, message);
        }
    }

    static void FlushLog() {
        for (const auto listener : listeners) {
            listener->flush();
        }
    }

}

#define REGISTER_LOG_LISTENER(listener) NestVTT::Logging::RegisterListener(listener);

#define LOG_FATAL(format_str, ...)    NestVTT::Logging::SubmitMessage(NestVTT::Logging::LogLevel::Fatal,    std::vformat(format_str, std::make_format_args( __VA_ARGS__)))
#define LOG_WARNING(format_str, ...)  NestVTT::Logging::SubmitMessage(NestVTT::Logging::LogLevel::Warning,  std::vformat(format_str, std::make_format_args( __VA_ARGS__)))
#define LOG_ERROR(format_str, ...)    NestVTT::Logging::SubmitMessage(NestVTT::Logging::LogLevel::Error,    std::vformat(format_str, std::make_format_args( __VA_ARGS__)))
#define LOG_INFO(format_str, ...)     NestVTT::Logging::SubmitMessage(NestVTT::Logging::LogLevel::Info,     std::vformat(format_str, std::make_format_args( __VA_ARGS__)))

#ifdef CORE_DEBUG
#define LOG_DEBUG(format_str, ...)    NestVTT::Logging::SubmitMessage(NestVTT::Logging::LogLevel::Debug,    std::vformat(format_str, std::make_format_args( __VA_ARGS__)))
#else
#define LOG_DEBUG(format_str, ...)
#endif

#define LOG_FLUSH() NestVTT::Logging::FlushLog()

#endif //RAVENSNESTVTT_LOGGER_H
