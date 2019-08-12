/*****************************************************************************
Filename: mt2503.h
Author  : Chuck Li (chuck.li@redteamobile.com)
Date    : 2018-04-08 11:42:30
Description:
*****************************************************************************/
#ifndef __MT2503_H__
#define __MT2503_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t l4c_source_id_enum;
typedef uint8_t kal_uint8;
typedef uint16_t kal_uint16;
typedef uint32_t kal_uint32;
typedef kal_uint16 sim_status;

#define kal_bool                            bool
#define KAL_FALSE                           false
#define KAL_TRUE                            true
#define usim_protocol_enum                  sim_protocol_phy_enum
#define usim_speed_enum                     sim_speed_enum
#define usim_clock_stop_enum                sim_clock_stop_enum
#define usim_power_enum                     sim_power_enum

#define DRV_SIM_MAX_LOGICAL_INTERFACE       2
#define RMMI_SRC                            5
#define USIM_DMA_MAX_SIZE                   260

typedef enum
{
    SIM_PROTOCOL,
    USIM_PROTOCOL
}sim_protocol_app_enum;

typedef enum{
    USIM_DIRECT,
    USIM_INVERSE
}usim_dir_enum;

typedef enum
{
    T0_PROTOCOL,
    T1_PROTOCOL,
    UNKNOWN_PROTOCOL
}sim_protocol_phy_enum;

typedef enum{
    UNKNOWN_POWER_CLASS = 0,
    CLASS_A_50V = 1,
    CLASS_B_30V = 2,
    CLASS_AB    = 3,
    CLASS_C_18V = 4,
    ClASS_BC    = 6,
    CLASS_ABC   = 7,
    CLASS_ALLSUPPORT = 0xff
}sim_power_enum;

typedef enum{
    CLOCK_STOP_NOT_SUPPORT  = 0x0,
    CLOCK_STOP_LOW          = 0x40,
    CLOCK_STOP_HIGH         = 0x80,
    CLOCK_STOP_ANY          = 0xc0,
    CLOCK_STOP_MSK          = 0xc0,
    CLOCK_STOP_UNKONW       = 0x0f
}sim_clock_stop_enum;

typedef enum{
    SPEED_372,
    SPEED_64,
    SPEED_32,
    SPEED_16
}sim_speed_enum;

typedef enum{
    USIM_NO_ERROR       = 0,

    // expected status
    USIM_WAITING_EVENT  = 1,     // initial wait event status
    USIM_BLOCK_REC      = 2,     // successfully received a complete block
    USIM_POWER_OFF      = 3,     // successfully powered off
    USIM_ATR_REC        = 4,           // successfully reveived all ATR
    USIM_S_BLOCK_REC    = 5,   // successfully reveived S RESP

    // error status
    USIM_NO_INSERT      = -1,
    USIM_VOLT_NOT_SUPPORT = -2,
    USIM_NO_ATR         = -3,
    USIM_TS_INVALID     = -4,
    USIM_ATR_ERR        = -5,
    USIM_INVALID_ATR    = -6,
    USIM_PTS_FAIL       = -7,
    USIM_RX_INVALID     = -8,   // EDC error or parity error
    USIM_BWT_TIMEOUT    = -9,
    USIM_DATA_ABORT     = -10,
    USIM_DEACTIVATED    = -11,
    USIM_S_BLOCK_FAIL   = -12,
    USIM_INVALID_WRST   = -13,
    USIM_GPT_TIMEOUT    = -14
}usim_status_enum;

typedef enum
{
    ME_UNKNOW =0,
    ME_18V_30V,
    ME_30V_ONLY,
    ME_18V_ONLY
} sim_env;

typedef enum
{
    sim_card_normal_speed = 0,
    sim_card_enhance_speed_64,
    sim_card_enhance_speed_32,
    sim_card_enhance_speed_16,
    sim_card_enhance_speed_8
} sim_card_speed_type;

typedef enum{
    usim_case_1 = 1,
    usim_case_2,
    usim_case_3,
    usim_case_4
}usim_cmd_case_enum;

typedef enum{
    USIM_RESET_NEGOTIABLE,  // type 1
    USIM_RESET_SPECIFIC     // type 2
}usim_reset_type_enum;

typedef enum{
    IDLE_STATE,
    ACTIVATION_STATE,
    ATR_STATE,
    PTS_STATE,
    MAIN_CMD_READY_STATE,
    CMD_TX_STATE,
    //CMD_RX_HEADER_STATE,
    CMD_RX_BLOCK_REC_STATE,
    //CMD_RX_S_BLOCK_STATE,
    CMD_RX_STATE,
    CLK_STOPPING_STATE,
    CLK_STOPPED_STATE,
    DEACTIVATION_STATE
}usim_main_state_enum;

