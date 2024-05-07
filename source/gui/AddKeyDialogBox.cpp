#include <InterfaceKit.h>
#include <Application.h>
#include <Catalog.h>
#include "AddKeyDialogBox.h"
#include "../StocksDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Add_key_dialog_box"

AddKeyDialogBox::AddKeyDialogBox(BRect frame, const char* providerName,
    const char* providerUrl, const char* saveLocation, const char* defaultId,
    bool firstLaunchFlag)
: BWindow(frame, B_TRANSLATE("Add key"), B_FLOATING_WINDOW_LOOK,
    B_FLOATING_ALL_WINDOW_FEEL,
    B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS),
  currentId(defaultId),
  currentApiKey(""),
  isFirstLaunch(firstLaunchFlag)
{
    BString description(B_TRANSLATE("This application needs an API key to access "
        "the data from \nthe data provider service %serviceProv% in order \n"
        "to work. Please write it down below. It will be saved in \n"
        "%saveLocation%."));
    description.ReplaceAll("%serviceProv%", providerName);
    description.ReplaceAll("%saveLocation%", saveLocation);
    BStringView* descriptionView = new BStringView("sv_desc", description.String());

    BFont annotationFont;
    float annotationSize = annotationFont.Size() * 0.9;
    annotationFont.SetFace(B_ITALIC_FACE);
    annotationFont.SetSize(annotationSize);
    BString providerInfo(B_TRANSLATE("You can get a %providerName% API key by "
        "signing in at \n%providerUrl%."));
    providerInfo.ReplaceAll("%providerName%", providerName);
    providerInfo.ReplaceAll("%providerUrl%", providerUrl);
    BStringView* providerInfoView = new BStringView("sv_prov", providerInfo.String());
    providerInfoView->SetFont(&annotationFont);

    fKeyData = new BTextControl("tc_data", B_TRANSLATE("API Key"),
        currentApiKey, new BMessage(AKDLG_VALUE_DATA));

    // fKeyIdentifier = new BTextControl("tc_id",
        // B_TRANSLATE("Identifier (for internal use)"), currentId,
        // new BMessage(AKDLG_VALUE_ID));

    fSaveButton = new BButton("bt_save", isFirstLaunch ?
        B_TRANSLATE("Save key and start") : B_TRANSLATE("Save key"),
        new BMessage(AKDLG_SAVE_REQUESTED));
    fSaveButton->SetEnabled(false);

    fCancelButton = new BButton("bt_cancel", isFirstLaunch ? B_TRANSLATE("Quit") :
        B_TRANSLATE("Cancel"), new BMessage(AKDLG_CANCEL_REQUESTED));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .SetInsets(B_USE_WINDOW_INSETS)
        .Add(descriptionView)
        .AddStrut(4.0f)
        .Add(providerInfoView)
        .AddGrid()
            .AddTextControl(fKeyData, 0, 0)
            // .AddTextControl(fKeyIdentifier, 0, 1)
        .End()
        .AddStrut(8.0f)
        .AddGroup(B_HORIZONTAL)
            .Add(fSaveButton)
            .Add(fCancelButton)
        .End()
    .End();

    CenterOnScreen();
}

void AddKeyDialogBox::MessageReceived(BMessage* msg)
{
    switch(msg->what)
    {
        case AKDLG_VALUE_DATA:
            currentApiKey = fKeyData->Text();
            fSaveButton->SetEnabled(_IsDataValid());
            break;
        case AKDLG_VALUE_ID:
            currentId = fKeyIdentifier->Text();
            fSaveButton->SetEnabled(_IsDataValid());
            break;
        case AKDLG_SAVE_REQUESTED:
            if(_IsDataValid()) {
                BMessage keymessage(M_USER_PROVIDES_KEY);
                keymessage.AddString("key", currentApiKey.String());
                keymessage.AddString("id", currentId.String());
                be_app->PostMessage(&keymessage);
                Quit();
            }
            break;
        case AKDLG_CANCEL_REQUESTED:
            if(isFirstLaunch)
                be_app->PostMessage(M_USER_CANCELLED_ADD_KEY);
            Quit();
            break;
        default:
            BWindow::MessageReceived(msg);
            break;
    }
}

bool AddKeyDialogBox::_IsDataValid()
{
    return !currentApiKey.IsEmpty() && !currentId.IsEmpty();
}
