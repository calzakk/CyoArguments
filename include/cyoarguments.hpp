/*
[CyoArguments] cyoarguments.hpp

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

#ifndef __CYOARGUMENTS_HPP
#define __CYOARGUMENTS_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#ifdef _MSC_VER //case insensitivity only on Windows
#   define strcompare _stricmp
#   define strncompare _strnicmp
#else
#   define strcompare std::strcmp
#   define strncompare std::strncmp
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef UNREFERENCED_PARAMETER
#   define UNREFERENCED_PARAMETER(p) (void)(p)
#endif

namespace cyoarguments
{
    template<typename T>
    class Argument;

    using stringlist = std::list<std::string>;
    using stringlist_iter = stringlist::const_iterator;
}

#include "detail/traits.hpp"
#include "detail/getvalue.hpp"

#include "detail/base.hpp"
#include "detail/option.hpp"
#include "detail/required.hpp"
#include "detail/list.hpp"

namespace cyoarguments
{
    class Arguments final
    {
    public:
        Arguments(const Arguments&) = delete;
        Arguments& operator =(const Arguments&) = delete;

        Arguments() = default;

        void DisableHelp();

        void SetName(std::string name);

        void SetVersion(std::string version);

        void SetHeader(std::string header);

        void SetFooter(std::string footer);

        void AddGroup(std::string group);

        template<typename T>
        void AddOption(char letter, std::string word, std::string description, T& target);

        template<typename T>
        void AddOption(char letter, std::string description, T& target);

        template<typename T>
        void AddOption(std::string word, std::string description, T& target);

        template<typename T>
        void AddRequired(std::string name, std::string description, T& target);

        template<typename T>
        void AddList(std::string name, std::string description, T& target);

        bool Process(int argc, char* argv[], std::string& error) const;

        bool Process(int argc, char* argv[]) const;

    private:
        bool helpEnabled_ = true;
        std::string name_;
        std::string version_;
        std::string header_;
        std::string footer_;
        detail::OptionsList options_;
        detail::RequiredList required_;
        detail::ListPtr list_;
        bool allowEmpty_ = false;
        int numGroups_ = 0;

        void VerifyLetter(char letter);
        void VerifyWord(const std::string& word);
        void VerifyRequired(const std::string& name);
        void VerifyList(const std::string& name);
        bool ProcessImpl(int argc, char* argv[], std::string& error) const;
        bool FindHelpOrVersion(const stringlist& args) const;
        void DisplayHelp() const;
        void DisplayVersion() const;
        bool ProcessOptions(stringlist_iter& currArg, const stringlist_iter& lastArg) const;
        bool ProcessWord(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch) const;
        bool ProcessLetters(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch) const;
        bool ProcessRequired(stringlist_iter& currArg, const stringlist_iter& lastArg, detail::RequiredList::const_iterator& it) const;
        bool ProcessList(stringlist_iter& currArg, const stringlist_iter& lastArg) const;
    };

    template<typename T>
    class Argument final
    {
    public:
        Argument() = default;
        explicit Argument(const T& value) : value_(value) { }

        bool operator()() const { return !blank_; }
        T get() const { return value_; }
        void set(T value) { blank_ = false; value_ = std::move(value); }

    protected:
        bool blank_ = true;
        T value_;
    };
}

#include "detail/cyoarguments.inl"

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
