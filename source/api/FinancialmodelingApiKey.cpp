//
// Created by Thorsten Claus on 26.04.23.
//

#include "FinancialmodelingApiKey.h"

std::string FinancialmodelingApiKey::GetApiKey() {
    return fApiKey;
}

void FinancialmodelingApiKey::SetApiKey(const char* apikey) {
    fApiKey = apikey;
}

bool FinancialmodelingApiKey::ValidateApiKey()
{
    // what about also checking that the non-empty value is actually valid?
    // does FMApi provides such functionality?
    return !fApiKey.empty();
}
