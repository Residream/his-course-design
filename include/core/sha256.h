/*
 * sha256加密模块
 */
#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* SHA256 输出长度 */
#define SHA256_BLOCK_SIZE 32

/* SHA256 上下文结构 */
typedef struct
{
    uint8_t data[64];  // 当前处理的512bit数据块
    uint32_t datalen;  // 当前数据长度
    uint64_t bitlen;   // 总数据长度(bit)
    uint32_t state[8]; // 8个哈希寄存器
} SHA256_CTX;

/* 计算SHA256哈希 */
void sha256(const uint8_t *data, size_t len, uint8_t hash[32]);

/* 将SHA256哈希转为16进制字符串 */
void sha256_to_hex(const uint8_t hash[32], char output[65]);

#endif