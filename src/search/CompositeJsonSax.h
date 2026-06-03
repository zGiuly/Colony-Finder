#pragma once

#include <nlohmann/json.hpp>
#include <vector>

class CompositeJsonSax : public nlohmann::json_sax<nlohmann::json>
{
public:
    void Add(nlohmann::json_sax<nlohmann::json>* handler)
    {
        if (handler)
        {
            handlers.push_back(handler);
        }
    }

    bool null() override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->null() && ok;
        return ok;
    }

    bool boolean(bool val) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->boolean(val) && ok;
        return ok;
    }

    bool number_integer(number_integer_t val) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->number_integer(val) && ok;
        return ok;
    }

    bool number_unsigned(number_unsigned_t val) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->number_unsigned(val) && ok;
        return ok;
    }

    bool number_float(number_float_t val, const string_t& s) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->number_float(val, s) && ok;
        return ok;
    }

    bool string(string_t& val) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->string(val) && ok;
        return ok;
    }

    bool binary(binary_t& val) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->binary(val) && ok;
        return ok;
    }

    bool start_object(std::size_t elements) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->start_object(elements) && ok;
        return ok;
    }

    bool key(string_t& val) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->key(val) && ok;
        return ok;
    }

    bool end_object() override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->end_object() && ok;
        return ok;
    }

    bool start_array(std::size_t elements) override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->start_array(elements) && ok;
        return ok;
    }

    bool end_array() override
    {
        bool ok = true;
        for (auto* h : handlers) ok = h->end_array() && ok;
        return ok;
    }

    bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::detail::exception& ex) override
    {
        for (auto* h : handlers) h->parse_error(position, last_token, ex);
        return false;
    }

private:
    std::vector<nlohmann::json_sax<nlohmann::json>*> handlers;
};
