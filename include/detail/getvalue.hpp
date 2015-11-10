/*
[CyoArguments] detail/getvalue.hpp

The MIT License (MIT)

Copyright (c) 2015 Graham Bull

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __CYOARGUMENTS_GETVALUE_HPP
#define __CYOARGUMENTS_GETVALUE_HPP

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
        template<typename T>
        int GetValue(const char* arg, T& target)
        {
            UNREFERENCED_PARAMETER(arg);
            UNREFERENCED_PARAMETER(target);
#ifdef _MSC_VER
            static_assert(false, "Unsupported argument type");
#else
            throw std::logic_error("Unsupported argument type");
#endif
        }

        template<>
        int GetValue(const char* arg, bool& target)
        {
            UNREFERENCED_PARAMETER(arg);
            target = true;
            return 0;
        }

        template<>
        int GetValue(const char* arg, int& target)
        {
            char* endptr = nullptr;
            target = (int)std::strtol(arg, &endptr, 0);
            return (int)(endptr - arg);
        }

        template<>
        int GetValue(const char* arg, unsigned int& target)
        {
            char* endptr = nullptr;
            target = (unsigned int)std::strtoul(arg, &endptr, 0);
            return (int)(endptr - arg);
        }

        template<>
        int GetValue(const char* arg, float& target)
        {
            char* endptr = nullptr;
            target = std::strtof(arg, &endptr);
            return (int)(endptr - arg);
        }

        template<>
        int GetValue(const char* arg, double& target)
        {
            char* endptr = nullptr;
            target = std::strtod(arg, &endptr);
            return (int)(endptr - arg);
        }

        template<>
        int GetValue(const char* arg, std::string& target)
        {
            if (*arg == '=')
            {
                target = (arg + 1);
                return ((int)target.size() + 1);
            }
            else
            {
                target = arg;
                return (int)target.size();
            }
        }

        template<typename T>
        int GetValue(const char* arg, Argument<T>& target)
        {
            T value;
            int ret = GetValue(arg, value);
            target.set(value);
            return ret;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_GETVALUE_HPP
