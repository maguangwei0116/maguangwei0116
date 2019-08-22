/*****************************************************************************
Filename: card.h
Author  : Chuck Li (chuck.li@redteamobile.com)
Date    : 2018-01-26 15:35:46
Description:
*****************************************************************************/
#ifndef __CARD_H__
#define __CARD_H__

#include <stdint.h>

int card_reset(uint8_t *atr, uint8_t *atr_size);
uint16_t card_cmd(uint8_t *apdu, uint16_t apdu_len, uint8_t *rsp, uint16_t *rsp_len);

#endif  // __CARD_H__
