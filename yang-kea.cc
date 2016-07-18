// Copyright (C) 2016 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
/// @file yang-kea.cc
///
/// This file contains primary interface for operations
/// needed for Kea YANG interface. It is specifiec to
/// Sysrepo implementation.

#include "yang-kea.h"

#include <sstream>
#include <iostream>

using namespace std;

std::string
tabs(int level) {
    stringstream tmp;
    const std::string indent("    ");
    for (int i=0; i < level; i++) {
        tmp << indent;
    }
    return (tmp.str());
}

std::string
SysrepoKea::srTypeToText(sr_type_t type)
{
    typedef struct {
        sr_type_t type;
        const std::string name;
    } sr_types_text;

    sr_types_text names[] = {
    /* special types that does not contain any data */
        { SR_UNKNOWN_T, "SR_UNKNOWN_T" },     /**< Element unknown to sysrepo (unsupported element). */
        { SR_LIST_T, "SR_LIST_T" },           /**< List instance. ([RFC 6020 sec 7.8](http://tools.ietf.org/html/rfc6020#section-7.8)) */
        { SR_CONTAINER_T, "SR_CONTAINER_T" }, /**< Non-presence container. ([RFC 6020 sec 7.5](http://tools.ietf.org/html/rfc6020#section-7.5)) */
        { SR_CONTAINER_PRESENCE_T, "SR_CONTAINER_PRESENCE_T" }, /**< Presence container. ([RFC 6020 sec 7.5.1](http://tools.ietf.org/html/rfc6020#section-7.5.1)) */
        { SR_LEAF_EMPTY_T, "SR_LEAF_EMPTY_T" },           /**< A leaf that does not hold any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
        { SR_UNION_T, "SR_UNION_T" },                /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */

    /* types containing some data */
        { SR_BINARY_T, "SR_BINARY_T" },   /**< Base64-encoded binary data ([RFC 6020 sec 9.8](http://tools.ietf.org/html/rfc6020#section-9.8)) */
        { SR_BITS_T, "SR_BITS_T" },        /**< A set of bits or flags ([RFC 6020 sec 9.7](http://tools.ietf.org/html/rfc6020#section-9.7)) */
        { SR_BOOL_T, "SR_BOOL_T" },         /**< A boolean value ([RFC 6020 sec 9.5](http://tools.ietf.org/html/rfc6020#section-9.5)) */
        { SR_DECIMAL64_T, "SR_DECIMAL64_T" },    /**< 64-bit signed decimal number ([RFC 6020 sec 9.3](http://tools.ietf.org/html/rfc6020#section-9.3)) */
        { SR_ENUM_T, "SR_ENUM_T" },         /**< A string from enumerated strings list ([RFC 6020 sec 9.6](http://tools.ietf.org/html/rfc6020#section-9.6)) */
        { SR_IDENTITYREF_T, "SR_IDENTITYREF_T" },  /**< A reference to an abstract identity ([RFC 6020 sec 9.10](http://tools.ietf.org/html/rfc6020#section-9.10)) */
        { SR_INSTANCEID_T, "SR_INSTANCEID_T" },  /**< References a data tree node ([RFC 6020 sec 9.13](http://tools.ietf.org/html/rfc6020#section-9.13)) */
        { SR_INT8_T, "SR_INT8_T" },         /**< 8-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_INT16_T, "SR_INT16_T" },        /**< 16-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_INT32_T, "SR_INT32_T" },       /**< 32-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_INT64_T, "SR_INT64_T" },        /**< 64-bit signed integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_STRING_T, "SR_STRING_T" },       /**< Human-readable string ([RFC 6020 sec 9.4](http://tools.ietf.org/html/rfc6020#section-9.4)) */
        { SR_UINT8_T, "SR_UINT8_T" },        /**< 8-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_UINT16_T, "SR_UINT16_T" },       /**< 16-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_UINT32_T, "SR_UINT32_T" },       /**< 32-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
        { SR_UINT64_T, "SR_UINT64_T" }       /**< 64-bit unsigned integer ([RFC 6020 sec 9.2](http://tools.ietf.org/html/rfc6020#section-9.2)) */
    };

    for (int i = 0; i < sizeof(names)/sizeof(names[0]); i++) {
        if (names[i].type == type) {
            return (names[i].name);
        }
    }

    return "unknown";
}

std::string
SysrepoKea::valueToText(sr_val_t *value, bool xpath, bool type)
{
    stringstream tmp;

    if (xpath) {
        tmp << value->xpath;
    }
    if (type) {
        tmp << ",type=" << srTypeToText(value->type) << ":";
    }

    if (xpath || type) {
        tmp <<  ":";
    }
    switch (value->type) {
        case SR_CONTAINER_T:
        case SR_CONTAINER_PRESENCE_T:
        case SR_LIST_T:
            /* do not print */
            break;
        case SR_STRING_T:
            tmp << value->data.string_val;
            break;
        case SR_BOOL_T:
            tmp << value->data.bool_val ? "true" : "false";
            break;
        case SR_UINT8_T:
            // Converted to uint16, because otherwise it will be printed as
            // a single character.
            tmp << static_cast<uint16_t>(value->xpath, value->data.uint8_val);
            break;
        case SR_UINT16_T:
            tmp << static_cast<uint16_t>(value->xpath, value->data.uint16_val);
            break;
        case SR_UINT32_T:
            tmp << static_cast<uint32_t>(value->xpath, value->data.uint16_val);
            break;
        case SR_IDENTITYREF_T:
            tmp << value->data.identityref_val;
            break;
        case SR_ENUM_T:
            tmp << value->data.enum_val;
            break;
        default:
            if (!xpath) {
                // We need to print it only if xpath was not printed
                // already.
                tmp << "xpath=" << value->xpath;
            }
            tmp << "(unprintable)";
    }

    return (tmp.str());
}

