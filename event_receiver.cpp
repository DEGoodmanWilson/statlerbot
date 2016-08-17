//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#include "event_receiver.h"
#include <iostream>


void event_receiver::handle_error(std::string message, std::string received)
{
    std::cout << message << " " << received << std::endl;
}

void event_receiver::handle_unknown(std::shared_ptr<slack::event::unknown> event, const slack::http_event_envelope &envelope)
{
    std::cout << event->type << std::endl;
}

void event_receiver::handle_message(std::shared_ptr<slack::event::message> event, const slack::http_event_envelope &envelope)
{
    std::cout << event->type << ": " << event->text << std::endl;
}


event_receiver::event_receiver(server *server, token_storage *store, const std::string &verification_token) : route_set{server}, handler{}
{
    handler.register_error_handler(std::bind(&event_receiver::handle_error, this, std::placeholders::_1, std::placeholders::_2));
    handler.register_event_handler<slack::event::unknown>(std::bind(&event_receiver::handle_unknown, this, std::placeholders::_1, std::placeholders::_2));
    handler.register_event_handler<slack::event::message>(std::bind(&event_receiver::handle_message, this, std::placeholders::_1, std::placeholders::_2));

    server->handle_request(request_method::POST, "/events", [&, verification_token](auto req) -> response
    {
            auto response = handler.handle_event(req.body);
            return {response};
    });
}
