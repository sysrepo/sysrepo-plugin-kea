#include <iostream>
#include "yang-kea.h"
#include "sysrepo.h"

using namespace std;

int main(int argc, const char *argv[]) {

    int rc = SR_ERR_OK;
    sr_conn_ctx_t *conn = NULL;
    sr_session_ctx_t *sess = NULL;

    rc = sr_connect("pull kea config", SR_CONN_DEFAULT, &conn);
    if (rc != SR_ERR_OK) {
        cerr << "Failed to create session" << endl;
        return (EXIT_FAILURE);
    }

    rc = sr_session_start(conn, SR_DS_STARTUP, SR_SESS_DEFAULT, &sess);
    if (rc != SR_ERR_OK) {
        cerr << "Failed to start session" << endl;
        return (EXIT_FAILURE);
    }

    SysrepoKea yang(sess);

    std::string json = yang.getConfig();

    cout << "Received JSON config is " << json.length() << " bytes long."
         << endl;
    cout << json;

    int status = EXIT_SUCCESS;
    rc = sr_session_stop(sess);
    if (rc != SR_ERR_OK) {
        cerr << "Failed to stop session" << endl;
        status = EXIT_FAILURE;
    }

    sr_disconnect(conn);

    return (status);
}
