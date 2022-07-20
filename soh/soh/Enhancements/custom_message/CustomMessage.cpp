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
    this->colors.clear();
    this->messageTables.clear();
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

void CustomMessage::FormatCustomMessage(std::string& message, ItemID iid) {
    message.insert(0, ITEM_OBTAINED(iid));
    size_t start_pos = 0;
    std::replace(message.begin(), message.end(), '&', NEWLINE()[0]);
    while ((start_pos = message.find('^', start_pos)) != std::string::npos) {
        message.replace(start_pos, 1, WAIT_FOR_INPUT() + ITEM_OBTAINED(iid));
        start_pos += 3;
    }
    std::replace(message.begin(), message.end(), '@', PLAYER_NAME()[0]);
    ReplaceSpecialCharacters(message);
    ReplaceColors(message);
    message += MESSAGE_END();
}

void CustomMessage::FormatCustomMessage(std::string& message) {
    size_t start_pos = 0;
    std::replace(message.begin(), message.end(), '&', NEWLINE()[0]);
    while ((start_pos = message.find('^', start_pos)) != std::string::npos) {
        message.replace(start_pos, 1, WAIT_FOR_INPUT());
        start_pos += 3;
    }
    std::replace(message.begin(), message.end(), '@', PLAYER_NAME()[0]);
    ReplaceSpecialCharacters(message);
    ReplaceColors(message);
    message += MESSAGE_END();
}

bool CustomMessage::InsertCustomMessage(std::string tableID, uint16_t textID, CustomMessageEntry messages) {
    auto result = messageTables.find(tableID);
    if (result == messageTables.end()) {
        return false;
    }
    auto& messageTable = result->second;
    auto success = messageTable.emplace(textID, messages);
    return success.second;
}



bool CustomMessage::CreateGetItemMessage(std::string tableID, GetItemID giid, ItemID iid, CustomMessageEntry messages) {
    FormatCustomMessage(messages.english, iid);
    FormatCustomMessage(messages.german, iid);
    FormatCustomMessage(messages.french, iid);
    const uint16_t textID = giid;
    return InsertCustomMessage(tableID, textID, messages);
}

bool CustomMessage::CreateMessage(std::string tableID, uint16_t textID, CustomMessageEntry messages) {
    FormatCustomMessage(messages.english);
    FormatCustomMessage(messages.german);
    FormatCustomMessage(messages.french);
    return InsertCustomMessage(tableID, textID, messages);
}

std::string CustomMessage::RetrieveMessage(GlobalContext* globalCtx, std::string tableID, uint16_t textID) {
    std::unordered_map<std::string, CustomMessageTable>::const_iterator result = messageTables.find(tableID);
    if (result == messageTables.end()) {
        return "";
    }
    CustomMessageTable messageTable = result->second;
    std::unordered_map<uint16_t, CustomMessageEntry>::const_iterator message_pair = messageTable.find(textID);
    if (message_pair == messageTable.end()) {
        return "";
    }
    CustomMessageEntry messages = message_pair->second;
    MessageContext* msgCtx = &globalCtx->msgCtx;
    Font* font = &msgCtx->font;
    font->charTexBuf[0] = (messages.textBoxType << 4) | messages.textBoxPos;
    switch (gSaveContext.language) { 
        case LANGUAGE_FRA:
            return messages.french;
        case LANGUAGE_GER:
            return messages.german;
        case LANGUAGE_ENG:
        default:
            return messages.english;
    }
}

bool CustomMessage::AddCustomMessageTable(std::string tableID) { 
    CustomMessageTable newMessageTable;
    return messageTables.emplace(tableID, newMessageTable).second;
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