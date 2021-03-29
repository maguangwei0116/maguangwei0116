/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : utils.c
 * Date        : 2021.01.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "utils.h"

/**
 * @brief                 get an u16 type from big-endian u08 array
 * @param[in] bytes       The pointer of bytes
 * @return uint16_t       The u16 number
 * @note
*/
uint16_t utils_u08s_to_u16(const uint8_t* bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

/**
 * @brief                 Convert an u16 type to big-endian u08 array
 * @param[in] word        The u16 number
 * @param[out] bytes      The pointer of bytes
 * @return void
 * @note
 */
void utils_u16_to_u08s(uint16_t word, uint8_t* bytes)
{
    bytes[0] = (uint8_t)(word >> 8);
    bytes[1] = (uint8_t)word;
}

/**
 * @brief                 get an u32 type from big-endian u08 array
 * @param[in] bytes       The pointer of bytes
 * @return uint32_t       The u32 number
 * @note
 */
uint32_t utils_u08s_to_u32(const uint8_t* bytes)
{
    return (((uint32_t)bytes[0] << 24)
        | ((uint32_t)bytes[1] << 16)
        | ((uint32_t)bytes[2] << 8)
        | bytes[3]);
}

/**
 * @brief                 Convert an u32 type to big-endian u08 array
 * @param[in] dword       The u32 number
 * @param[out] bytes      The pointer of bytes
 * @return void
 * @note 
 */
void utils_u32_to_u08s(uint32_t dword, uint8_t* bytes)
{
    bytes[0] = (uint8_t)(dword >> 24);
    bytes[1] = (uint8_t)(dword >> 16);
    bytes[2] = (uint8_t)(dword >> 8);
    bytes[3] = (uint8_t)dword;
}

/**
 * @brief                 Revert the bits sequence in a byte
 * @param[in] byte        The u08 byte
 * @return uint8_t        The result u08 byte
 * @note
 */
uint8_t utils_byte_reverse(uint8_t byte)
{
    uint8_t output;
    uint8_t i;

    output = 0x02;
    for (i = 0; i < 7; i++) {
        output |= (byte & 0x01);
        output <<= 1;
        byte >>= 1;
    }

    output |= (byte & 0x01);
    return output;
}

/**
 * @brief                 Copy a piece of memory data
 * @param[out] dst        The pointer of destination
 * @param[in] src         The pointer of source
 * @param[in] len         The length to copy
 * @return void
 * @note                  Support for overlapping arrays
 */
void utils_mem_copy(uint8_t* dst, const uint8_t* src, uint16_t len)
{
    uint16_t i;

    if (dst > src) {
        while (len != 0) {
            len--;
            dst[len] = src[len];
        }
    } else {
        for (i = 0; i < len; i++) {
            dst[i] = src[i];
        }
    }
}

/**
 * @brief                 Fill a piece of memory
 * @param[out] dst        The pointer of destination
 * @param[in] content     The content to be filled
 * @param[in] len      The length to fill
 * @return void
 * @note
 */
void utils_mem_fill(uint8_t* dst, uint8_t content, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++) {
        dst[i] = content;
    }
}


/**
 * @brief                 Clear a piece of memory to all zero
 * @param[out] dst        The pointer of destination
 * @param[in] len         The length to clear
 * @return void
 * @note                  
 */
void utils_mem_clr(uint8_t* dst, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++) {
        dst[i] = 0x00;
    }
}

/**
 * @brief                 Compare two byte arrays
 * @param[in] src1        The first byte array
 * @param[in] src2        The second byte array
 * @param[in] len         The length to compare
 * @return uint8_t        The compare result <br>
 *   <em> UTILS_CMP_EQU </em>   ----The two arrays are equal.    <br>
 *   <em> UTILS_CMP_BIG </em>   ----The first byte array is bigger than the second.    <br>
 *   <em> UTILS_CMP_SMA </em>   ----The first byte array is smaller than the second.
 * @note                  
 */
uint8_t utils_mem_cmp(const uint8_t* src1, const uint8_t* src2, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++) {
        if (src1[i] > src2[i]) {
            return UTILS_CMP_BIG;
        }

        if (src1[i] < src2[i]) {
            return UTILS_CMP_SMA;
        }
    }

    return UTILS_CMP_EQU;
}

