/*
[CyoArguments] detail/traits.hpp

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

#ifndef __CYOARGUMENTS_TRAITS_HPP
#define __CYOARGUMENTS_TRAITS_HPP

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
        template<typename T> struct allowRequired : std::true_type { };
        template<> struct allowRequired<bool> : std::false_type { };
        template<typename T> struct allowRequired<Argument<T>> : std::false_type { };

        template<typename T> struct valueless : std::false_type { };
        template<> struct valueless<bool> : std::true_type { };
        template<> struct valueless<Argument<bool>> : std::true_type { };

        template<typename T> struct requiresEquals : std::true_type { };
        template<> struct requiresEquals<int> : std::false_type { };
        template<> struct requiresEquals<Argument<int>> : std::false_type { };
        template<> struct requiresEquals<unsigned int> : std::false_type { };
        template<> struct requiresEquals<Argument<unsigned int>> : std::false_type { };
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_TRAITS_HPP