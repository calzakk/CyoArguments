/*
[CyoArguments] detail/traits.hpp

The MIT License (MIT)

Copyright (c) 2015-2016 Graham Bull

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
        // allow_required_argument is used to prohibit certain data types from being used as required arguments
        template<typename T> struct allow_required_argument : std::true_type { };
        template<> struct allow_required_argument<bool> : std::false_type { };
        template<typename U> struct allow_required_argument<Argument<U>> : std::false_type { };
        template<typename U> struct allow_required_argument<std::list<U>> : std::false_type { };
        template<typename U> struct allow_required_argument<std::vector<U>> : std::false_type { };

        // allow_list_argument is used to allow certain data types to be used as list arguments
        template<typename T> struct allow_list_argument : std::false_type { };
        template<typename U> struct allow_list_argument<std::list<U>> : std::true_type { };
        template<typename U> struct allow_list_argument<std::vector<U>> : std::true_type { };

        // is_valueless is used to prohibit assignments to certain data types
        // (for example, booleans cannot be assigned: --version=true)
        template<typename T> struct is_valueless : std::false_type { };
        template<> struct is_valueless<bool> : std::true_type { };
        template<> struct is_valueless<Argument<bool>> : std::true_type { };

        // requires_assignment determines which options can be specified without an assignment
        // (for example, allow -j5 as well as -j=5)
        template<typename T> struct requires_assignment : std::true_type { };
        template<> struct requires_assignment<int> : std::false_type { };
        template<> struct requires_assignment<Argument<int>> : std::false_type { };
        template<> struct requires_assignment<unsigned int> : std::false_type { };
        template<> struct requires_assignment<Argument<unsigned int>> : std::false_type { };

        // is_numeric determines whether NUM or VALUE is output in the help text
        // (for example, --age=NUM or --name=VALUE)
        template<typename T> struct is_numeric : std::false_type { };
        template<> struct is_numeric<int> : std::true_type { };
        template<> struct is_numeric<Argument<int>> : std::true_type { };
        template<> struct is_numeric<unsigned int> : std::true_type { };
        template<> struct is_numeric<Argument<unsigned int>> : std::true_type { };
        template<> struct is_numeric<float> : std::true_type { };
        template<> struct is_numeric<Argument<float>> : std::true_type { };
        template<> struct is_numeric<double> : std::true_type { };
        template<> struct is_numeric<Argument<double>> : std::true_type { };
        template<typename U> struct is_numeric<std::list<U>> : is_numeric<U> { };
        template<typename U> struct is_numeric<std::vector<U>> : is_numeric<U> { };
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_TRAITS_HPP
