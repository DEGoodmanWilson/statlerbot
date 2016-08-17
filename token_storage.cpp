//
// Created by D.E. Goodman-Wilson on 8/8/16.
//

#include "token_storage.h"

void token_storage::set_token(const std::string &team_id, const std::string &token)
{
    storage_[team_id] = token;
}

bool token_storage::get_token_for_team(const std::string &team_id, std::string &token)
{
    if(!storage_.count(team_id)) return false;

    token = storage_.at(team_id);
    return true;
}