//
// Created by D.E. Goodman-Wilson on 8/13/16.
//

#pragma once

#include <luna/luna.h>

class route_set
{
public:
    route_set(luna::server *server);
    ~route_set();

    void add_route(luna::server::request_handler_handle handle);

private:
    luna::server *server_; //assume that the server will outlive us, and that we do not own a handle to it.
    std::vector<luna::server::request_handler_handle> handles_;
};
