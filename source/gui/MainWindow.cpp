#include "MainWindow.h"

#include "DetailsView.h"
#include "stocksPanel/SearchFieldControl.h"
#include <Application.h>
#include <Catalog.h>
#include <InterfaceKit.h>
#include <LayoutBuilder.h>
#include "utils/EscapeCancelFilter.h"
#include <Window.h>
#include <private/netservices2/NetServicesDefs.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


MainWindow::MainWindow(BRect frame)
        : BWindow(frame, B_TRANSLATE_SYSTEM_NAME("Stocks"), B_TITLED_WINDOW,
                  B_ASYNCHRONOUS_CONTROLS),
          fQuoteResultHandler(new QuoteResultHandler()) {

    SetWindowSizeLimits();
    InitViews();

    BLayoutBuilder::Group<>((BWindow *) this, B_HORIZONTAL, 0)
            .SetInsets(0)
            .Add(fStocksPanelView, 100)
            .Add(fChartView, 1);
    InitWorker();
}

void MainWindow::InitViews() {
    fStocksPanelView = new StocksPanelView();
    fChartView = new DetailsView();
}

MainWindow::~MainWindow() {
    fDelayedQueryTimer->StopThread();
    fQuoteUpdateJob->StopThread();

    delete fQuoteResultHandler;
}

void
MainWindow::InitWorker() {
    fDelayedQueryTimer = new DelayedQueryTimer(this);
    fDelayedQueryTimer->StartThread();

    fQuoteUpdateJob = new QuoteUpdateJob(this);
    fQuoteUpdateJob->StartThread();

    AddCommonFilter(new EscapeCancelFilter());
}

void
MainWindow::SetWindowSizeLimits() {
    BRect screenFrame = (BScreen(this)).Frame();
    SetSizeLimits(700.0, screenFrame.Width(), 500.0, screenFrame.Height());
}

void
MainWindow::Show() {
    BWindow::Show();
}

void
MainWindow::MessageReceived(BMessage *message) {
    switch (message->what) {
        case (TimeRange::M_DAY):
        case (TimeRange::M_WEEK):
        case (TimeRange::M_MONTH):
        case (TimeRange::M_THREE_MONTH):
        case (TimeRange::M_SIX_MONTH):
        case (TimeRange::M_YEAR):
        case (TimeRange::M_TWO_YEARS):
        case (TimeRange::M_FIVE_YEARS): {
            // TODO:  How can a time range button directly send a message to its parent?
            // Dont like this detour
            TimeRange timeRange = static_cast<TimeRange>(message->what);
            fChartView->SetTimeRange(timeRange);
            break;
        }

        case (SearchFieldMessages::M_START_SHARES_SEARCH) : {
            BString searchTerm;
            if (message->FindString(SEARCH_TERM, &searchTerm) != B_OK) {
                searchTerm = "";
            }
            RequestForSearch(searchTerm);
            break;
        }
        case (BPrivate::Network::UrlEvent::RequestCompleted): {
            ResultHandler(message->GetInt32(BPrivate::Network::UrlEventData::Id, -1));
            break;
        }
        case (SearchFieldMessages::M_DISMISS_SEARCH) : {
            fStocksPanelView->DismissSearch();
            break;
        }
        case (SearchFieldMessages::M_ACCEPT_SELECTION) : {
            fStocksPanelView->AcceptSearch();
            break;
        }
        case SearchFieldMessages::M_SET_STOCK: {
            fStocksPanelView->StockSelected();
            break;
        }
        case (DelayedQueryTimerMessages::CHARACTER_DELAY_EXPIRED) : {
            const char *searchQuery = message->FindString(SEARCH_FOR_TEXT);
            fStocksPanelView->SearchForSymbol(searchQuery);
            break;
        }
        default: {
            BWindow::MessageReceived(message);
            break;
        }
    }
}

void
MainWindow::ResultHandler(int requestId) {
    fStocksPanelView->HandleResult(requestId);
    fQuoteResultHandler->HandleQuoteResults(requestId);
}

void
MainWindow::RequestForSearch(BString &searchTerm) {
    fDelayedQueryTimer->RunQuery(new std::string(searchTerm.String()));
}

bool
MainWindow::QuitRequested() {
    be_app->PostMessage(B_QUIT_REQUESTED);
    return true;
}
