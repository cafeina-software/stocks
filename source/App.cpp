#ifndef APP_CPP_
#define APP_CPP_

#include <StorageKit.h>
#include <Catalog.h>
#include <KeyStore.h>
#include "App.h"
#include "../StocksDefs.h"
#include "gui/AddKeyDialogBox.h"
#include "gui/MainWindow.h"
#include "FinancialmodelingApiKey.h"

FinancialmodelingApiKey fmApiKey;

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"

App::App(BString providedKey)
: BApplication("application/x-vnd.tclaus-StocksApp"),
  frame(BRect(100, 100, 500, 400)) {
    // If there was not a command-line provided key, search in keystore
    //    or in environment variable
    if(providedKey.IsEmpty()) {
        BString apikey;
        status_t status = LoadApiKey(&apikey);
        if(status == B_OK && !apikey.IsEmpty()) {
            fmApiKey.SetApiKey(apikey.String());
        }
    }

    _LoadSettings();
}

void App::ReadyToRun()
{
    if(!fmApiKey.ValidateApiKey()) {
        AddKeyDialogBox* dlg = new AddKeyDialogBox(BRect(100, 100, 640, 360),
            "Financial Modeling Prep",
            "https://site.financialmodelingprep.com/",
            B_TRANSLATE("the system keystore"), "STOCKS_APP_API_KEY", true);
        // the dialog could also offer the option to save in the UBS
        dlg->Show();
    }
    else {
        _InitWindow();
    }
}

void App::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case B_QUIT_REQUESTED:
            QuitRequested();
            break;
        case M_USER_PROVIDES_KEY:
        {
            BKey key;
            BString keyid, keydata;
            if(msg->FindString("id", &keyid) == B_OK &&
            msg->FindString("key", &keydata) == B_OK) {
                if(AddApiKey(keydata, keyid) == B_OK) {
                    fmApiKey.SetApiKey(keydata.String());
                    _InitWindow();
                }
                else {
                    fprintf(stderr, "Key could not be saved.\n");
                    QuitRequested();
                }
            }
            break;
        }
        case M_USER_CANCELLED_ADD_KEY:
            PostMessage(B_QUIT_REQUESTED);
            break;
        default:
            BApplication::MessageReceived(msg);
            break;
    }
}

bool App::QuitRequested()
{
    _SaveSettings();
    return BApplication::QuitRequested();
}

status_t App::_LoadSettings()
{
    status_t status = B_OK;
    BPath usrsetpath;
    if((status = find_directory(B_USER_SETTINGS_DIRECTORY, &usrsetpath)) != B_OK) {
        return status;
    }

    usrsetpath.Append(kAppName "/gui.settings");
    BFile file(usrsetpath.Path(), B_READ_ONLY);
    if((status = file.InitCheck()) != B_OK) {
        return status;
    }

    if((status = currentSettings.Unflatten(&file)) != B_OK) {
        return status;
    }

    if(currentSettings.FindRect("frame", &frame) != B_OK)
        return B_ERROR;

    return status;
}

