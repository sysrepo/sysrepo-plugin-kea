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

int kea_ctrl_send(const char* json) {
    if (!json || !strlen(json)) {
        printf("Must provide non-empty configuration");
        return SR_ERR_INVAL_ARG;
    }
    printf("STUB: sending configuration: [%s]\n",
           json);
    return SR_ERR_OK;
}

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

/* prints one value retrieved from sysrepo */
static void
print_value(sr_val_t *value)
{
    switch (value->type) {
        case SR_CONTAINER_T:
        case SR_CONTAINER_PRESENCE_T:
        case SR_LIST_T:
            /* do not print */
            break;
        case SR_STRING_T:
            log_fmt("%s = '%s'", value->xpath, value->data.string_val);
            break;
        case SR_BOOL_T:
            log_fmt("%s = %s", value->xpath, value->data.bool_val ? "true" : "false");
            break;
        case SR_UINT8_T:
            log_fmt("%s = %u", value->xpath, value->data.uint8_val);
            break;
        case SR_UINT16_T:
            log_fmt("%s = %u", value->xpath, value->data.uint16_val);
            break;
        case SR_UINT32_T:
            log_fmt("%s = %u", value->xpath, value->data.uint32_val);
            break;
        case SR_IDENTITYREF_T:
            log_fmt("%s = %s", value->xpath, value->data.identityref_val);
            break;
        case SR_ENUM_T:
            log_fmt("%s = %s", value->xpath, value->data.enum_val);
            break;
        default:
            log_fmt("%s (unprintable)", value->xpath);
    }
}

    string
get_pool(sr_session_ctx_t* session, const char *xpath) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    cout << "Retrieving data for pool " << xpath << endl;

    string name = string(xpath) + "/pool-prefix";

    rc = sr_get_item(session, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << "                { \"pool\": \"" << value->data.string_val << "\" }" << endl;
        sr_free_values(value, 1);
    }

    return tmp.str();
}

string
get_pools(sr_session_ctx_t* session, const char *xpath) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* pools;
    size_t pools_cnt = 32;


    string name = string(xpath) + "/pools/*";
    cout << "Retrieving pools for subnet " << xpath << ", xpath=" << name << endl;

    rc = sr_get_items(session, name.c_str(), &pools, &pools_cnt);
    if (rc == SR_ERR_OK) {
        cout << "Retrieved " << pools_cnt << " pools." << endl;

        tmp << "            \"pools\": [ " << endl;

        for (int i = 0; i < pools_cnt; i++) {
            if (i) {
                tmp << ",";
            }
            tmp << get_pool(session, pools[i].xpath);
        }

        tmp << "            ]" << endl;
        sr_free_values(pools, pools_cnt);

    }

    cout << "#### get_pools=" << tmp.str() << endl;

    return tmp.str();
}


string
get_subnet(sr_session_ctx_t* session, const char *xpath) {
    stringstream tmp;
    int rc = SR_ERR_OK;
    sr_val_t* value = NULL;

    cout << "Retrieving data for subnet " << xpath << endl;

    string name = string(xpath) + "/subnet";

    tmp << "        { " << endl;

    rc = sr_get_item(session, name.c_str(), &value);
    if (rc == SR_ERR_OK) {
        tmp << "        \"subnet\": \"" << value->data.string_val << "\"," << endl;

        sr_free_values(value, 1);
    }

    tmp << get_pools(session, xpath);
    tmp << "        }" << endl;

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
                s << "," << endl;
            }
            string subnet_txt = get_subnet(session, subnets[i].xpath);
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
