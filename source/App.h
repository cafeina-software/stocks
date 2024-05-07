#ifndef APP_H
#define APP_H

#include <Application.h>
#include "api/FinancialmodelingApiKey.h"

extern FinancialmodelingApiKey fmApiKey;

class App : public BApplication {
public:
    App(BString providedKey);
    virtual void ReadyToRun();
    virtual void MessageReceived(BMessage* msg);
    virtual bool QuitRequested();

    status_t LoadApiKey(BString* apikey);
    status_t AddApiKey(BString data, BString id);

    static void checkForApiKey();
private:
    status_t _LoadSettings();
    status_t _SaveSettings();
    void     _InitWindow();

    status_t LoadAppKeyring();
    status_t LoadApiKeyFromKeyStore(BString* outApiKey,
        BString inKeyIdentifier = "STOCKS_APP_API_KEY");
private:
    BWindow *window{};
    BMessage currentSettings;
    BRect frame;
};

#endif
