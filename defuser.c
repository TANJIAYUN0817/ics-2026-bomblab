#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

//phase_1
int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    uint32_t low  = (uint32_t)challenge;
    uint32_t high  = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t s1  = (high - middle) * 3u + 0xfcf6u;
    uint32_t s0  = (low + 2u * high) ^ 0x3085u;
    uint32_t ans = ((s0 * 5u) + s1) ^ 0x7733u;
    values[0] = ans;
    count = 1;

    return write_answer(out, cap, values, count);
}

//phase_2
int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;    
    values[0] = (uint32_t)((challenge >> 4) & 0xfu);
    for (uint32_t i = 1; i < 6; i++) {
        uint32_t four_bits = (uint32_t)((challenge >> (i + 4)) & 0xfu);
        values[i] = (four_bits + values[i - 1]) ^ (2u * i);
    }
    count = 6;
    return write_answer(out, cap, values, count);
}

//phase_3
int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    uint32_t a = (uint32_t)challenge & 7u;
    uint32_t b = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    values[0] = a;
    switch (a) {
    case 0: values[1] = (2u * b) ^ 0xa53cu; break;                 
    case 1: values[1] = 5u * b + 0xe8abu; break;                   
    case 2: values[1] = 5u * b + 0xfa93u; break;                   
    case 3: values[1] = (b ^ 0xf807u) + (b >> 2); break;           
    case 4: values[1] = 5u * b + 0x372fdu; break;                  
    case 5: values[1] = (4u * b) ^ 0x9ba8u; break;                 
    case 6: values[1] = 5u * b + 0x40970u; break;                  
    case 7: values[1] = (b ^ 0xb154u) + (b >> 1); break;  
    }
    count=2;
    return write_answer(out, cap, values, count);
}

//phase_4
static uint32_t p4_walk(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint32_t mid = b + ((c - b) >> 1);
    if (mid == a) {
        return (mid + 0x70u) ^ d;
    }
    uint32_t salt = mid + d;
    if (a < mid) {
        return (3u * p4_walk(a, b, mid - 1, d)) ^ salt ^ 0xau;
    } else {
        return (3u * p4_walk(a, mid + 1, c, d)) ^ salt ^ 0x2u;
    }
}
int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    uint32_t a = (uint32_t)challenge & 0x7fu;
    uint32_t b = 0;
    uint32_t c = 0x7fu;
    uint32_t d = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = p4_walk(a, b, c, d);
    count = 1;
    return write_answer(out, cap, values, count);
}

//phase_5
struct p5_node {
    uint32_t f0;
    uint32_t f4;
    int next;      
};
static const struct p5_node p5_nodes[12] = {
    { 7257,  2097, -1 },  
    { 19350, 1666,  9 },  
    { 56570,  117, 11 },  
    { 27691, 2925,  1 },  
    { 31477, 1864,  3 },  
    { 55738,  364,  6 },   
    { 33807, 2597,  7 },   
    { 17642,  620,  8 },   
    { 24670, 2378,  4 },   
    { 49711,  883,  0 },   
    { 49635, 1129,  5 },   
    { 52868, 1412, 10 },   
};
static const int p5_heads[8] = { 2, 11, 10, 5, 6, 7, 8, 4 };
static uint32_t p5_traverse(uint64_t challenge, uint32_t *out) {
    uint32_t a = (uint32_t)challenge & 7u;
    int idx = p5_heads[a];
    const struct p5_node *node = &p5_nodes[idx];
    uint32_t eax = ((uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32)) ^ 0x930cu;
    eax += node->f0;
    eax *= 5u;
    *out = node->f4;
    eax ^= node->f4;
    uint32_t steps = ((uint32_t)(challenge >> 3)) & 3u;
    for (uint32_t i = 1; i <= steps; i++) {
        idx = node->next;
        if (idx < 0) break;
        node = &p5_nodes[idx];
        eax += node->f0;
        eax *= 5u;
        *out = node->f4;
        eax ^= node->f4;
    }
    return eax;
}
int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    uint32_t x = 0;
    uint32_t ret = p5_traverse(challenge, &x);
    values[0] = ret;
    values[1] = x;
    count = 2;
    return write_answer(out, cap, values, count);
}

//phase_6
static uint32_t p6_0(uint32_t x) {
    return (x ^ 0x39f9u) + 8u * x;
}
static uint32_t p6_1(uint32_t x) {
    uint32_t r = (x << 5) | (x >> 27);
    return r ^ 0x1dabu;
}
static uint32_t p6_2(uint32_t x) {
    return (4u * x + 0x2fb8u) ^ (7u * x);
}
static uint32_t p6_3(uint32_t x) {
    return ((x >> 7) ^ x) + 0xd93bu;
}
static uint32_t p6_4(uint32_t x) {
    return (x >> 2) | (x << 30);
}
static uint32_t p6_5(uint32_t x) {
    return (x + 0x8427u) ^ (x >> 5);
}
static uint32_t p6_call(uint32_t idx, uint32_t arg) {
    switch (idx) {
        case 0: return p6_0(arg);
        case 1: return p6_0(arg);
        case 2: return p6_2(arg);
        case 3: return p6_3(arg);
        case 4: return p6_1(arg);
        case 5: return p6_0(arg);
        case 6: return p6_5(arg);
        case 7: return p6_4(arg);
        default: return 0;
    }
}
static uint32_t p6_dispatch(uint64_t challenge) {
    uint32_t a = (uint32_t)challenge & 7u;
    uint32_t b = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t i2 = ((uint32_t)challenge >> 3) & 7u;
    uint32_t r1 = p6_call(a, b);
    uint32_t r2 = p6_call(i2, r1 ^ 0xb4b0u);
    return r2 + 0x6294u;
}
int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;
    values[0] = p6_dispatch(challenge);
    count = 1;
    return write_answer(out, cap, values, count);
}

//honor
static uint32_t secret_mix(uint32_t x) {
    uint32_t r = (x << 5) | (x >> 27);
    r += 0x6f35u;
    return r ^ 0x8506u;
}
int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t ch_low = (uint32_t)challenge;
    uint32_t ch_high = (uint32_t)(challenge >> 32);
    uint32_t cx_low = (uint32_t)context;
    uint32_t cx_high = (uint32_t)(context >> 32);
    uint32_t cx_middle = (uint32_t)(context >> 16);

    uint32_t low2 = cx_low & 3u;

    uint32_t t = ch_high ^ ch_low;
    uint32_t u = cx_high ^ cx_low;
    uint32_t r9 = low2 * 0x6f35u ^ cx_middle;

    uint32_t v0 = low2;
    uint32_t v1 = secret_mix(t ^ u) ^ r9;
    uint32_t v2 = v1 ^ ((u + low2 + 0x8506u) + (t >> 5));
    int n = snprintf(out, cap, "%32sOPEN %010u %010u %010u", "", v0, v1, v2);
    return (n < 0 || (size_t)n >= cap) ? 1 : 0;
}
 
