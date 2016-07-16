#include <stdio.h>
#include <stdlib.h>
#include "sysrepo.h"

#define CHECK_RC(RC, LABEL) do {if (SR_ERR_OK != rc) {goto LABEL; } } while(0)

int main(int argc, char **argv) {

   int rc = SR_ERR_OK; 
   sr_conn_ctx_t *conn = NULL;
   sr_session_ctx_t *sess = NULL;
   sr_val_t val =  {0};

   sr_log_stderr(SR_LL_DBG);

   rc = sr_connect("push kea config", SR_CONN_DEFAULT, &conn);
   CHECK_RC(rc, cleanup);

   rc = sr_session_start(conn, SR_DS_STARTUP, SR_SESS_DEFAULT, &sess);
   CHECK_RC(rc, cleanup);
   
   /* socket type */
   val.xpath = "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/socket-type";
   val.type = SR_STRING_T;
   val.data.string_val = "unix";

   rc = sr_set_item(sess, val.xpath, &val, SR_EDIT_DEFAULT);
   CHECK_RC(rc, cleanup);

   /* socket name */
   val.xpath = "/ietf-kea-dhcpv6:server/serv-attributes/control-socket/socket-name";
   val.type = SR_STRING_T;
   val.data.string_val = "/tmp/";

   rc = sr_set_item(sess, val.xpath, &val, SR_EDIT_DEFAULT);
   CHECK_RC(rc, cleanup);

   rc = sr_commit(sess);
   CHECK_RC(rc, cleanup);
   
cleanup:   
   sr_session_stop(sess);
   sr_disconnect(conn);
}
