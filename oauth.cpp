//
// Created by D.E. Goodman-Wilson on 8/8/16.
//

#include <sstream>
#include <slack/slack.h>
#include "oauth.h"

oauth::oauth(luna::server *server, token_storage *store, const std::string &scope, const std::string &client_id, const std::string &client_secret) : route_set{server}
{
    add_route(server->handle_request(request_method::GET, "/", [=](auto req) -> response
        {
            std::stringstream output;
            std::string state{"why"};
            //TODO state handling
            //TODO find a templating library
            output << "<h1>Sign in with Slack</h1>" << std::endl;
            output << "<a href = \"https://slack.com/oauth/authorize?scope=" << scope << "&client_id=" << client_id << "&state=" << state << "\">" << std::endl;
            output << "    <img alt=\"Add to Slack\", height=\"40\", width=\"139\", src=\"https://platform.slack-edge.com/img/add_to_slack.png\" srcset=\"https://platform.slack-edge.com/img/add_to_slack.png 1x, https://platform.slack-edge.com/img/add_to_slack@2x.png 2x\"></img>" << std::endl;
            output << "</a>" << std::endl;
            return {output.str()};
        }));

    add_route(server->handle_request(request_method::GET, "/oauth", [=](auto req) -> response
        {
            // This is the second half of the OAuth dance. We need to do a few things here.

            if(req.params.count("error"))
            {
                std::stringstream output;

                output << "<h1>Error: " << req.params["error"] << "</h1>" << std::endl;
                return {output.str()};
            }

            if(!req.params.count("state") || !req.params.count("code"))
            {
                return {"<h1>Error: missing necessary parameters</h1>"};
            }

            if(req.params["state"] != "why")
            {
                return {"<h1>Error: Invalid state</h1>"};
            }

            slack::slack slack;
            auto result = slack.oauth.access(client_id, client_secret, req.params["code"]);
            if(!result)
            {
                std::stringstream output;

                output << "<h1>Error: " << *result.error_message << "</h1>" << std::endl;
                return {output.str()};
            }

            // At this point we have a team id and an access token, let's store it.
            store->set_token(result.team_id, {result.access_token, result.bot->bot_access_token, result.user_id, result.bot->bot_user_id});

            return {"<h1>Success!</h1>"};
        }));
}