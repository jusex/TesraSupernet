


#ifndef UNIVALUE_UTFFILTER_H
#define UNIVALUE_UTFFILTER_H

#include <string>

/**
 * Filter that generates and validates UTF-8, as well as collates UTF-16
 * surrogate pairs as specified in RFC4627.
 */
class JSONUTF8StringFilter
{
public:
    JSONUTF8StringFilter(std::string &s):
        str(s), is_valid(true), codepoint(0), state(0), surpair(0)
    {
    }
    
    void push_back(unsigned char ch)
    {
        if (state == 0) {
            if (ch < 0x80) 
                str.push_back(ch);
            else if (ch < 0xc0) 
                is_valid = false;
            else if (ch < 0xe0) { 
                codepoint = (ch & 0x1f) << 6;
                state = 6;
            } else if (ch < 0xf0) { 
                codepoint = (ch & 0x0f) << 12;
                state = 12;
            } else if (ch < 0xf8) { 
                codepoint = (ch & 0x07) << 18;
                state = 18;
            } else 
                is_valid = false;
        } else {
            if ((ch & 0xc0) != 0x80) 
                is_valid = false;
            state -= 6;
            codepoint |= (ch & 0x3f) << state;
            if (state == 0)
                push_back_u(codepoint);
        }
    }
    
    void push_back_u(unsigned int codepoint_)
    {
        if (state) 
            is_valid = false;
        if (codepoint_ >= 0xD800 && codepoint_ < 0xDC00) { 
            if (surpair) 
                is_valid = false;
            else
                surpair = codepoint_;
        } else if (codepoint_ >= 0xDC00 && codepoint_ < 0xE000) { 
            if (surpair) { 
                
                append_codepoint(0x10000 | ((surpair - 0xD800)<<10) | (codepoint_ - 0xDC00));
                surpair = 0;
            } else 
                is_valid = false;
        } else {
            if (surpair) 
                is_valid = false;
            else
                append_codepoint(codepoint_);
        }
    }
    
    
    bool finalize()
    {
        if (state || surpair)
            is_valid = false;
        return is_valid;
    }
private:
    std::string &str;
    bool is_valid;
    
    unsigned int codepoint;
    int state; 

    
    
    
    
    
    
    
    
    
    
    unsigned int surpair; 

    void append_codepoint(unsigned int codepoint_)
    {
        if (codepoint_ <= 0x7f)
            str.push_back((char)codepoint_);
        else if (codepoint_ <= 0x7FF) {
            str.push_back((char)(0xC0 | (codepoint_ >> 6)));
            str.push_back((char)(0x80 | (codepoint_ & 0x3F)));
        } else if (codepoint_ <= 0xFFFF) {
            str.push_back((char)(0xE0 | (codepoint_ >> 12)));
            str.push_back((char)(0x80 | ((codepoint_ >> 6) & 0x3F)));
            str.push_back((char)(0x80 | (codepoint_ & 0x3F)));
        } else if (codepoint_ <= 0x1FFFFF) {
            str.push_back((char)(0xF0 | (codepoint_ >> 18)));
            str.push_back((char)(0x80 | ((codepoint_ >> 12) & 0x3F)));
            str.push_back((char)(0x80 | ((codepoint_ >> 6) & 0x3F)));
            str.push_back((char)(0x80 | (codepoint_ & 0x3F)));
        }
    }
};

#endif
