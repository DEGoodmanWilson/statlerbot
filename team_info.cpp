//
// Created by D.E. Goodman-Wilson on 9/26/16.
//

#include <sstream>
#include "team_info.h"
#include <json/json.h>

std::string team_info::to_json()
{
    Json::Value res;
    res["companion_user_id"] = companion_user_id;
    res["companion_bot_id"] = companion_bot_id;
    std::stringstream out;
    out << res;
    return out.str();
}

team_info from_json(const std::string &str)
{
    team_info info;
    Json::Value obj;
    Json::Reader reader;
    bool parsed_success = reader.parse(str, obj, false);
    if(parsed_success)
    {
        if (obj["companion_user_id"].isString())
        {
            info.companion_user_id = obj["companion_user_id"].asString();
        }
        if (obj["companion_bot_id"].isString())
        {
            info.companion_bot_id = obj["companion_bot_id"].asString();
        }
    }

    return info;
}