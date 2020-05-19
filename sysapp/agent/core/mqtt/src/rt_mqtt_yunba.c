
#include "rt_type.h"
#include "rt_mqtt.h"

#define YUNBA_APPKEY                    "596882bf2a9275716fe3c1e2"  // APPKET of YUNBA server
#define YUNBA_SERVER_IP                 "23.91.101.68"              // the IP address of yunba server
#define YUNBA_URL_PORT                  "1883"                      // yunba mqtt port
#define YUNBA_URL                       "tcp://"YUNBA_SERVER_IP":"YUNBA_URL_PORT
#define YUNBA_SERVER_PORT               8085                        // the port of yunba server
#define YUNBA_SERVER_PORT2              8383
#define YUNBA_AUTHKEY_URL               "abj-redismsg-4.yunba.io"
#define YUNBA_AUTHKEY_PORT              8060

static int32_t retstatus;
static char auth_key[80];

static int32_t get_ret_status(const char *json_data)
{
    return -1;
}

static int32_t get_authkey_status(const char *json_data)
{
    return -1;
}

/* callback for YUNBA ticket server */
static int32_t mqtt_yunba_ticket_server_cb(const char *json_data) 
{
    return RT_ERROR;
}

/* get YUNBA MQTT connect param API */
int32_t MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_opts_t *opts)
{
    return RT_ERROR;
}

int MQTTClient_set_authkey(char *cid, char *appkey, char* authkey, int *ret_status)
{
    return RT_ERROR;
}

int MQTTClient_get_authkey(char *cid, char *appkey, char* authkey, int *ret_status)
{
    return RT_ERROR;
}

//connect yunba server
rt_bool mqtt_connect_yunba(mqtt_param_t *param, const char *ticket_server)
{
    return RT_FALSE;
}

