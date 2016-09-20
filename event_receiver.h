//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#pragma once

#include <luna/luna.h>
#include "route_set.h"
#include <slack/http_event_client.h>
#include <deque>

using namespace luna;

class event_receiver : public route_set
{
public:
    event_receiver(server* server, const std::string &verification_token);

    void handle_error(std::string message, std::string received);
    void handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope);

    void handle_join_channel(std::shared_ptr<slack::event::message_channel_join> event, const slack::http_event_envelope &envelope);

    void handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope);
private:
    slack::http_event_client handler_;
};
