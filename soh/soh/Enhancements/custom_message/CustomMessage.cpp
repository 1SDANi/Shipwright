#include "CustomMessage.h"
#include <algorithm>

using namespace std::literals::string_literals;

CustomMessage::CustomMessage() {
    this->textBoxSpecialCharacters = { { "�", 0x80 }, { "�", 0x81 }, { "�", 0x82 }, { "�", 0x83 }, { "�", 0x84 },
                                       { "�", 0x85 }, { "�", 0x86 }, { "�", 0x87 }, { "�", 0x88 }, { "�", 0x89 },
                                       { "�", 0x8A }, { "�", 0x8B }, { "�", 0x8C }, { "�", 0x8D }, { "�", 0x8E },
                                       { "�", 0x8F }, { "�", 0x90 }, { "�", 0x91 }, { "�", 0x92 }, { "�", 0x93 },
                                       { "�", 0x94 }, { "�", 0x95 }, { "�", 0x96 }, { "�", 0x97 }, { "�", 0x98 },
                                       { "�", 0x99 }, { "�", 0x9A }, { "�", 0x9B }, { "�", 0x9C }, { "�", 0x9D },
                                       { "�", 0x9E } };
    this->colors = { { "w", QM_WHITE }, { "r", QM_RED },  { "g", QM_GREEN },  { "b", QM_BLUE },
                     { "c", QM_LBLUE }, { "p", QM_PINK }, { "y", QM_YELLOW }, { "B", QM_BLACK } };
}

CustomMessage::~CustomMessage() {
    this->textBoxSpecialCharacters.clear();
}

void CustomMessage::ReplaceSpecialCharacters(std::string& string) {
    // add special characters
    for (auto specialCharacterPair : textBoxSpecialCharacters) {
        size_t start_pos = 0;
        std::string textBoxSpecialCharacterString = "";
        textBoxSpecialCharacterString += specialCharacterPair.second;
        while ((start_pos = string.find(specialCharacterPair.first, start_pos)) != std::string::npos) {
            string.replace(start_pos, specialCharacterPair.first.length(), textBoxSpecialCharacterString);
            start_pos += textBoxSpecialCharacterString.length();
        }
    }
}

void CustomMessage::ReplaceColors(std::string& string) {
    for (auto colorPair : colors) {
        std::string textToReplace = "%";
        textToReplace += colorPair.first;
        size_t start_pos = 0;
        while ((start_pos = string.find(textToReplace)) != std::string::npos) {
            string.replace(start_pos, textToReplace.length(), COLOR(colorPair.second));
            start_pos += textToReplace.length();
        }
    }
}

void CustomMessage::CreateGetItemMessage(GetItemID giid, ItemID iid, std::string messages[LANGUAGE_MAX]) {
    for (int i = 0; i < LANGUAGE_MAX; i++) {
        if (!(messages[i].empty())) {
            std::string message = messages[i];
            std::string formattedMessage = ITEM_OBTAINED(iid) + message;
            size_t start_pos = 0;
            std::replace(formattedMessage.begin(), formattedMessage.end(), '&', NEWLINE()[0]);
            while ((start_pos = formattedMessage.find('^', start_pos)) != std::string::npos) {
                formattedMessage.replace(start_pos, 1, WAIT_FOR_INPUT() + ITEM_OBTAINED(iid));
                start_pos += 3;
            }
            std::replace(formattedMessage.begin(), formattedMessage.end(), '@', PLAYER_NAME()[0]);
            ReplaceSpecialCharacters(formattedMessage);
            ReplaceColors(formattedMessage);
            formattedMessage += MESSAGE_END();
            this->getItemMessageTable[i].emplace(giid, formattedMessage);
        } else {
            this->getItemMessageTable[i].emplace(giid, MESSAGE_END());
        }
    }
}

std::string CustomMessage::RetrieveGetItemMessage(GetItemID giid) {
    std::unordered_map<GetItemID, std::string>::const_iterator result =
        getItemMessageTable[gSaveContext.language].find(giid);
    if (result == getItemMessageTable[gSaveContext.language].end()) {
        switch (gSaveContext.language) {
            case LANGUAGE_FRA:
                return "Il n'y a pas de message personnalis� pour cet �l�ment.";
            case LANGUAGE_GER:
                return "F�r diesen Artikel gibt es keine benutzerdefinierte Nachricht.";
            case LANGUAGE_ENG:
            default:
                return "There is no custom message for this item.";
        }
    }
    return result->second;
}

std::string CustomMessage::MESSAGE_END() {
    return "\x02"s;
}

std::string CustomMessage::ITEM_OBTAINED(uint8_t x) {
    return "\x13"s + char(x);
}

std::string CustomMessage::NEWLINE() {
    return "\x01"s;
}

std::string CustomMessage::COLOR(uint8_t x) {
    return "\x05"s + char(x);
}

std::string CustomMessage::WAIT_FOR_INPUT() {
    return "\x04"s;
}

std::string CustomMessage::PLAYER_NAME() {
    return "\x0F"s;
}