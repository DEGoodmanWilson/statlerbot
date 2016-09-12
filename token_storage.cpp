//
// Created by D.E. Goodman-Wilson on 8/8/16.
//

#include "token_storage.h"
#include <iostream>

void token_storage::set_token(const std::string &team_id, const token_info &token)
{
    _store[team_id] = token;
}

bool token_storage::get_token_for_team(const std::string &team_id, token_info &token)
{
    if(_store.count(team_id))
    {
        token = _store.at(team_id);
        return true;
    }

    return false;
}