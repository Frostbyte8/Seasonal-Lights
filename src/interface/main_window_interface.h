#pragma once

namespace MainWindowInterfaceResponses {
    static const int YES    = 6;
    static const int NO     = 7;
    static const int CANCEL = 2;
}

__interface MainWindowInterface {
    public:
        virtual void implDisplayMessage(const wchar_t* message, const wchar_t* title) = 0;
        //virtual int implAskYesNoQuestion(const wchar_t* message, const wchar_t* title) = 0;       
};