string
SysrepoKea::get_pool(sr_session_ctx_t* session, const char *xpath,int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    string name = string(xpath) + "/pool-prefix";

    rc = sr_get_item(session, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << tabs(indent) << "{ \"pool\": \""
            << value->data.string_val << "\" }" << endl;
        sr_free_values(value, 1);
    }

    return tmp.str();
}

string
SysrepoKea::get_pools(sr_session_ctx_t* session, const char *xpath, int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* pools;
    size_t pools_cnt = 32;

    string name = string(xpath) + "/pools/*";
    cout << "Retrieving pools for subnet " << xpath << ", xpath="
         << name << endl;

    rc = sr_get_items(session, name.c_str(), &pools, &pools_cnt);
    if (rc == SR_ERR_OK) {
        cout << "Retrieved " << pools_cnt << " pools." << endl;

        tmp << tabs(indent) << "\"pools\": [ " << endl;

        for (int i = 0; i < pools_cnt; i++) {
            if (i) {
                tmp << tabs(indent) << ",";
            }
            tmp << get_pool(session, pools[i].xpath, indent + 1);
        }

        tmp << tabs(indent) << "]" << endl;
        sr_free_values(pools, pools_cnt);
    }

    return tmp.str();
}


string
SysrepoKea::get_subnet(sr_session_ctx_t* session, const char *xpath, int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    cout << "Retrieving data for subnet " << xpath << endl;

    string name = string(xpath) + "/subnet";

    tmp << tabs(indent) << "{" << endl;

    rc = sr_get_item(session, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << tabs(indent+1) << "\"subnet\": \"" << value->data.string_val << "\"," << endl;

        sr_free_values(value, 1);
    }

    tmp << get_pools(session, xpath, indent + 1);
    tmp << tabs(indent) << "}" << endl;

    return tmp.str();
}

std::string
SysrepoKea::getConfig(sr_session_ctx_t * session) {

    sr_val_t *all_values = NULL;
    sr_val_t *values = NULL;
    size_t count = 0;
    size_t all_count = 0;
    int rc = SR_ERR_OK;


    /* Going through all of the nodes */
    sr_session_refresh(session);
    rc = sr_get_items(session, "/ietf-kea-dhcpv6:server//*", &all_values, &all_count);
    if (SR_ERR_OK != rc) {
        cerr << "Error by sr_get_items: %s" << sr_strerror(rc);
        return ("");
    }

    std::ostringstream s;

    s << "{" << endl << "\"Dhcp6\": {" << endl;

    rc = sr_get_items(session, "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/*", &values, &count);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"control-socket\": {" << endl;
        for (size_t i = 0; i < count; ++i) {
            if (std::string(values[i].xpath) == "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/socket-type") {
                s << tabs(2) << "\"socket-type\": \"" << values[i].data.string_val << "\"," << endl;
            }
            if (std::string(values[i].xpath) == "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/socket-name") {
                s << tabs(2) << "\"socket-name\": \"" << values[i].data.string_val << "\"" << endl;
            }
        }
        s << tabs(1) << "}," << endl;
        sr_free_values(values, count);
    }

    /// @todo: There may be multiple interfaces specified, need to iterate through all of them, not just pick first.
    sr_val_t* value = NULL;
    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/interfaces-config/interfaces", &value);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"interfaces-config\": { " << std::endl;;
        s << tabs(2) << "\"interfaces\": [ \"" << std::endl;;
        s << tabs(2) << value->data.string_val << std::endl;
        s << tabs(2) << "\" ]" << std::endl;
        s << tabs(1) << "}," << std::endl;;
        sr_free_values(value, 1);

    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/renew-timer", &value);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"renew-timer\":" << value->data.uint32_val << "," << std::endl;
        sr_free_values(value, 1);
    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/rebind-timer", &value);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"rebind-timer\":" << value->data.uint32_val << "," << std::endl;;
        sr_free_values(value, 1);
    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/preferred-lifetime", &value);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"preferred-lifetime\":" << value->data.uint32_val << "," << std::endl;;
        sr_free_values(value, 1);
    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/valid-lifetime", &value);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"valid-lifetime\":" << value->data.uint32_val << ", " << std::endl;;
        sr_free_values(value, 1);
    }

    sr_val_t* subnets;
    size_t subnets_cnt = 32;
    rc = sr_get_items(session, "/ietf-kea-dhcpv6:server/network-ranges/subnet6",
                      &subnets, &subnets_cnt);
    if (rc == SR_ERR_OK) {
        cout << "#### Received " << subnets_cnt << " subnet[s]" << endl;
        s << "    \"subnet6\": [" << endl;
        for (int i = 0; i < subnets_cnt; i++) {

            if (i) {
                s << tabs(2) << "," << endl;
            }
            string subnet_txt = get_subnet(session, subnets[i].xpath, 2);
            s << subnet_txt;
        }
        s << "    ]" << endl;

        sr_free_values(subnets, subnets_cnt);
    }

    s << "}" << std::endl << "}" << std::endl;

    sr_free_values(all_values, all_count);

    return (s.str());
}
