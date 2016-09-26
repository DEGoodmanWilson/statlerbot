//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#pragma once

#include <luna/luna.h>
#include <slack/slack.h>
#include "team_info.h"
#include "beep_boop_persist.h"

class event_receiver
{
public:
    event_receiver(luna::server &server, beep_boop_persist &store, const std::string &verification_token);

    void handle_error(std::string message, std::string received);
    void handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope);

    void handle_join_channel(std::shared_ptr<slack::event::message_channel_join> event, const slack::http_event_envelope &envelope);

    void handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope);

private:
    luna::server &server_;
    slack::http_event_client handler_;
    beep_boop_persist &store_;

    bool get_companion_info_(const slack::token &token, team_info &info);
    void handle_message_internal_(const slack::token &token, const slack::channel_id &channel_id);
};

using namespace luna;
