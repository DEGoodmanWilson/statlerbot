//
// Created by D.E. Goodman-Wilson on 8/8/16.
//

#include "token_storage.h"
#include <iostream>

void token_storage::set_token(const std::string &team_id, const token_info &token)
{
    auto collection = conn["statlerbot"]["tokens"];

    bsoncxx::builder::stream::document document{};
    document << "team_id" << team_id
             << "access_token" << token.access_token
             << "bot_token" << token.bot_token
             << "user_id" << token.user_id
             << "bot_id" << token.bot_id;

    collection.insert_one(document.view());
}

bool token_storage::get_token_for_team(const std::string &team_id, token_info &token)
{
    auto cursor = conn["statlerbot"]["tokens"].find(bsoncxx::builder::stream::document{} << "team_id"
                                                                                         << team_id
                                                                                         << bsoncxx::builder::stream::finalize);

    for (auto &&doc : cursor)
    {
        token.access_token = doc["access_token"].get_utf8().value.to_string();
        token.bot_token = doc["bot_token"].get_utf8().value.to_string();
        token.user_id = doc["user_id"].get_utf8().value.to_string();
        token.bot_id = doc["bot_id"].get_utf8().value.to_string();
        return true;
    }

    return false;
}