typedef enum{
    EVENT_TX =      0x1,
    EVENT_RX =      0x2,
    EVENT_OV =      0x4,
    EVENT_TOUT =    0x8,
    EVENT_TXERR =   0x10,
    EVENT_NATR =    0x20,
    EVENT_OFF =     0x40,
    EVENT_T0END =   0x80,
    EVENT_RXERR =   0x100,
    EVENT_T1END =   0x200,
    EVENT_EDCERR = 0x400
}usim_event_type_enum;

typedef enum{
    USIM_CMD_READY,
    I_BLOCK_RX,
    I_BLOCK_TX,
    I_BLOCK_M0_RX,
    I_BLOCK_M0_TX,
    I_BLOCK_M1_RX,
    I_BLOCK_M1_TX,
    R_BLOCK_RX,
    R_BLOCK_TX,
    S_BlOCK_REQ_RX,
    S_BlOCK_REQ_TX,
    S_BlOCK_RESP_RX,
    S_BlOCK_RESP_TX
}usim_cmd_state_enum;

typedef enum {
    DMA_BYTE=0,
    DMA_SHORT,
    DMA_LONG
} DMA_TranSize;

typedef enum {
    DMA_HWTX,          /*use DMA_HWMENU*/  /*from RAM to register, 4~11*/
    DMA_HWRX,          /*use DMA_HWMENU*/ /*from register to RAM, 4~11*/
    DMA_SWCOPY,        /*use DMA_SWCOPYMENU*/  /*from RAM to RAM, 1~3*/
    DMA_HWTX_RINGBUFF, /*use DMA_HWRINGBUFF_MENU*/   /*from RAM to register, 4~11*/
    DMA_HWRX_RINGBUFF  /*use DMA_HWRINGBUFF_MENU*/  /*from register to RAM, 4~11*/
} DMA_Type;

typedef struct{
    sim_power_enum power;
    sim_speed_enum speed;
    sim_clock_stop_enum clock_stop;
    sim_protocol_app_enum app_proto;
    sim_protocol_phy_enum phy_proto;
    kal_bool T0_support;    // if T0 is supported
    kal_bool T1_support;    // if T1 is supported
    kal_uint8 hist_index; // index to the historical char of ATR
    kal_uint8 *ATR;
    /*following information is necessary for SIM task for UICC identification*/
    kal_bool TAiExist; //if the first TA for T=15 is existed
}sim_info_struct;

typedef void (*DCL_SIM_PLUG_OUT_CALLBACK)(kal_uint32 simIf);
typedef void (*DCL_SIM_PLUG_IN_CALLBACK)(kal_uint32 simIf);

typedef struct{
    kal_uint32  head;
    /*
        Here defines MTK related HW information of this logical interface, these values are defined as constant in old driver.
        Now we make it variable.
    */
    kal_uint32  mtk_baseAddr;
    kal_uint32  mtk_pdnAddr;
    kal_uint32  mtk_pdnBit;
    kal_uint32  mtk_pdnDevice;
    kal_uint32  mtk_dmaMaster;
    kal_uint32  mtk_lisrCode;
    /*
        in multiple SIM drivers, simInterface is used in all most all functions, we need record this information.
    */
    kal_uint32  simInterface; /*The logical number. This value now can be 0~n, not limted as 0~1 before. We can assume it less than 2 now*/
    kal_uint32  MT6302ChipNo;   /*record which MT6302 switch used for this card*/
    kal_uint32  MT6302PortNo;       /*record which port of MT6302 is used for this card*/
    void            *MT6302PeerInterfaceCb;/*MT6302 need peer's information, so we have to maintain a way to find its peer*/
    kal_uint32  simSwitchChipNo;
    kal_uint32  simSwitchPortNo;
    void        *simSwitchPeerInterfaceCb;
    void        *simSwitchPeerInterfaceCb1;
    void        *simSwitchPeerInterfaceCb2;
    DCL_SIM_PLUG_IN_CALLBACK simHotPlugIn;
    DCL_SIM_PLUG_OUT_CALLBACK simHotPlugOut;
    kal_uint32 debounceTime; /* hot swap EINT debounce time */
    kal_uint32  tail;
    kal_bool polarity; /* hot swap EINT poarity */
    kal_bool    IsCardRemove;
    kal_uint8 smHandler;
}sim_HW_cb;

