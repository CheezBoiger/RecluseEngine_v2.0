// Recluse v2.0 (c) All rights reserved.
//
#ifndef RECLUSE_STRING_HPP
#define RECLUSE_STRING_HPP

#pragma once
#include "Recluse/Types.hpp"

// Defines the recluse text string that would be ideal for handling unicode formats.
#define R_TEXT(s) (u8##s)

namespace Recluse {
//
////! String class object. Defines the standard type for string objects in Recluse engine.
////! Note:   This does not absolutely need to be used for applications, but would be desirable
////!         in order to manually handle string types in C++.
class R_PUBLIC_API String 
{
public:
    String() : m_size(0), m_cStr(nullptr) { }
    String(String&& other);
    String(const String& other);
    String(const U8* other);

    ~String();
    
    String& operator=(String&& other);
    String& operator=(const String& other);
    String& operator=(const U8* other);

    String& operator+(const String& other);
    String& operator+(const U8* other);

    void operator+=(const String& other);
    void operator+=(const U8* other);

    const U8* raw() const { return m_cStr; }
    U8* raw() { return m_cStr; }

    U32 getSize() const { return m_size; }
    
    //! Check if string is equal to other string.
    B32 equalTo(const String& other);
    B32 equalTo(const U8* other);

    String substr(U32 start, U32 end);
    String beginsWith(const String& str);
    String replace(const String& str);
    
private:
    //! Actual string data, in characters.
    U8* m_cStr;
    //! Size, in characters.
    U32 m_size;
}; // String


R_PUBLIC_API std::string toLowerCase(const std::string& str);
R_PUBLIC_API std::string toUpperCase(const std::string& str);

} // Recluse
#endif // RECLUSE_STRING_HPP