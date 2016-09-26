//
// Created by D.E. Goodman-Wilson on 8/9/16.
//

#pragma once

// There are far more clever ways to do this. I'd like to use the subscript operator with a custom member class
// so you could use this just like a hash map. But, hey, that can come later.

#include <string>
#include <map>
#include <cpr/cpr.h>
#include "logging.h"

class beep_boop_persist
{
public:
    beep_boop_persist(const std::string &url, const std::string &token) :
            url_{url}, header_{{"Authorization", "Bearer " + token}}
    {
        if (url.empty())
        {
            in_memory_ = true;
            LOG(DEBUG) << "beep_boop_persist: Using in-memory store";
        }
    }

    ~beep_boop_persist()
    {}

    template<class K>
    bool get(K &&key, std::string &value) const
    {
        if (in_memory_)
        {
            if (mem_store_.count(key))
            {
                value = mem_store_.at(key);
                return true;
            }
            else
            {
                return false;
            }
        }

        auto resp = cpr::Get(url_ + "/persist/kv" + key, header_);
        if (resp.status_code != 200)
        {
            LOG(WARNING) << "KV GET failure " << resp.status_code << " " << resp.text;
            return false;
        }

        value = resp.text;
        return true;
    }

//    template<class Ks>
//    std::string mget(Ks &&keys...) const
//    {
//        auto resp = cpr::get(url_+"/persist/kv"+key, header_);
//        return resp.text;
//    }

    template<class K, class V>
    bool set(K &&key, V &&value)
    {
        if (key.empty()) return false;

        if (in_memory_)
        {
            mem_store_[key] = value;
            return true;
        }

        auto my_headers = header_;
        my_headers["Content-Type"] = "application/json";
        auto resp = cpr::Put(url_ + "/persist/kv" + key, my_headers, cpr::Body{value});
        if (resp.status_code != 200)
        {
            LOG(WARNING) << "KV PUT failure " << resp.status_code << " " << resp.text;
            return false;
        }

        return true;
    };

    template<class K>
    bool erase(K &&key)
    {
        if (key.empty()) return "";

        if (in_memory_)
        {
            mem_store_.erase(key);
            return true;
        }

        auto resp = cpr::Delete(url_ + "/persist/kv" + key, header_);
        if (resp.status_code != 200)
        {
            LOG(WARNING) << "KV DELETE failure " << resp.status_code << " " << resp.text;
            return false;
        }

        return true;
    }

private:
    cpr::Url url_;
    cpr::Header header_;
    bool in_memory_;
    std::map<std::string, std::string> mem_store_;
};