typedef usim_status_enum  (*SIM_API_RESET) (sim_power_enum ExpectVolt, sim_power_enum *ResultVolt, kal_bool warm, sim_HW_cb *hw_cb);
typedef sim_status (*SIM_API_CMD) (kal_uint8  *txData,kal_uint32  *txSize,kal_uint8  *rxData, kal_uint32  *rxSize, sim_HW_cb *hw_cb);
typedef void (*SIM_API_PWROFF) (sim_HW_cb *hw_cb);
typedef void (*SIM_API_CARDINFO) (sim_info_struct *info, sim_HW_cb *hw_cb);
typedef void (*SIM_API_ENHANCED_SPEED) (kal_bool enable, sim_HW_cb *hw_cb);
typedef void (*SIM_API_ENHANCED_SELECT_PHY) (sim_protocol_phy_enum T, sim_HW_cb *hw_cb);
typedef kal_bool (*SIM_API_SET_CLKSTOP) (sim_clock_stop_enum mode, sim_HW_cb *hw_cb);
typedef void (*SIM_API_EOC) (sim_HW_cb *hw_cb);
typedef void (*SIM_API_MSG)(kal_uint32 tag, kal_uint32 event, kal_uint32 data1, kal_uint32 data2);
typedef void (*SIM_API_TOUT_TEST)(kal_uint32 toutValue, sim_HW_cb *hw_cb);

typedef struct
{
    SIM_API_RESET reset;
    SIM_API_CMD command;
    SIM_API_PWROFF powerOff;
    SIM_API_CARDINFO getCardInfo;
    SIM_API_ENHANCED_SPEED enableEnhancedSpeed;
    SIM_API_ENHANCED_SELECT_PHY selectPreferPhyLayer;
    SIM_API_SET_CLKSTOP setClockStopMode;
    SIM_API_EOC EOC;/*use this to hook necessary action before return to SIM task, this is called by adaption layer, not SIM task*/
    SIM_API_MSG addMessage;
    SIM_API_TOUT_TEST toutTest;
}sim_ctrlDriver;

typedef struct
{
    kal_uint8    State;
    kal_uint8    Data_format;      /*SIM_direct,SIM_indirect*/
    kal_uint8    Power;            /*SIM_3V,SIM_5V*/
    kal_uint8    recData[40];     /*PTS or ATR data*/
    kal_bool     recDataErr;
    kal_uint16   recDataLen;       /* for command, ATR process   */
    kal_uint8    result;           /* for ATR, command, RST   */
    kal_uint32   EvtFlag;
    sim_env      SIM_ENV;
#ifndef SIM_ADDDMA
    kal_uint8    *txbuffer;        /* only used for no DMA */
    kal_uint16   txsize;           /* only used for no DMA */
    kal_uint16   txindex;          /* only used for no DMA */
    kal_uint8    *rxbuffer;        /* only used for no DMA */
#ifdef   NoT0CTRL
    kal_uint16   recsize;
    kal_uint8    INS;
    kal_uint8    SW1;
    kal_uint8    SW2;
#endif  /*NoT0CTRL*/
#endif   /*SIM_ADDDMA*/
/*add for clock stop mode*/
    kal_uint8    cmdState;         /* only used for no T0CTRL, and for clock stop */
    kal_uint8    Speed;            /*Speed372,Speed64,Speed32*/
    kal_bool     clkStop;          /*Clok Stop Enable*/
    kal_bool     clkStopLevel;     /*Clok Stop level*/
    kal_bool     reject_set_event;
    kal_bool     event_state;
    kal_uint8    initialPower;
    sim_card_speed_type  sim_card_speed;
    kal_hisrid        hisr;             /*SIM HISR*/
    kal_eventgrpid    event;      /*SIM Event*/

    sim_protocol_app_enum app_proto;
    kal_bool    timeout;
    usim_cmd_case_enum cmd_case;
    kal_bool    is_err; // sim command has error once.
    kal_bool    get9000WhenSelect;
}Sim_Card;

typedef struct
{
    kal_bool  burst_mode; /*burst mode = 0 ==> single mode*/
    kal_uint8 cycle;      /*active only when (burst_mode == TRUE)*/
}DMA_TMODE;              /*only active when hw management*/

typedef struct
{
    DMA_Type       type;
    DMA_TranSize   size;
    kal_uint32     count;
#if defined(__DMA_API_V2__)
    kal_bool     fixed_pattern;
#endif
    void           *menu;      /*DMA_HWMENU,DMA_HWRINGBUFF_MENU,DMA_SWCOPYMENU,or DMA_SWCOPY_RINGBUFF_MENU*/
    void           (*callback)(void);      /*if callback == NULL, interrupt disable*/
}DMA_INPUT;

