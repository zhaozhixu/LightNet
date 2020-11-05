/*
 * Copyright (c) 2018-2020 Zhixu Zhao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ln_test_bitset.h"

#define BITS_PER_INT 64

ln_test_bitset *ln_test_bitset_create(int init_num_bits)
{
    ln_test_bitset *bitset;

    bitset = ln_test_alloc(sizeof(ln_test_bitset));
    bitset->num_ints = (init_num_bits + BITS_PER_INT - 1) / BITS_PER_INT;
    bitset->num_bits = bitset->num_ints * BITS_PER_INT;
    bitset->ints = ln_test_alloc(sizeof(uint64_t) * bitset->num_ints);
    memset(bitset->ints, 0, sizeof(uint64_t) * bitset->num_ints);

    return bitset;
}

void ln_test_bitset_free(ln_test_bitset *bitset)
{
    ln_test_free(bitset->ints);
    ln_test_free(bitset);
}

ln_test_bitset *ln_test_bitset_ones(int num_ones)
{
    ln_test_bitset *bitset;
    int index;

    bitset = ln_test_bitset_create(num_ones);
    index = num_ones / BITS_PER_INT;
    memset(bitset->ints, -1, sizeof(uint64_t) * index);
    bitset->ints[index] = ~(~(uint64_t)0 << num_ones % BITS_PER_INT);

    return bitset;
}

void ln_test_bitset_zero(ln_test_bitset *bitset)
{
    memset(bitset->ints, 0, sizeof(uint64_t) * bitset->num_ints);
}

ln_test_bitset *ln_test_bitset_copy(ln_test_bitset *bitset)
{
    ln_test_bitset *bitset_copy;

    bitset_copy = ln_test_alloc(sizeof(ln_test_bitset));
    bitset_copy->num_bits = bitset->num_bits;
    bitset_copy->num_ints = bitset->num_ints;
    bitset_copy->ints = ln_test_alloc(sizeof(uint64_t) * bitset->num_ints);
    memcpy(bitset_copy->ints, bitset->ints, sizeof(uint64_t) * bitset->num_ints);

    return bitset_copy;
}

static int population_u32(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

static int population_u64(uint64_t x) {
    return population_u32(x & 0xFFFFFFFF) +
            population_u32((x & 0xFFFFFFFF00000000) >> 32);
}

int ln_test_bitset_population(const ln_test_bitset *bitset)
{
    int sum = 0;

    for (int i = 0; i < bitset->num_ints; i++) {
        sum += population_u64(bitset->ints[i]);
    }
    return sum;
}

int *ln_test_bitset_create_one_indexes(const ln_test_bitset *bitset, int *count)
{
    int *result;
    uint64_t tmp;
    int index = 0;

    result = ln_test_alloc(sizeof(int) * ln_test_bitset_population(bitset));
    for (int i = 0; i < bitset->num_ints; i++) {
        tmp = bitset->ints[i];
        for (int j = 0; j < BITS_PER_INT; j++) {
            if (tmp & 1) {
                result[index++] = i * BITS_PER_INT + j;
            }
            tmp >>= 1;
        }
    }
    *count = index;

    return result;
}

void ln_test_bitset_expand(ln_test_bitset *bitset, int num_bits)
{
    int origin_num_ints;

    if (num_bits > bitset->num_bits) {
        origin_num_ints = bitset->num_ints;
        bitset->num_ints = (num_bits + BITS_PER_INT - 1) / BITS_PER_INT;
        bitset->num_bits = bitset->num_ints * BITS_PER_INT;
        bitset->ints = ln_test_realloc(bitset->ints,
                                       sizeof(uint64_t) * bitset->num_ints);
        memset(bitset->ints + origin_num_ints, 0,
               sizeof(uint64_t) * (bitset->num_ints - origin_num_ints));
    }
}

void ln_test_bitset_set(ln_test_bitset *bitset, int n)
{
    ln_test_bitset_expand(bitset, n + 1);
    bitset->ints[n / BITS_PER_INT] |= (uint64_t)1 << n % BITS_PER_INT;
}

void ln_test_bitset_clear(ln_test_bitset *bitset, int n)
{
    ln_test_bitset_expand(bitset, n + 1);
    bitset->ints[n / BITS_PER_INT] &= ~((uint64_t)1 << n % BITS_PER_INT);
}

void ln_test_bitset_and_assign(ln_test_bitset *bitset1, ln_test_bitset *bitset2)
{
    ln_test_bitset_expand(bitset1, bitset2->num_bits);
    for (int i = 0; i < bitset1->num_ints; i++)
        bitset1->ints[i] &= bitset2->ints[i];
}

void ln_test_bitset_or_assign(ln_test_bitset *bitset1, ln_test_bitset *bitset2)
{
    ln_test_bitset_expand(bitset1, bitset2->num_bits);
    for (int i = 0; i < bitset1->num_ints; i++)
        bitset1->ints[i] |= bitset2->ints[i];
}

void ln_test_bitset_not_assign(ln_test_bitset *bitset)
{
    for (int i = 0; i < bitset->num_ints; i++)
        bitset->ints[i] = ~bitset->ints[i];
}
