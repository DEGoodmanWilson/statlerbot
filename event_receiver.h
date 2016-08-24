//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#pragma once

#include <luna/luna.h>
#include "token_storage.h"
#include "route_set.h"
#include <slack/http_event_client.h>

using namespace luna;

class event_receiver : public route_set
{
public:
    event_receiver(server* server, token_storage* store, const std::string &verification_token);

    void handle_error(std::string message, std::string received);
    void handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope);

    void handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope);
private:
    slack::http_event_client handler_;
    token_storage *store_;
};
