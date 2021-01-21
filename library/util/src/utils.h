/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : utils.h
 * Date        : 2021.01.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cpluplus
extern "C" {
#endif

#include "rtm_typedef.h"

/**
 * definition of compare result
 */
#define UTILS_CMP_EQU       0x00
#define UTILS_CMP_BIG       0x01
#define UTILS_CMP_SMA       0x02

/**
 * definition of memory add result
 */
#define UTILS_MEMADD_NOERROR    0x00
#define UTILS_MEMADD_OVERFLOW   0x01

/**
 * @brief                 get an uint16_t type from big-endian uint8_t array
 * @param[in] bytes       The pointer of bytes
 * @return uint16_t       The uint16_t number
 * @note
*/
uint16_t utils_u08s_to_u16(const uint8_t* bytes);

/**
 * @brief                 Convert an uint16_t type to big-endian uint8_t array
 * @param[in] word        The uint16_t number
 * @param[out] bytes      The pointer of bytes
 * @return void
 * @note
 */
void utils_u16_to_u08s(uint16_t word, uint8_t* bytes);

/**
 * @brief                 get an u32 type from big-endian uint8_t array
 * @param[in] bytes       The pointer of bytes
 * @return uint32_t       The u32 number
 * @note
 */
uint32_t utils_u08s_to_u32(const uint8_t* bytes);

/**
 * @brief                 Convert an u32 type to big-endian uint8_t array
 * @param[in] dword       The u32 number
 * @param[out] bytes      The pointer of bytes
 * @return void
 * @note 
 */
void utils_u32_to_u08s(uint32_t dword, uint8_t* bytes);

/**
 * @brief                 Make a uint8_t number from two nibble bytes
 * @param[in] byte1       The high nibble
 * @param[in] byte2       The low nibble
 * @return uint16_t       The result uint8_t number
 * @note                  The outside should ensure that the input nibble is not greater than 0x0F, <br>
                          otherwise the function does not guarantee the validity of the result
 */
#define utils_make_u08(high_nibble, low_nibble)    ((uint8_t)(((uint8_t)(high_nibble) << 4) | (uint8_t)(low_nibble)))

/**
 * @brief                 Make a uint16_t number from two uint8_t bytes
 * @param[in] byte1       The 1st uint8_t byte
 * @param[in] byte2       The 2nd uint8_t byte
 * @return uint16_t       The result uint16_t number
 * @note
 */
#define utils_make_u16(byte1, byte2)    ((uint16_t)((((uint16_t)((uint8_t)(byte1) << 8)) | ((uint8_t)(byte2)))))

/**
 * @brief                 Make a u32 number from four uint8_t bytes
 * @param[in] byte1       The 1st uint8_t byte
 * @param[in] byte2       The 2nd uint8_t byte
 * @param[in] byte3       The 3rd uint8_t byte
 * @param[in] byte4       The 4th uint8_t byte
 * @return uint32_t       The result u32 number
 * @note
 */
#define utils_make_u32(byte1, byte2, byte3, byte4)    ((uint32_t)(((uint8_t)(byte1) << 24) | ((uint8_t)(byte2) << 16) | ((uint8_t)(byte3) << 8) | (uint8_t)(byte4)))

/**
 * @brief                 Make a u32 number from two uint16_t word
 * @param[in] word1       The high uint16_t word
 * @param[in] word2       The low uint16_t word
 * @return uint32_t       The result u32 number
 * @note
 */
#define utils_make_u32_by_u16(word1, word2)     ((uint32_t)(((uint32_t)(word1) << 16) | (uint16_t)(word2)))

/**
 * @brief                 Get high byte from a uint32_t type
 * @param[in] word        The uint32_t value
 * @return uint8_t        The high u16
 * @note
 */
#define utils_get_high_u16(dword)       ((uint16_t)((uint32_t)(dword) >> 16))

 /**
  * @brief                 Get low byte from a uint32_t type
  * @param[in] word        The uint32_t value
  * @return uint8_t        The low u16
  * @note
  */
#define utils_get_low_u16(dword)        ((uint16_t)(dword))

/**
 * @brief                 Get high byte from a uint16_t type
 * @param[in] word        The uint16_t value
 * @return uint8_t        The high byte
 * @note                  
 */
#define utils_get_high_u08(word)       ((uint8_t)((uint16_t)(word) >> 8))

 /**
  * @brief                 Get low byte from a uint16_t type
  * @param[in] word        The uint16_t value
  * @return uint8_t        The low byte
  * @note
  */
#define utils_get_low_u08(word)        ((uint8_t)(word))

 /**
  * @brief                 Get high nibble from a byte
  * @param[in] word        The byte nibble
  * @return uint8_t        The high nibble
  * @note
  */
#define utils_high_nibble(byte)         ((uint8_t)(((uint8_t)(byte) >> 4) & 0x0F))

 /**
  * @brief                 Get low nibble from a byte
  * @param[in] word        The byte nibble
  * @return uint8_t        The low nibble
  * @note
  */
#define utils_low_nibble(byte)          ((uint8_t)((uint8_t)(byte) & 0x0F))

/**
 * @brief                 Revert the bits sequence in a byte
 * @param[in] byte        The uint8_t byte
 * @return uint8_t        The result uint8_t byte
 * @note
 */
uint8_t utils_byte_reverse(uint8_t byte);

/**
 * @brief                 Copy a piece of memory data
 * @param[out] dst        The pointer of destination
 * @param[in] src         The pointer of source
 * @param[in] len         The length to copy
 * @return void
 * @note                  Support for overlapping arrays
 */
void utils_mem_copy(uint8_t* dst, const uint8_t* src, uint16_t len);

/**
 * @brief                 Fill a piece of memory
 * @param[out] dst        The pointer of destination
 * @param[in] content     The content to be filled
 * @param[in] len         The length to fill
 * @return void
 * @note
 */
void utils_mem_fill(uint8_t* dst, uint8_t content, uint16_t len);


/**
 * @brief                 Clear a piece of memory to all zero
 * @param[out] dst        The pointer of destination
 * @param[in] len         The length to clear
 * @return void
 * @note                  
 */
void utils_mem_clr(uint8_t* dst, uint16_t len);

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
uint8_t utils_mem_cmp(const uint8_t* src1, const uint8_t* src2, uint16_t len);

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
uint8_t utils_mem_search(const uint8_t* src, uint16_t src_len, const uint8_t* target, uint16_t target_len, uint16_t* offset);

/**
 * @brief                 Get the negated value of memory
 * @param[in][out] dst    The pointer of memory to operate
 * @param[in] len         The length of this memory
 * @return void
 * @note                  
 */
void utils_mem_not(uint8_t* dst, uint16_t len);

/**
 * @brief                 One array bitwise or another array
 * @param[in][out] dst    The pointer of source 1 and destination
 * @param[in] src         The pointer of source 2
 * @param[in] len         The length to operate
 * @return void 
 * @note                  Support for overlapping arrays
 */
void utils_mem_or(uint8_t* dst, const uint8_t* src, uint16_t len);

/**
 * @brief                 One array bitwise and another array
 * @param[in][out] dst    The pointer of source 1 and destination
 * @param[in] src         The pointer of source 2
 * @param[in] len         The length to operate
 * @return void
 * @note                  Support for overlapping arrays
 */
void utils_mem_and(uint8_t* dst, const uint8_t* src, uint16_t len);

/**
 * @brief                 One array bitwise XOR another array
 * @param[in][out] dst    The pointer of source 1 and destination
 * @param[in] src         The pointer of source 2
 * @param[in] len         The length to operate
 * @return void
 * @note                  Support for overlapping arrays
 */
void utils_mem_xor(uint8_t* dst, const uint8_t* src, uint16_t len);

/**
 * @brief                 One array bitwise XOR another array, save the result to the 3rd array
 * @param[in][out] dst    The pointer of destination
 * @param[in] src1        The pointer of source 1
 * @param[in] src2        The pointer of source 2
 * @param[in] len         The length to operate
 * @return void
 * @note                  
 */
void utils_mem_xor_ex(uint8_t* dst, const uint8_t* src1, const uint8_t* src2, uint16_t len);

/**
 * @brief                 Array addition operation
 * @param[in] add         The pointer of addend
 * @param[in] add_len     Length of addend
 * @param[in][out] dst    The pointer of summand and result
 * @param[in] dst_len     Length of summand and result buffer
 * @return uint8_t <br>
 *   <em> UTILS_MEMADD_NOERROR </em>    ----success    <br>
 *   <em> UTILS_MEMADD_OVERFLOW </em>   ----The result overflowed.
 * @note                  
 */
uint8_t utils_mem_add(const uint8_t* add, uint16_t add_len, uint8_t* dst, uint16_t dst_len);


/**
 * @brief                 Array addition operation extension
 * @param[out] Des        The result buffer
 * @param[in] Src1        The source 1 buffer
 * @param[in] Src2        The source 2 buffer
 * @param[in] len         Length of addition
 * @return uint8_t <br>
 *   <em> UTILS_MEMADD_NOERROR </em>    ----success    <br>
 *   <em> UTILS_MEMADD_OVERFLOW </em>   ----The result overflowed.
 * @note                  
 */
uint8_t utils_mem_add_ex(uint8_t* Des, const uint8_t* Src1, const uint8_t* Src2, uint16_t len);

/**
 * @brief                 Increase a counter in bytes array
 * @param[in][out] buf    The pointer of buffer
 * @param[in] len         length of buffer 
 * @return uint8_t <br>
 *   <em> UTILS_MEMADD_NOERROR </em>    ----success    <br>
 *   <em> UTILS_MEMADD_OVERFLOW </em>   ----The result overflowed.
 * @note                  
 */
uint8_t utils_mem_increase(uint8_t* buf, uint16_t len);

/**
 * @brief                 Calculate the XOR result of all bytes in the buffer
 * @param[in] src         The input buffer
 * @param[in] len         length of buffer
 * @return uint8_t        The XOR result
 * @note                  
 */
uint8_t utils_mem_calc_xor(const uint8_t* src, uint16_t len);

/**
 * @brief                 Check whether all members of the specified array have specified values
 * @param[in] buf         The pointer of buffer to check
 * @param[in] content     The specified value
 * @param[in] len         Length of buffer
 * @return bool_t <br>
 *   <em> true </em>    ----success    <br>
 *   <em> false </em>   ----failed.
 * @note                  
 */
bool_t utils_mem_is_all_byte(const uint8_t* buf, uint8_t content, uint16_t len);

/**
 * @brief                 Check whether all members of the specified array is 0x00
 * @param[in] buf         The pointer of buffer to check
 * @param[in] len         Length of buffer
 * @return bool_t <br>
 *   <em> true </em>    ----success    <br>
 *   <em> false </em>   ----failed.
 * @note
 */
bool_t utils_mem_is_all_zero(const uint8_t* buf, uint16_t len);

/**
 * @brief                 Reverse the order of array members
 * @param[in][out] bytes  The pointer of byte array
 * @param[in] len         length of array
 * @return void
 * @note                  
 */
void utils_mem_reverse(uint8_t* bytes, uint16_t len);

/**
 * @brief                 Nibble reverses all array members
 * @param[in][out] bytes  The pointer of byte array
 * @param[in] len         length of array
 * @return void
 * @note                  
 */
void utils_mem_bcd_swap(uint8_t* bytes, uint16_t len);

/**
 * @brief                 Shift the array to the left
 * @param[in][out] bytes  The byte array
 * @param[in] len         The length of array
 * @param[in] shift       Number of bits to be shift
 * @return void
 * @note
 */
void utils_mem_shift_left(uint8_t* bytes, uint16_t len, uint8_t shift);

#ifdef __cpluplus
}
#endif

#endif // __UTILS_H__
