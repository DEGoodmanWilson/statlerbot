//
// Created by D.E. Goodman-Wilson on 8/8/16.
//

#pragma once

#include <string>
#include <map>

// for now, let's just store this in memory; we can adapt this to a persistence layer later

class token_storage
{
public:
    void set_token(const std::string &team_id, const std::string &token);

    //not super fond of this in/out second parameter.
    bool get_token_for_team(const std::string &team_id, std::string &token);
private:
    std::map<std::string, std::string> storage_;
};
