// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2012 Cloudmeter, Inc.  (http://www.cloudmeter.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <pion/algorithm.hpp>
#include <boost/assert.hpp>

// macro to shift bitmask by a single bit
#define SHIFT_BITMASK(ptr, mask)    if (mask & 0x01) { mask = 0x80; ++ptr; } else mask >>= 1;


namespace pion {        // begin namespace pion


bool algorithm::base64_decode(const std::string &input, std::string &output)
{
    static const char nop = -1; 
    static const char decoding_data[] = {
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop, 62, nop,nop,nop, 63,
        52, 53, 54,  55,  56, 57, 58, 59,  60, 61,nop,nop, nop,nop,nop,nop,
        nop, 0,  1,   2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
        15, 16, 17,  18,  19, 20, 21, 22,  23, 24, 25,nop, nop,nop,nop,nop,
        nop,26, 27,  28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
        41, 42, 43,  44,  45, 46, 47, 48,  49, 50, 51,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop,
        nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop, nop,nop,nop,nop
        };

    unsigned int input_length=input.size();
    const char * input_ptr = input.data();

    // allocate space for output string
    output.clear();
    output.reserve(((input_length+2)/3)*4);

    // for each 4-bytes sequence from the input, extract 4 6-bits sequences by droping first two bits
    // and regenerate into 3 8-bits sequence

    for (unsigned int i=0; i<input_length;i++) {
        char base64code0;
        char base64code1;
        char base64code2 = 0;   // initialized to 0 to suppress warnings
        char base64code3;

        base64code0 = decoding_data[static_cast<int>(input_ptr[i])];
        if(base64code0==nop)            // non base64 character
            return false;
        if(!(++i<input_length)) // we need at least two input bytes for first byte output
            return false;
        base64code1 = decoding_data[static_cast<int>(input_ptr[i])];
        if(base64code1==nop)            // non base64 character
            return false;

        output += ((base64code0 << 2) | ((base64code1 >> 4) & 0x3));

        if(++i<input_length) {
            char c = input_ptr[i];
            if(c =='=') { // padding , end of input
                BOOST_ASSERT( (base64code1 & 0x0f)==0);
                return true;
            }
            base64code2 = decoding_data[static_cast<int>(input_ptr[i])];
            if(base64code2==nop)            // non base64 character
                return false;

            output += ((base64code1 << 4) & 0xf0) | ((base64code2 >> 2) & 0x0f);
        }

        if(++i<input_length) {
            char c = input_ptr[i];
            if(c =='=') { // padding , end of input
                BOOST_ASSERT( (base64code2 & 0x03)==0);
                return true;
            }
            base64code3 = decoding_data[static_cast<int>(input_ptr[i])];
            if(base64code3==nop)            // non base64 character
                return false;

            output += (((base64code2 << 6) & 0xc0) | base64code3 );
        }

    }

    return true;
}

bool algorithm::base64_encode(const std::string &input, std::string &output)
{
    static const char encoding_data[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned int input_length=input.size();
    const char * input_ptr = input.data();

    // allocate space for output string
    output.clear();
    output.reserve(((input_length+2)/3)*4);

    // for each 3-bytes sequence from the input, extract 4 6-bits sequences and encode using 
    // encoding_data lookup table.
    // if input do not contains enough chars to complete 3-byte sequence,use pad char '=' 
    for (unsigned int i=0; i<input_length;i++) {
        int base64code0=0;
        int base64code1=0;
        int base64code2=0;
        int base64code3=0;

        base64code0 = (input_ptr[i] >> 2)  & 0x3f;  // 1-byte 6 bits
        output += encoding_data[base64code0];
        base64code1 = (input_ptr[i] << 4 ) & 0x3f;  // 1-byte 2 bits +

        if (++i < input_length) {
            base64code1 |= (input_ptr[i] >> 4) & 0x0f; // 2-byte 4 bits
            output += encoding_data[base64code1];
            base64code2 = (input_ptr[i] << 2) & 0x3f;  // 2-byte 4 bits + 

            if (++i < input_length) {
                base64code2 |= (input_ptr[i] >> 6) & 0x03; // 3-byte 2 bits
                base64code3  = input_ptr[i] & 0x3f;       // 3-byte 6 bits
                output += encoding_data[base64code2];
                output += encoding_data[base64code3];
            } else {
                output += encoding_data[base64code2];
                output += '=';
            }
        } else {
            output += encoding_data[base64code1];
            output += '=';
            output += '=';
        }
    }

    return true;
}

std::string algorithm::url_decode(const std::string& str)
{
    char decode_buf[3];
    std::string result;
    result.reserve(str.size());
    
    for (std::string::size_type pos = 0; pos < str.size(); ++pos) {
        switch(str[pos]) {
        case '+':
            // convert to space character
            result += ' ';
            break;
        case '%':
            // decode hexidecimal value
            if (pos + 2 < str.size()) {
                decode_buf[0] = str[++pos];
                decode_buf[1] = str[++pos];
                decode_buf[2] = '\0';
                result += static_cast<char>( strtol(decode_buf, 0, 16) );
            } else {
                // recover from error by not decoding character
                result += '%';
            }
            break;
        default:
            // character does not need to be escaped
            result += str[pos];
        }
    };
    
    return result;
}
    
std::string algorithm::url_encode(const std::string& str)
{
    char encode_buf[4];
    std::string result;
    encode_buf[0] = '%';
    result.reserve(str.size());

    // character selection for this algorithm is based on the following url:
    // http://www.blooberry.com/indexdot/html/topics/urlencoding.htm
    
    for (std::string::size_type pos = 0; pos < str.size(); ++pos) {
        switch(str[pos]) {
        default:
            if (str[pos] > 32 && str[pos] < 127) {
                // character does not need to be escaped
                result += str[pos];
                break;
            }
            // else pass through to next case
        case ' ':   
        case '$': case '&': case '+': case ',': case '/': case ':':
        case ';': case '=': case '?': case '@': case '"': case '<':
        case '>': case '#': case '%': case '{': case '}': case '|':
        case '\\': case '^': case '~': case '[': case ']': case '`':
            // the character needs to be encoded
            sprintf(encode_buf+1, "%.2X", (unsigned char)(str[pos]));
            result += encode_buf;
            break;
        }
    };
    
    return result;
}
    
void algorithm::float_from_bytes(long double& value, const unsigned char *ptr, size_t num_exp_bits, size_t num_fraction_bits)
{
    // get sign of the number from the first bit
    const int value_sign = (*ptr & 0x80) ? -1 : 1;
    
    // build exponent value from bitstream
    unsigned char mask = 0x80;
    boost::int16_t exponent = 0;
    for (size_t n = 0; n < num_exp_bits; ++n) {
        SHIFT_BITMASK(ptr, mask);
        exponent *= 2;
        if (*ptr & mask)
            exponent += 1;
    }
    
    // build significand from bitstream
    long double significand = exponent ? 1.0 : 0.0;
    long double significand_value = 1.0;
    while (num_fraction_bits) {
        SHIFT_BITMASK(ptr, mask);
        significand_value /= 2;
        if (*ptr & mask)
            significand += significand_value;
        --num_fraction_bits;
    }
    
    // calculate final value
    exponent -= (::pow((long double)2, (int)(num_exp_bits - 1)) - 1);
    value = value_sign * significand * ::pow((long double)2, exponent);
}

void algorithm::float_to_bytes(long double value, unsigned char *buf, size_t num_exp_bits, size_t num_fraction_bits)
{
    // first initialize output buffer to zeros
    unsigned char *ptr = buf;
    memset(ptr, 0x00, ::ceil(static_cast<float>(num_exp_bits + num_fraction_bits + 1) / 8));
    
    // initialize first byte starting with sign of number
    if (value < 0) {
        *ptr = 0x80;
        value *= -1;
    }
    
    // skip past exponent bits because we don't know the value yet
    unsigned char mask = 0x40;
    for (size_t n = num_exp_bits; n > 0; --n) {
        if (n >= 8) {
            ++ptr;
            n -= 7;
        } else {
            SHIFT_BITMASK(ptr, mask);
        }
    }
    
    // break number into int value and fractional value
    bool got_exponent = false;
    boost::uint16_t num_bits = 0;
    boost::int16_t exponent = 0;
    long double high_bit = ::pow(2, (double)(num_fraction_bits - 1));
    long double int_value = ::floor(value);
    value -= int_value;
    
    // serialize int value >= 1.0
    if (int_value >= 1) {
        for (boost::int16_t high_bit_pos = num_fraction_bits - 1; high_bit_pos >= 0; --high_bit_pos) {
            if (got_exponent) {
                if (int_value >= high_bit) {
                    *ptr |= mask;
                    int_value -= high_bit;
                }
                SHIFT_BITMASK(ptr, mask);
                ++num_bits;
            } else {
                if (int_value >= high_bit) {
                    int_value -= high_bit;
                    exponent = high_bit_pos;
                    got_exponent = true;
                }
            }
            high_bit /= 2;
        }
    }
    
    // serialize fractional value < 1.0
    while (value && num_bits < num_fraction_bits) {
        value *= 2;
        if (got_exponent) {
            if (value >= 1.0) {
                *ptr |= mask;
                value -= 1.0;
            }
            SHIFT_BITMASK(ptr, mask);
            ++num_bits;
        } else {
            --exponent;
            if (value >= 1.0) {
                value -= 1.0;
                got_exponent = true;
            }
        }
    }
    
    // normalize exponent.
    // note: we should have a zero exponent if value == 0
    high_bit = ::pow((long double)2, (int)(num_exp_bits - 1));
    if (got_exponent)
        exponent += (high_bit - 1);
    else
        exponent = 0;
    
    // serialize exponent bits
    ptr = buf;
    mask = 0x80;
    for (size_t n = 0; n < num_exp_bits; ++n) {
        SHIFT_BITMASK(ptr, mask);
        if (exponent >= high_bit) {
            *ptr |= mask;
            exponent -= high_bit;
        }
        high_bit /= 2;
    }
}
    
}   // end namespace pion
