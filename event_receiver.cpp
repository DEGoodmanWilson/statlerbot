//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <iostream>


void event_receiver::handle_error(std::string message, std::string received)
{
    std::cout << message << " " << received << std::endl;
}

void
event_receiver::handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope)
{
    std::cout << event->type << std::endl;
}

void
event_receiver::handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope)
{
    std::cout << event->type << ": " << event->text << std::endl;
    router_.route(*event);
}

event_receiver::event_receiver(server *server, token_storage *store, const std::string &verification_token) :
        route_set{server},
        handler_{verification_token},
        router_{[=](const slack::team_id team_id) -> std::string
                    {
                        std::string token;
                        if (store->get_token_for_team(team_id, token))
                        {
                            return token;
                        }
                        return "";
                    }}
{
    handler_.register_error_handler(std::bind(&event_receiver::handle_error,
                                              this,
                                              std::placeholders::_1,
                                              std::placeholders::_2));
    handler_.register_event_handler<slack::event::unknown>(std::bind(&event_receiver::handle_unknown,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    handler_.register_event_handler<slack::event::message>(std::bind(&event_receiver::handle_message,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));

    server->handle_request(request_method::POST, "/events", [&](auto req) -> response
        {
            return {handler_.handle_event(req.body)};
        });

    router_.hears(std::regex{"hello", std::regex_constants::ECMAScript}, [](const auto &message)
        {
            std::cout << message.text << std::endl;
            message.reply("Hello");
//            slack::chat.postMessage();
        });
}
