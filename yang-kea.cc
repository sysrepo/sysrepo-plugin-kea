// Copyright (C) 2016-2018 Internet Systems Consortium, Inc. ("ISC")
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

const char* SysrepoKea::DEFAULT_MODEL_NAME = "/ietf-kea-dhcpv6:server/";

using namespace std;

string
tabs(int level) {
    stringstream tmp;
    const string indent("    ");
    for (int i=0; i < level; i++) {
        tmp << indent;
    }
    return (tmp.str());
}

SysrepoKea::SysrepoKea(sr_session_ctx_t* session)
    :model_name_(DEFAULT_MODEL_NAME), session_(session) {
}

string
SysrepoKea::srTypeToText(sr_type_t type)
{
    typedef struct {
        sr_type_t type;
        const string name;
    } sr_types_text;

    sr_types_text names[] = {
    /* special types that does not contain any data */
        { SR_UNKNOWN_T, "SR_UNKNOWN_T" },     /**< Element unknown to sysrepo (unsupported element). */
        { SR_LIST_T, "SR_LIST_T" },           /**< List instance. ([RFC 6020 sec 7.8](http://tools.ietf.org/html/rfc6020#section-7.8)) */
        { SR_CONTAINER_T, "SR_CONTAINER_T" }, /**< Non-presence container. ([RFC 6020 sec 7.5](http://tools.ietf.org/html/rfc6020#section-7.5)) */
        { SR_CONTAINER_PRESENCE_T, "SR_CONTAINER_PRESENCE_T" }, /**< Presence container. ([RFC 6020 sec 7.5.1](http://tools.ietf.org/html/rfc6020#section-7.5.1)) */
        { SR_LEAF_EMPTY_T, "SR_LEAF_EMPTY_T" },           /**< A leaf that does not hold any value ([RFC 6020 sec 9.11](http://tools.ietf.org/html/rfc6020#section-9.11)) */
#ifdef SR_UNION_T
        { SR_UNION_T, "SR_UNION_T" },                /**< Choice of member types ([RFC 6020 sec 9.12](http://tools.ietf.org/html/rfc6020#section-9.12)) */
#endif

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

string
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
            tmp << "\"" << value->data.string_val << "\"";
            break;
        case SR_BOOL_T:
            tmp << (value->data.bool_val ? "true" : "false");
            break;
        case SR_UINT8_T:
            // Converted to uint16, because otherwise it will be printed as
            // a single character.
            tmp << static_cast<uint16_t>(value->data.uint8_val);
            break;
        case SR_UINT16_T:
            tmp << static_cast<uint16_t>(value->data.uint16_val);
            break;
        case SR_UINT32_T:
            tmp << static_cast<uint32_t>(value->data.uint16_val);
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
SysrepoKea::getPool(const char *xpath,int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    string name = string(xpath) + "/pool-prefix";

    rc = sr_get_item(session_, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << tabs(indent) << "{ \"pool\": \""
            << value->data.string_val << "\" }" << endl;
        sr_free_values(value, 1);
    }

    return tmp.str();
}

string
SysrepoKea::getPools(const char *xpath, int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* pools;
    size_t pools_cnt = 32;

    string name = string(xpath) + "/pools/*";
    cout << "Retrieving pools for subnet " << xpath << ", xpath="
         << name << endl;

    rc = sr_get_items(session_, name.c_str(), &pools, &pools_cnt);
    if (rc == SR_ERR_OK) {
        cout << "Retrieved " << pools_cnt << " pools." << endl;

        tmp << tabs(indent) << "\"pools\": [ " << endl;

        for (int i = 0; i < pools_cnt; i++) {
            if (i) {
                tmp << tabs(indent) << ",";
            }
            tmp << getPool(pools[i].xpath, indent + 1);
        }

        tmp << tabs(indent) << "]" << endl;
        sr_free_values(pools, pools_cnt);
    }

    return tmp.str();
}


string
SysrepoKea::getSubnet(const char *xpath, int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    cout << "Retrieving data for subnet " << xpath << endl;

    string name = string(xpath) + "/subnet";

    tmp << tabs(indent) << "{" << endl;

    rc = sr_get_item(session_, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << tabs(indent+1) << "\"subnet\": \"" << value->data.string_val
            << "\"," << endl;

        sr_free_values(value, 1);
    }

    tmp << getPools(xpath, indent + 1);
    tmp << tabs(indent) << "}" << endl;

    return tmp.str();
}

string
SysrepoKea::getValue(const string& xpath) {
    int rc;
    sr_val_t* value = NULL;

    string path = model_name_ + xpath;

    rc = sr_get_item(session_, path.c_str(), &value);
    if (rc != SR_ERR_OK) {
        cerr << "sr_get_item() for xpath=" << xpath << " failed" << endl;
        /// @todo: throw here
        return "";
    }

    string v = valueToText(value, false, false);
    sr_free_values(value, 1);
    return (v);
}

string
SysrepoKea::getFormattedValue(const string& xpath,
                              const string& json_name, int indent,
                              bool comma) {
    stringstream tmp;
    tmp << tabs(indent) << "\"" << json_name << "\": " << getValue(xpath);
    if (comma) {
        tmp << ",";
    }
    return (tmp.str());
}

string
SysrepoKea::getSubnets(const string& xpath, int indent) {
    stringstream s;
    sr_val_t* subnets = NULL;
    size_t subnets_cnt = 9999;
    int rc = SR_ERR_OK;
    string path = model_name_ + xpath;
    rc = sr_get_items(session_, path.c_str(), &subnets, &subnets_cnt);
    if (rc == SR_ERR_OK) {
        s << tabs(indent) << "\"subnet6\": [" << endl;
        for (int i = 0; i < subnets_cnt; i++) {

            if (i) {
                s << tabs(indent + 1) << "," << endl;
            }
            string subnet_txt = getSubnet(subnets[i].xpath, indent + 1);
            s << subnet_txt;
        }
        s << tabs(indent) << "]" << endl;

        sr_free_values(subnets, subnets_cnt);
    }

    return (s.str());
}


string
SysrepoKea::getConfig() {

    sr_val_t *all_values = NULL;
    sr_val_t *values = NULL;
    size_t count = 0;
    size_t all_count = 0;
    int rc = SR_ERR_OK;

    /* Going through all of the nodes */
    sr_session_refresh(session_);
    rc = sr_get_items(session_, "/ietf-kea-dhcpv6:server//*", &all_values, &all_count);
    if (SR_ERR_OK != rc) {
        cerr << "Error by sr_get_items: %s" << sr_strerror(rc);
        return ("");
    }
    sr_free_values(all_values, all_count);

    ostringstream s;

    s << "{" << endl << "\"Dhcp6\": {" << endl;

    // Control socket parameters
    s << tabs(1) << "\"control-socket\": {" << endl;
    s << getFormattedValue("serv-attributes/control-socket/socket-type", "socket-type", 2, true) << endl;
    s << getFormattedValue("serv-attributes/control-socket/socket-name", "socket-name", 2, false) << endl;
    s << tabs(1) << "}," << endl;

    /// @todo: There may be multiple interfaces specified, need to iterate through all of them, not just pick first.
    sr_val_t* value = NULL;
    rc = sr_get_item(session_, "/ietf-kea-dhcpv6:server/serv-attributes/interfaces-config/interfaces", &value);
    if (rc == SR_ERR_OK) {
        s << tabs(1) << "\"interfaces-config\": { " << endl;;
        s << tabs(2) << "\"interfaces\": [ \"" << endl;;
        s << tabs(2) << value->data.string_val << endl;
        s << tabs(2) << "\" ]" << endl;
        s << tabs(1) << "}," << endl;;
        sr_free_values(value, 1);

    }

    // Lease database
    /// @todo: Lease database does not seem to be configurable using YANG model.

    // Generate all subnets
    string subnets = getSubnets("network-ranges/subnet6", 1);

    // Timers
    s << getFormattedValue("serv-attributes/renew-timer", "renew-timer", 1, true) << endl;
    s << getFormattedValue("serv-attributes/rebind-timer", "rebind-timer", 1, true) << endl;
    s << getFormattedValue("serv-attributes/preferred-lifetime", "preferred-lifetime", 1, true) << endl;
    s << getFormattedValue("serv-attributes/valid-lifetime", "valid-lifetime", 1, !subnets.empty()) << endl;

    s << subnets << endl;

    s << "}" << endl << "}" << endl;

    return (s.str());
}