/**
 * @brief                 Search a special content in a buffer
 * @param[in] src         The buffer
 * @param[in] src_len     The buffer length
 * @param[in] target      The special content to be search in buffer
 * @param[in] target_len  The special content length
 * @param[in] offset      The offset of special content was found in buffer
 * @return uint8_t <br>
 *   <em> 0 </em>    ----search success    <br>
 *   <em> -1 </em>   ----target not found.
 * @note
 */
uint8_t utils_mem_search(const uint8_t* src, uint16_t src_len, const uint8_t* target, uint16_t target_len, uint16_t* offset)
{
    uint16_t i;

    for (i = 0; (i + target_len) <= src_len; i++) {
        if (utils_mem_cmp(src + i, target, target_len) == UTILS_CMP_EQU) {
            *offset = i;
            return RT_SUCCESS;
        }
    }

    return (uint8_t)RT_ERROR;
}

/**
 * @brief                 Get the negated value of memory
 * @param[in][out] dst    The pointer of memory to operate
 * @param[in] len         The length of this memory
 * @return void
 * @note                  
 */
void utils_mem_not(uint8_t* dst, uint16_t len)
{
    while (len != 0) {
        len--;
        dst[len] = ~dst[len];
    }
}

/**
 * @brief                 One array bitwise or another array
 * @param[in][out] dst    The pointer of source 1 and destination
 * @param[in] src         The pointer of source 2
 * @param[in] len         The length to operate
 * @return void 
 * @note                  Support for overlapping arrays
 */
void utils_mem_or(uint8_t* dst, const uint8_t* src, uint16_t len)
{
    uint16_t i;

    if (dst > src) {
        while (len != 0) {
            len--;
            dst[len] |= src[len];
        }
    }
    else {
        for (i = 0; i < len; i++) {
            dst[i] |= src[i];
        }
    }
}

/**
 * @brief                 One array bitwise and another array
 * @param[in][out] dst    The pointer of source 1 and destination
 * @param[in] src         The pointer of source 2
 * @param[in] len         The length to operate
 * @return void
 * @note                  Support for overlapping arrays
 */
void utils_mem_and(uint8_t* dst, const uint8_t* src, uint16_t len)
{
    uint16_t i;

    if (dst > src) {
        while (len != 0) {
            len--;
            dst[len] &= src[len];
        }
    }
    else {
        for (i = 0; i < len; i++) {
            dst[i] &= src[i];
        }
    }
}

/**
 * @brief                 One array bitwise XOR another array
 * @param[in][out] dst    The pointer of source 1 and destination
 * @param[in] src         The pointer of source 2
 * @param[in] len         The length to operate
 * @return void
 * @note                  Support for overlapping arrays
 */
void utils_mem_xor(uint8_t* dst, const uint8_t* src, uint16_t len)
{
    uint16_t i;

    if (dst > src) {
        while (len != 0) {
            len--;
            dst[len] ^= src[len];
        }
    }
    else {
        for (i = 0; i < len; i++) {
            dst[i] ^= src[i];
        }
    }
}

/**
 * @brief                 One array bitwise XOR another array, save the result to the 3rd array
 * @param[in][out] dst    The pointer of destination
 * @param[in] src1        The pointer of source 1
 * @param[in] src2        The pointer of source 2
 * @param[in] len         The length to operate
 * @return void
 * @note                  
 */
void utils_mem_xor_ex(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, uint16_t len)
{
    while (len != 0) {
        len--;
        dst[len] = src1[len] ^ src2[len];
    }
}

/**
 * @brief                 Array addition operation
 * @param[in] add         The pointer of addend
 * @param[in] add_len     length of addend
 * @param[in][out] dst    The pointer of summand and result
 * @param[in] dst_len     length of summand and result buffer
 * @return uint8_t <br>
 *   <em> UTILS_MEMADD_NOERROR </em>    ----success    <br>
 *   <em> UTILS_MEMADD_OVERFLOW </em>   ----The result overflowed.
 * @note                  Can not support for overlapping arrays
 */
uint8_t utils_mem_add(const uint8_t* add, uint16_t add_len, uint8_t* dst, uint16_t dst_len)
{
    uint16_t i = add_len;
    uint16_t j = dst_len;
    uint8_t carry = 0;
    uint16_t sum = 0;

    // add length longer than dst, overflow
    if (add_len > dst_len) {
        return UTILS_MEMADD_OVERFLOW;
    }

    for (; i > 0; i--, j--) {
        sum = add[i - 1] + dst[j - 1] + carry;
        dst[j - 1] = (uint8_t)sum;
        carry = (uint8_t)(sum >> 8);
    }

    while ((j > 0) && (carry != 0)) {
        sum = dst[j - 1] + carry;
        dst[j - 1] = (uint8_t)sum;
        carry = (uint8_t)(sum >> 8);
        j--;
    }

    if (sum > 0xFF) {
        // overflow
        return UTILS_MEMADD_OVERFLOW;
    }

    // 加法正常结束
    return UTILS_MEMADD_NOERROR;
}

