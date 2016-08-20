//
// Created by D.E. Goodman-Wilson on 8/19/16.
//

#include "message_router.h"

bool message_router::route(const slack::event::message &message)
{
    //TODO I am not super happy with this, and I don't really have a good clue how to improve it.
    for (const auto &handler_pair : callbacks_)
    {
        std::smatch pieces_match;
        auto message_regex = std::get<std::regex>(handler_pair);

        auto callback = std::get<hears_cb>(handler_pair);
        if (std::regex_search(message.text, pieces_match, message_regex))
        {
            callback(message); //TODO or we could return the retval from the callback, have it return true if handled and false if it chose not to
            return true;
        }
    }

    return false;
}