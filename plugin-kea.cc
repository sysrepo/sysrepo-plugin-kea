/**
 * @file plugin-kea.cc
 * @author Tomek Mrugalski, Marcin Siodelski
 * @brief plugin for sysrepo datastore for ISC Kea
 *
 * @copyright
 * Copyright (C) 2011-2016 Internet Systems Consortium, Inc. ("ISC")
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdio>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>

extern "C" {
#include "sysrepo.h"

    using namespace std;

    const string KEA_CONTROL_SOCKET = "/tmp/kea-control-channel";
    const string KEA_CONTROL_CLIENT = "~/devel/sysrepo-plugin-kea/kea-client/ctrl-client-cli";
    const string CFG_TEMP_FILE = "/tmp/kea-plugin-gen-cfg.json";

/* logging macro for unformatted messages */
#define log_msg(MSG) \
    do { \
        fprintf(stderr, MSG "\n"); \
        syslog(LOG_INFO, MSG); \
    } while(0)

/* logging macro for formatted messages */
#define log_fmt(MSG, ...) \
    do { \
        fprintf(stderr, MSG "\n", __VA_ARGS__); \
        syslog(LOG_INFO, MSG, __VA_ARGS__); \
    } while(0)

    using namespace std;

/// @brief converts sr_type_t to textual form
///
/// @param type type to be converted
/// @return printable representation of the type
std::string srTypeToText(sr_type_t type)
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

/// @brief Converts sysrepo value into a string representation.
///
/// @param value pointer to the sr_var_t object to be represented
/// @param xpath should the xpath information be printed?
/// @param type should the type be printed?
///
/// @return string representing the value
std::string
valueToText(sr_val_t *value, bool xpath = false, bool type = false)
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
            log_fmt("%s (unprintable)", value->xpath);
    }

    return (tmp.str());
}

std::string
tabs(int level) {
    stringstream tmp;
    const std::string indent("    ");
    for (int i=0; i < level; i++) {
        tmp << indent;
    }
    return (tmp.str());
}

string
get_pool(sr_session_ctx_t* session, const char *xpath, int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    string name = string(xpath) + "/pool-prefix";

    rc = sr_get_item(session, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << tabs(indent) << "{ \"pool\": \"" << value->data.string_val << "\" }" << endl;
        sr_free_values(value, 1);
    }

    return tmp.str();
}

string
get_pools(sr_session_ctx_t* session, const char *xpath, int indent) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* pools;
    size_t pools_cnt = 32;

    string name = string(xpath) + "/pools/*";
    cout << "Retrieving pools for subnet " << xpath << ", xpath=" << name << endl;

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
get_subnet(sr_session_ctx_t* session, const char *xpath, int indent) {
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


/* retrieves & prints current turing-machine configuration */
static void
retrieve_current_config(sr_session_ctx_t *session)
{
    sr_val_t *all_values = NULL;
    sr_val_t *values = NULL;
    size_t count = 0;
    size_t all_count = 0;
    int rc = SR_ERR_OK;

    log_msg("current plugin-kea configuration:");

    /* Going through all of the nodes */
    log_msg("control-socket parameters:\n");
    sr_session_refresh(session);
    rc = sr_get_items(session, "/ietf-kea-dhcpv6:server//*", &all_values, &all_count);
    if (SR_ERR_OK != rc) {
        printf("Error by sr_get_items: %s", sr_strerror(rc));
        return;
    }

    std::ostringstream s;

    s << "{ \"Dhcp6\": {";

    rc = sr_get_items(session, "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/*", &values, &count);
    if (rc == SR_ERR_OK) {
        s << "\"control-socket\": {";
        for (size_t i = 0; i < count; ++i) {
            if (std::string(values[i].xpath) == "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/socket-type") {
                s << "\"socket-type\": \"" << values[i].data.string_val << "\",";
            }
            if (std::string(values[i].xpath) == "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/socket-name") {
                s << "\"socket-name\": \"" << values[i].data.string_val << "\"";
            }
        }
        s << "},";
        sr_free_values(values, count);
    }

    sr_val_t* value = NULL;
    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/interfaces-config/interfaces", &value);
    if (rc == SR_ERR_OK) {
        s << "\"interfaces-config\": { " << std::endl;;
        s << "\"interfaces\": [ \"" << std::endl;;
        s << value->data.string_val << std::endl;
        s << "\" ]" << std::endl;
        s <<  "}," << std::endl;;
        sr_free_values(value, 1);

    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/renew-timer", &value);
    if (rc == SR_ERR_OK) {
        s << "\"renew-timer\":" << value->data.uint32_val << "," << std::endl;
        sr_free_values(value, 1);
    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/rebind-timer", &value);
    if (rc == SR_ERR_OK) {
        s << "\"rebind-timer\":" << value->data.uint32_val << "," << std::endl;;
        sr_free_values(value, 1);
    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/preferred-lifetime", &value);
    if (rc == SR_ERR_OK) {
        s << "\"preferred-lifetime\":" << value->data.uint32_val << "," << std::endl;;
        sr_free_values(value, 1);
    }

    rc = sr_get_item(session, "/ietf-kea-dhcpv6:server/serv-attributes/valid-lifetime", &value);
    if (rc == SR_ERR_OK) {
        s << "\"valid-lifetime\":" << value->data.uint32_val << ", " << std::endl;;
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

    std::ofstream fs;
    fs.open(CFG_TEMP_FILE.c_str(), std::ofstream::out);
    fs << s.str();
    fs.close();

    string cmd = KEA_CONTROL_CLIENT + " " + KEA_CONTROL_SOCKET + " " + CFG_TEMP_FILE;

    system (cmd.c_str());

    remove(CFG_TEMP_FILE.c_str());

    std::cout << s.str() << std::endl;

    sr_free_values(all_values, all_count);
}

static int
module_change_cb(sr_session_ctx_t *session, const char *module_name, sr_notif_event_t event,
                 void *private_ctx)
{
    log_msg("plugin-kea configuration has changed");
    retrieve_current_config(session);

    return SR_ERR_OK;
}

int
sr_plugin_init_cb(sr_session_ctx_t *session, void **private_ctx)
{
    sr_subscription_ctx_t *subscription = NULL;
    int rc = SR_ERR_OK;

    //rc = sr_module_change_subscribe(session, "ietf-kea-dhcpv6", module_change_cb, NULL,
    //                              0, SR_SUBSCR_DEFAULT, &subscription);
    rc = sr_subtree_change_subscribe(session, "/ietf-kea-dhcpv6:server/*", module_change_cb, NULL,
                                    0, SR_SUBSCR_DEFAULT, &subscription);
    if (SR_ERR_OK != rc) {
        goto error;
    }

    log_msg("plugin-kea initialized successfully");

    retrieve_current_config(session);

    /* set subscription as our private context */
    *private_ctx = subscription;

    return SR_ERR_OK;

error:
    log_fmt("plugin-kea initialization failed: %s", sr_strerror(rc));
    sr_unsubscribe(session, subscription);
    return rc;
}

void
sr_plugin_cleanup_cb(sr_session_ctx_t *session, sr_subscription_ctx_t *private_ctx)
{
    /* subscription was set as our private context */
    sr_unsubscribe(session, private_ctx);

    log_msg("pluging-kea plugin cleanup finished");
}

}