/**
 * @brief                 Array addition operation extension
 * @param[out] des        The result buffer
 * @param[in] src1        The source 1 buffer
 * @param[in] src2        The source 2 buffer
 * @param[in] len         length of addition
 * @return uint8_t <br>
 *   <em> UTILS_MEMADD_NOERROR </em>    ----success    <br>
 *   <em> UTILS_MEMADD_OVERFLOW </em>   ----The result overflowed.
 * @note
 */
uint8_t utils_mem_add_ex(uint8_t* des, const uint8_t* src1, const uint8_t* src2, uint16_t len)
{
    int32_t i;
    uint8_t carry = 0;
    uint16_t res;

    for (i = len - 1; i >= 0; i--) {
        res = (uint16_t)src1[i] + src2[i] + carry;

        if (utils_get_high_u08(res)) {
            carry = 1;
        } else {
            carry = 0;
        }

        des[i] = utils_get_low_u08(res);
    }

    if (carry == 1) {
        return UTILS_MEMADD_OVERFLOW;
    }

    return UTILS_MEMADD_NOERROR;
}

/**
 * @brief                 Increase a counter in bytes array
 * @param[in][out] buf    The pointer of buffer
 * @param[in] len         length of buffer 
 * @return uint8_t <br>
 *   <em> UTILS_MEMADD_NOERROR </em>    ----success    <br>
 *   <em> UTILS_MEMADD_OVERFLOW </em>   ----The result overflowed.
 * @note                  
 */
uint8_t utils_mem_increase(uint8_t* buf, uint16_t len)
{
    uint8_t carry = 1;
    uint16_t sum = 0;

    while ((len > 0) && (carry != 0)) {
        sum = buf[len - 1] + carry;
        buf[len - 1] = (uint8_t)sum;
        carry = (uint8_t)(sum >> 8);
        len--;
    }

    if (sum > 0xFF) {
        // overflow
        return UTILS_MEMADD_OVERFLOW;
    }

    return UTILS_MEMADD_NOERROR;
}

/**
 * @brief                 Calculate the XOR result of all bytes in the buffer
 * @param[in] src         The input buffer
 * @param[in] len         length of buffer
 * @return uint8_t        The XOR result
 * @note                  
 */
uint8_t utils_mem_calc_xor(const uint8_t* src, uint16_t len)
{
    uint8_t result;

    result = 0x00;
    while (len != 0) {
        len--;
        result ^= src[len];
    }

    return result;
}

/**
 * @brief                 Check whether all members of the specified array have specified values
 * @param[in] buf         The pointer of buffer to check
 * @param[in] content     The specified value
 * @param[in] len         length of buffer
 * @return bool <br>
 *   <em> true </em>    ----success    <br>
 *   <em> false </em>   ----failed.
 * @note                  
 */
rt_bool utils_mem_is_all_byte(const uint8_t* buf, uint8_t content, uint16_t len)
{
    while (len != 0) {
        len--;
        if (buf[len] != content)
        {
            return RT_FALSE;
        }
    }

    return RT_TRUE;
}

/**
 * @brief                 Check whether all members of the specified array is 0x00
 * @param[in] buf         The pointer of buffer to check
 * @param[in] len         length of buffer
 * @return bool <br>
 *   <em> true </em>    ----success    <br>
 *   <em> false </em>   ----failed.
 * @note
 */
rt_bool utils_mem_is_all_zero(const uint8_t* buf, uint16_t len)
{
    while (len != 0) {
        len--;
        if (buf[len] != 0x00)
        {
            return RT_FALSE;
        }
    }

    return RT_TRUE;
}

/**
 * @brief                 Reverse the order of array members
 * @param[in][out] bytes  The pointer of byte array
 * @param[in] len         length of array
 * @return void
 * @note                  
 */
void utils_mem_reverse(uint8_t* bytes, uint16_t len)
{
    uint16_t offset;
    uint8_t temp;
    
    for (offset = 0; offset < (len / 2); offset++) {
        temp = bytes[offset];
        bytes[offset] = bytes[len - offset - 1];
        bytes[len - offset - 1] = temp;
    }
}
