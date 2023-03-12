//
// Created by Thorsten Claus on 07.03.23.
//

#ifndef STOCKS_STOCKREQUESTS_H
#define STOCKS_STOCKREQUESTS_H


class StockConnector {

public:
    virtual void
    Search(const char *searchQuery) = 0;
    //
    // void RequestBatchData(vector<String> symbols); // Aktualisiere alle Aktien aus dem Portfolio
    // void RequestHistoricalPrices(char* symbol, period); // 1 day, 1 Woche, 1 Monat, 3 Monate, 6 Monate, 1,2,5 10 Jahre
    // void RequestCompanyProfile(char* symbol); //https://financialmodelingprep.com/image-stock/AAPL.png

};

#endif //STOCKS_STOCKREQUESTS_H