status_t App::_SaveSettings()
{
    status_t status = B_OK;

    if(window) {
        if(currentSettings.ReplaceRect("frame", window->Frame()) != B_OK)
            currentSettings.AddRect("frame", window->Frame());
    }

    BPath usrsetpath;
    if((status = find_directory(B_USER_SETTINGS_DIRECTORY, &usrsetpath)) != B_OK) {
        return status;
    }

    usrsetpath.Append(kAppName "/gui.settings");
    BFile file(usrsetpath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
    if((status = file.InitCheck()) != B_OK) {
        return status;
    }
    file.SetPermissions(DEFFILEMODE);

    if((status = currentSettings.Flatten(&file)) != B_OK) {
        return status;
    }

    return status;
}

void App::_InitWindow()
{
    window = new MainWindow(frame);
    window->Show();
}

void
App::checkForApiKey() {
    if (fmApiKey.ValidateApiKey()) {
        const char *no_api_key_set = "No API key set in build. Rebuild app with API key set in the environment. "
                                     "Have a look in the readme file.";
        // throw std::invalid_argument(no_api_key_set);
        printf("%s\n", no_api_key_set);

    }
}

status_t App::LoadApiKey(BString* outApikey)
{
    BString apikey;
    status_t status = B_ERROR;

    status = LoadApiKeyFromKeyStore(&apikey, "STOCKS_APP_API_KEY");
    if(status == B_OK && !apikey.IsEmpty()) { // Find in KeyStore
        *outApikey = apikey;
    }
    else { // fallback location in the environment, to keep compat with older versions
        const char* env_var = std::getenv("STOCKS_APP_API_KEY");
        if(env_var != nullptr) {
            *outApikey = env_var;
            status = B_OK;
        }
    }

    return status;
}

status_t App::LoadAppKeyring()
{
    status_t status = B_ERROR;
    BKeyStore keystore;
    uint32 cookie = 0;
    BString currentkeyring;
    bool next = true;

    // Find if the app's keyring exists, it creates one if there is none
    while(next) {
        switch(keystore.GetNextKeyring(cookie, currentkeyring)) {
            case B_OK: // a keyring was found, let's check if it is the proper one
                if(strcmp(kAppKeyringName, currentkeyring.String()) == 0) {
                    next = false;
                    status = B_OK;
                }
                break;
            case B_ENTRY_NOT_FOUND: // app's keyring was not found, let's create one
                status = keystore.AddKeyring(kAppKeyringName);
                if(status == B_OK || status == B_NAME_IN_USE) {
                    status = B_OK; // if it exists, then it's ok anyway
                }
                else
                    fprintf(stderr, "Keyring could not be created\n");
                next = false;
                break;
            default: // error trying to communicate with keystore_server
                fprintf(stderr, "keystore_server communication error\n");
                next = false;
                break;
        }
    }

    return status;
}

status_t App::LoadApiKeyFromKeyStore(BString* outApiKey, BString inKeyIdentifier)
{
    BKeyStore keystore;
    status_t status = B_ERROR;

    if((status = LoadAppKeyring()) != B_OK)
        return status;

    BKey key;
    status = keystore.GetKey(kAppKeyringName, B_KEY_TYPE_GENERIC,
        inKeyIdentifier.String(), key);

    switch(status) {
        case B_OK:
            *outApiKey = reinterpret_cast<const char*>(key.Data());
            break;
        case B_ENTRY_NOT_FOUND:
        case B_BAD_VALUE:
        case B_NOT_ALLOWED:
        default:
            break;
    }

    return status;
}

status_t App::AddApiKey(BString data, BString id)
{
    BKeyStore keystore;
    status_t status = keystore.AddKey(kAppKeyringName, BKey(B_KEY_PURPOSE_WEB,
        id.String(), NULL, reinterpret_cast<const uint8*>(data.String()),
        strlen(data.String())));

    return status;
}

// #pragma mark main

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "main"

static void helpmessage() {
    printf(B_TRANSLATE_COMMENT(
        "%s\n"
        "Usage: %s [option]\n"
        "Options include:\n"
        "--help                   Shows the help (this message).\n"
        "--version                Shows the application version.\n"
        "--use-api-key <Key>      Starts the application using the <Key> API key.\n",
        "Do not translate flags' names (those looking like \"--flag\""),
        kAppName, kAppName);
}

static int option(const char* op) {
    if(strcmp(op, "--help") == 0 || strcmp(op, "-h") == 0)
        return 1;
    else if(strcmp(op, "--version") == 0)
        return 2;
    else if(strcmp(op, "--use-api-key") == 0)
        return 3;
    else
        return 0;
}

int main(int argc, char* argv[]) {
    BString apikey("");

    if(argc > 1) { // if no args were provided, the app is launched right away
        switch(option(argv[1]))
        {
            case 2:
                printf(B_TRANSLATE("%s. Version %s.\n"), kAppName, "1");
                return 0;
            case 3:
                if(argc == 3) { // user provisioned api key via command line
                    apikey = argv[2];
                    break;
                }
                else {
                    printf(B_TRANSLATE("Error: invalid use of --use-api-key. "
                        "Please read the help message."));
                    return -1; // Terminal prompt changes to red
                }
            default:
                helpmessage();
                return 0;
        }
    }
    // App::checkForApiKey();

    App *app = new App(apikey.String());
    app->Run();
    delete app;
    return 0;
}

#endif // APP_CPP_
