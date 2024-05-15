#include "sha1.h"
#include <fstream>
#include <iomanip>
#include <sstream>

#define SHA1_ROL(value, bits) (((value) << (bits)) | (((value) & 0xffffffff) >> (32 - (bits))))
#define SHA1_BLK(i)                                                                                                    \
    (block[i & 15] = SHA1_ROL(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i & 15], 1))

#define SHA1_CONST1 0x5a827999
#define SHA1_CONST2 0x6ed9eba1
#define SHA1_CONST3 0x8f1bbcdc
#define SHA1_CONST4 0xca62c1d6

#define SHA1_R0(v, w, x, y, z, i)                                                                                      \
    z += ((w & (x ^ y)) ^ y) + block[i] + SHA1_CONST1 + SHA1_ROL(v, 5);                                                \
    w = SHA1_ROL(w, 30);
#define SHA1_R1(v, w, x, y, z, i)                                                                                      \
    z += ((w & (x ^ y)) ^ y) + SHA1_BLK(i) + SHA1_CONST1 + SHA1_ROL(v, 5);                                             \
    w = SHA1_ROL(w, 30);
#define SHA1_R2(v, w, x, y, z, i)                                                                                      \
    z += (w ^ x ^ y) + SHA1_BLK(i) + SHA1_CONST2 + SHA1_ROL(v, 5);                                                     \
    w = SHA1_ROL(w, 30);
#define SHA1_R3(v, w, x, y, z, i)                                                                                      \
    z += (((w | x) & y) | (w & x)) + SHA1_BLK(i) + SHA1_CONST3 + SHA1_ROL(v, 5);                                       \
    w = SHA1_ROL(w, 30);
#define SHA1_R4(v, w, x, y, z, i)                                                                                      \
    z += (w ^ x ^ y) + SHA1_BLK(i) + SHA1_CONST4 + SHA1_ROL(v, 5);                                                     \
    w = SHA1_ROL(w, 30);

SHA1::SHA1()
{
    reset();
}

void SHA1::update(const std::string &s)
{
    std::istringstream is(s);
    update(is);
}

void SHA1::update(std::istream &is)
{
    std::string rest_of_buffer;
    read(is, rest_of_buffer, BLOCK_BYTES - buffer.size());
    buffer += rest_of_buffer;

    while (is)
    {
        uint32 block[BLOCK_INTS];
        buffer_to_block(buffer, block);
        transform(block);
        read(is, buffer, BLOCK_BYTES);
    }
}

std::string SHA1::final()
{
    uint64 total_bits = (transforms * BLOCK_BYTES + buffer.size()) * 8;

    buffer += 0x80;
    unsigned int orig_size = buffer.size();
    while (buffer.size() < BLOCK_BYTES)
    {
        buffer += (char)0x00;
    }

    uint32 block[BLOCK_INTS];
    buffer_to_block(buffer, block);

    if (orig_size > BLOCK_BYTES - 8)
    {
        transform(block);
        for (unsigned int i = 0; i < BLOCK_INTS - 2; i++)
        {
            block[i] = 0;
        }
    }

    block[BLOCK_INTS - 1] = total_bits;
    block[BLOCK_INTS - 2] = (total_bits >> 32);
    transform(block);

    std::ostringstream result;
    for (unsigned int i = 0; i < DIGEST_INTS; i++)
    {
        result << std::hex << std::setfill('0') << std::setw(8);
        result << (digest[i] & 0xffffffff);
    }

    reset();

    return result.str();
}

std::string SHA1::from_file(const std::string &filename)
{
    std::ifstream stream(filename.c_str(), std::ios::binary);
    SHA1 checksum;
    checksum.update(stream);
    return checksum.final();
}

void SHA1::reset()
{
    digest[0] = 0x67452301;
    digest[1] = 0xefcdab89;
    digest[2] = 0x98badcfe;
    digest[3] = 0x10325476;
    digest[4] = 0xc3d2e1f0;

    transforms = 0;
    buffer = "";
}

void SHA1::transform(uint32 block[BLOCK_BYTES])
{
    uint32 a = digest[0];
    uint32 b = digest[1];
    uint32 c = digest[2];
    uint32 d = digest[3];
    uint32 e = digest[4];

    for (int i = 0; i <= 15; ++i)
    {
        SHA1_R0(a, b, c, d, e, i);
        std::swap(a, b);
        std::swap(a, c);
        std::swap(a, d);
        std::swap(a, e);
    }
    for (int i = 16; i <= 19; ++i)
    {
        SHA1_R1(a, b, c, d, e, i);
        std::swap(a, b);
        std::swap(a, c);
        std::swap(a, d);
        std::swap(a, e);
    }
    for (int i = 20; i <= 39; ++i)
    {
        SHA1_R2(a, b, c, d, e, i);
        std::swap(a, b);
        std::swap(a, c);
        std::swap(a, d);
        std::swap(a, e);
    }
    for (int i = 40; i <= 59; ++i)
    {
        SHA1_R3(a, b, c, d, e, i);
        std::swap(a, b);
        std::swap(a, c);
        std::swap(a, d);
        std::swap(a, e);
    }
    for (int i = 60; i <= 79; ++i)
    {
        SHA1_R4(a, b, c, d, e, i);
        std::swap(a, b);
        std::swap(a, c);
        std::swap(a, d);
        std::swap(a, e);
    }

    digest[0] += a;
    digest[1] += b;
    digest[2] += c;
    digest[3] += d;
    digest[4] += e;

    transforms++;
}

void SHA1::buffer_to_block(const std::string &buffer, uint32 block[BLOCK_BYTES])
{
    for (unsigned int i = 0; i < BLOCK_INTS; i++)
    {
        block[i] = (buffer[4 * i + 3] & 0xff) | (buffer[4 * i + 2] & 0xff) << 8 | (buffer[4 * i + 1] & 0xff) << 16 |
                   (buffer[4 * i + 0] & 0xff) << 24;
    }
}

void SHA1::read(std::istream &is, std::string &s, int max)
{
    char sbuf[max];
    is.read(sbuf, max);
    s.assign(sbuf, is.gcount());
}

std::string sha1(const std::string &string)
{
    SHA1 checksum;
    checksum.update(string);
    return checksum.final();
}
