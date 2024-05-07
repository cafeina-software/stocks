#ifndef __ADDKEY_DLG_H_
#define __ADDKEY_DLG_H_

#include <InterfaceKit.h>
#include <SupportDefs.h>

#define AKDLG_VALUE_DATA       'data'
#define AKDLG_VALUE_ID         'id  '
#define AKDLG_SAVE_REQUESTED   'save'
#define AKDLG_CANCEL_REQUESTED 'cncl'

class AddKeyDialogBox : public BWindow {
public:
                  AddKeyDialogBox(BRect frame, const char* providerName = "",
                    const char* providerUrl = "", const char* saveLocation = "",
                    const char* defaultId = "STOCKS_APP_API_KEY",
                    bool firstLaunchFlag = false);
    virtual      ~AddKeyDialogBox() = default;
    virtual void  MessageReceived(BMessage* msg);
private:
    bool          _IsDataValid();
private:
    BString       currentId,
                  currentApiKey;
    bool          isFirstLaunch;

    BTextControl *fKeyData,
                 *fKeyIdentifier;
    BButton      *fSaveButton,
                 *fCancelButton;
};

#endif /* __ADDKEY_DLG_H_ */
