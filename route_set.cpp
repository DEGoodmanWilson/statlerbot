//
// Created by D.E. Goodman-Wilson on 8/13/16.
//

#include "route_set.h"

route_set::route_set(luna::server *server) : server_{server}
{

}

route_set::~route_set()
{
    for (auto handle : handles_)
    {
        server_->remove_request_handler(handle);
    }
}

void route_set::add_route(luna::server::request_handler_handle handle)
{
    handles_.emplace_back(handle);
}