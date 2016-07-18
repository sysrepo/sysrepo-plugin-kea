/// @file plugin-kea.cc
/// @author Tomek Mrugalski, Marcin Siodelski
/// @brief plugin for sysrepo datastore for ISC Kea
///
///  @copyright
///  Copyright (C) 2016 Internet Systems Consortium, Inc. ("ISC")
///
///  This Source Code Form is subject to the terms of the Mozilla Public
///  License, v. 2.0. If a copy of the MPL was not distributed with this
///  file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstdio>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "yang-kea.h"

extern "C" {
#include "sysrepo.h"

using namespace std;

const string KEA_CONTROL_SOCKET = "/tmp/kea-control-channel";
const string KEA_CONTROL_CLIENT = "~/devel/sysrepo-plugin-kea/kea-client/ctrl-client-cli";
const string CFG_TEMP_FILE = "/tmp/kea-plugin-gen-cfg.json";

/* retrieves & prints current turing-machine configuration */
static void
retrieve_current_config(sr_session_ctx_t *session)
{
    SysrepoKea interface;

    string json = interface.getConfig(session);

    std::ofstream fs;
    fs.open(CFG_TEMP_FILE.c_str(), std::ofstream::out);
    fs << json;
    fs.close();

    string cmd = KEA_CONTROL_CLIENT + " " + KEA_CONTROL_SOCKET + " " + CFG_TEMP_FILE;

    system (cmd.c_str());

    remove(CFG_TEMP_FILE.c_str());

    std::cout << json << std::endl;

}

static int
module_change_cb(sr_session_ctx_t *session, const char *module_name, sr_notif_event_t event,
                 void *private_ctx)
{
    cerr << "plugin-kea configuration has changed" << endl;
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

    cerr << "plugin-kea initialized successfully" << endl;

    retrieve_current_config(session);

    /* set subscription as our private context */
    *private_ctx = subscription;

    return SR_ERR_OK;

error:
    cerr << "plugin-kea initialization failed: " << sr_strerror(rc) << endl;
    sr_unsubscribe(session, subscription);
    return rc;
}

void
sr_plugin_cleanup_cb(sr_session_ctx_t *session, sr_subscription_ctx_t *private_ctx)
{
    /* subscription was set as our private context */
    sr_unsubscribe(session, private_ctx);

    cout << "pluging-kea plugin cleanup finished" << endl;
}

}