#define DMA_Master          uint8_t
typedef struct
{
    DMA_TMODE      TMOD;
    DMA_Master     master;
    kal_uint32     addr;
}DMA_HWMENU;

typedef struct{
    // ATR info
    kal_uint8 ATR_data[36];     // used to store all ATR data string
    kal_uint8 ATR_index;            // index to the ATR_data
    kal_uint8 header_tx[4], header_tx_bak[4]; // header_tx_bak used to backup the previous command
    kal_uint8 header_rx[4];
    kal_uint8 dma_buffer[USIM_DMA_MAX_SIZE];

    // informations
    usim_dir_enum dir;  // convention of character frame
    sim_env sim_env;        // the voltage which MS can supply
    usim_power_enum power_in;       // expected power class input form application layer
    usim_power_enum power;          // power class used
    usim_power_enum power_class;        // supported power class indicated at ATR
    // usim_protocol_enum T;
    usim_speed_enum speed;      // speed selected
    usim_speed_enum card_speed; // TA1, max speed card can support
    kal_bool high_speed_en;     // control if high speed is enalbed
    usim_clock_stop_enum clock_stop_type;
    kal_bool    clock_stop_en;      // clock_stop is enabled or not
    kal_uint16 etu_of_1860;
    kal_uint16 etu_of_700;
    usim_reset_type_enum reset_mode; // specific or negotiable mode
    kal_bool warm_rst;  // KAL_TRUE: it's a warm reset, KAL_FALSE: a cold reset
    kal_bool T0_support;    // if T0 is supported
    kal_bool T1_support;    // if T1 is supported
    kal_uint16 Fi;
    kal_uint8 Di;

    // state control
    volatile usim_main_state_enum main_state;
    volatile usim_status_enum ev_status;
    kal_uint8 retry;

    // time out control
    kal_uint32 WWT; // work waiting time (T0)
    kal_uint32 CWT; // character waiting time in etu(T1)
    kal_uint32 BWT; // blcok waiting time in etu(T1)
    kal_uint32 timeout; // etu

    // T=1
    kal_uint8 ns;       // sequence # of sending
    kal_uint8 nr;       // sequence # of receiving
    kal_uint8 ifsd; // information size of interface device
    kal_uint8 ifsc; // information size of card
    usim_cmd_state_enum cmd_state;
    usim_cmd_state_enum cmd_state_bak;
    kal_bool abort;
    kal_bool wtx;   // waiting time extension
    kal_bool resync;
    kal_bool send_prev; // send the previous block
    kal_bool tx_chain;
    kal_bool rx_chain;
    kal_uint16 tx_size;
    kal_uint16 rx_size;
    kal_uint16 tx_index;
    kal_uint16 rx_index;
    kal_uint8 *tx_buf;
    kal_uint8 *rx_buf;
    kal_uint8 sw[2];        // used to contain SW1 and SW2
    kal_uint8 wtx_m;        // multiplier of BWT

    // others
    kal_bool ts_hsk_en; // enable handshake at TS byte (error signal and char repetition)
    kal_uint8 dma_port;
    DMA_INPUT dma_input;
    DMA_HWMENU dma_menu;
    kal_eventgrpid event;
    kal_uint32 ev_flag;
    kal_hisrid hisr;
    kal_uint32 int_status;
    sim_protocol_app_enum app_proto;    // application protocol (USIM, SIM)
    sim_protocol_phy_enum phy_proto; // protocol type selected (physical layer)
    kal_uint8 hist_index;       // index to the historical characters
    usim_status_enum status;
    usim_cmd_case_enum cmd_case;
    kal_uint8 gpt_handle;
    kal_bool present;
    sim_protocol_phy_enum perfer_phy_proto; // protocol type selected (physical layer)
#if defined(USIM_DEBUG)
    kal_int32 sline[INDEX_COUNT];   // set event at the which line in usim_drv.c
    kal_uint32 sindex;  // index to the sline[4]
    kal_int32 wline[INDEX_COUNT];   // wait event at the which line in usim_drv.c
    kal_uint32 windex;  // index to the wline
#endif

    /*SIM task need following information for UICC identification*/
    kal_bool TAiExist; //if the first TA for T=15 is existed
}usim_dcb_struct;

typedef struct driver_table {
    sim_ctrlDriver esim;
    sim_ctrlDriver *pmtk;
    kal_bool use_esim;
} sim_drv_t;

extern void l4c_nw_cfun_state_req(l4c_source_id_enum src, uint8_t state);
extern sim_ctrlDriver *sim_driverTable[DRV_SIM_MAX_LOGICAL_INTERFACE];

#endif  // __MT2503_H__
