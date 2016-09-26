//
// Created by D.E. Goodman-Wilson on 9/26/16.
//

#pragma once

#include <vector>
#include <string>
#include <slack/slack.h>

struct team_info
{
    std::string to_json();

    slack::user_id companion_user_id;
    slack::bot_id companion_bot_id;
};

team_info from_json(const std::string &str);