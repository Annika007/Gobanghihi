//
// Created by Cyril on 2023/11/14.
//

#include "Message.h"
#include "raylib.h"

Message::Message(const char *message) {
    m_message = (char *) message;
    m_id = GAME_OBJECT_ID::MESSAGE_ID;
}

void Message::Draw() {
    const int w = 300, h = 80;
    int x = 500 - w / 2, y = 60 - h / 2;
    DrawRectangle(x, y, w, h, { 255, 200, 0, 100 });
    DrawText(m_message, x + 40, y + 20, 40, DARKBROWN);
}

Event Message::Update() {
    return EVENT_NONE;
}
