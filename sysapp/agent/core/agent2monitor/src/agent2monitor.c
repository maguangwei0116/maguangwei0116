
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "agent_main.h"
#include "agent2monitor.h"

int32_t agent_set_monitor_param(config_info_t *config_info)
{
    info_vuicc_data_t info = {0};
    atom_data_t c_data = {0};
    uint8_t buf[256] = {0};
    uint16_t len = sizeof(info_vuicc_data_t);

    MSG_PRINTF(LOG_INFO, "atom len:%d\n", sizeof(atom_data_t));

    info.vuicc_switch = config_info->lpa_channel_type;
    info.log_level = config_info->monitor_log_level;
    info.log_size = config_info->log_max_size;

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = 0x00;
    c_data.data_len = (uint8_t)len;
    c_data.data = (uint8_t *)&info;
    len = sizeof(atom_data_t) - 4;
    rt_os_memcpy(&buf[0], &c_data, len);
    rt_os_memcpy(&buf[len], c_data.data, c_data.data_len);
    len += c_data.data_len;

    MSG_PRINTF(LOG_INFO, "len:%d, log_max_size:%d, %08x\n", len, info.log_size, info.log_level);

    return ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &len);
}

int32_t agent_get_monitor_version(char *name, int n_size, char *version, int v_size, char *chip_modle, int c_size)
{
    monitor_version_t m_version = {0};
    atom_data_t c_data = {0};
    uint8_t buf[sizeof(monitor_version_t)] = {0};
    uint16_t len = sizeof(monitor_version_t);

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = CMD_GET_MONITOR_VER;
    c_data.data_len = (uint8_t)len;
    c_data.data = (uint8_t *)&m_version;
    len = sizeof(atom_data_t) - 4;
    rt_os_memcpy(&buf[0], &c_data, len);
    rt_os_memcpy(&buf[len], c_data.data, c_data.data_len);
    len += c_data.data_len;

    ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &len); 

    rt_os_memcpy((uint8_t *)&m_version, (uint8_t *)buf, sizeof(monitor_version_t));
    //MSG_HEXDUMP("version", &m_version, sizeof(monitor_version_t));

    snprintf(name, n_size, "%s", m_version.name);    
    snprintf(version, v_size, "%s", m_version.version);    
    snprintf(chip_modle, c_size, "%s", m_version.chip_model);

    return 0;
}