#pragma once
#include <cstdint>
#include <optional>
#include "process.hpp"
#include "raiitimer.hpp"
#include "servermsg.hpp"
#include "labelboard.hpp"
#include "notifyboard.hpp"
#include "passwordbox.hpp"
#include "tritexbutton.hpp"
#include "inputstringboard.hpp"

class ProcessSelectChar: public Process
{
    private:
        TritexButton m_start;
        TritexButton m_create;
        TritexButton m_delete;
        TritexButton m_exit;

    private:
        NotifyBoard m_notifyBoard;
        InputStringBoard m_deleteInput;

    private:
        std::optional<SMQueryCharOK> m_smChar;

    public:
        ProcessSelectChar();

    public:
        int id() const override
        {
            return PROCESSID_CREATEACCOUNT;
        }

    public:
        void update(double) override;
        void draw() override;
        void processEvent(const SDL_Event &) override;

    private:
        void onStart();
        void onCreate();
        void onDelete();
        void onExit();

    public:
        void net_QUERYCHAROK    (const uint8_t *, size_t);
        void net_QUERYCHARERROR (const uint8_t *, size_t);
        void net_DELETECHAROK   (const uint8_t *, size_t);
        void net_DELETECHARERROR(const uint8_t *, size_t);
        void net_ONLINEOK       (const uint8_t *, size_t);
        void net_ONLINEERROR    (const uint8_t *, size_t);
};
