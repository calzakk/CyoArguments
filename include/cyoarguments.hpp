/*
[CyoArguments] cyoarguments.hpp

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
    class Arguments;

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

        void DisableHelp() { helpEnabled_ = false; }

        void SetName(std::string name) { name_ = std::move(name); }

        void SetVersion(std::string version) { version_ = std::move(version); }

        void SetHeader(std::string header) { header_ = std::move(header); }

        void SetFooter(std::string footer) { footer_ = std::move(footer); }

        template<typename T>
        void AddOption(char letter, std::string word, std::string description, T& target)
        {
            VerifyLetter(letter);
            VerifyWord(word);
            options_.push_back(std::make_unique<detail::Option<T>>(letter, word, description, target));
        }

        template<typename T>
        void AddOption(char letter, std::string description, T& target)
        {
            VerifyLetter(letter);
            options_.push_back(std::make_unique<detail::Option<T>>(letter, description, target));
        }

        template<typename T>
        void AddOption(std::string word, std::string description, T& target)
        {
            VerifyWord(word);
            options_.push_back(std::make_unique<detail::Option<T>>('\0', word, description, target));
        }

        template<typename T>
        void AddRequired(std::string name, std::string description, T& target)
        {
            static_assert(detail::allow_required<T>::value, "Disallowed type of required argument");
            VerifyRequired(name);
            required_.push_back(std::make_unique<detail::Required<T>>(name, description, target));
        }

        template<typename T>
        void AddList(std::string name, std::string description, T& target)
        {
            static_assert(detail::is_container<T>::value, "Disallowed type of list argument");
            VerifyList(name);
            list_ = std::make_unique<detail::List<T>>(name, description, target);
        }

        bool Process(int argc, char* argv[], std::string& error) const
        {
            return ProcessImpl(argc, argv, error);
        }

        bool Process(int argc, char* argv[]) const
        {
            std::string error;
            if (ProcessImpl(argc, argv, error))
                return true;
            if (!error.empty())
                std::cerr << error << std::endl;
            return false;
        }

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

        void VerifyLetter(char letter)
        {
            if (!std::isalnum(letter))
                throw std::logic_error(std::string("Option is not alphanumeric: ") + letter);
        }

        void VerifyWord(const std::string& word)
        {
            if (word.empty())
                throw std::logic_error("Option cannot be blank");
            if (std::find_if(word.begin(), word.end(), [](char ch){ return !std::isalnum(ch); }) != word.end())
                throw std::logic_error(std::string("Option contains a non-alphanumeric character: ") + word);
        }

        void VerifyRequired(const std::string& name)
        {
            if (name.empty())
                throw std::logic_error("Required argument must have a name");
        }

        void VerifyList(const std::string& name)
        {
            if (list_)
                throw std::logic_error("Only one list argument can be specified");
            if (name.empty())
                throw std::logic_error("List argument must have a name");
        }

        bool ProcessImpl(int argc, char* argv[], std::string& error) const
        {
            error.clear();

            if (options_.empty() && required_.empty())
                throw std::logic_error("No optional or required arguments!");

            stringlist args;
            for (int index = 1; index < argc; ++index)
                args.emplace_back(argv[index]);

            // Help or version?

            if ((helpEnabled_ || !version_.empty()) && FindHelpOrVersion(args))
                return false;

            // Process optional and required arguments...

            auto nextRequired = required_.begin();

            auto lastArg = args.cend();
            for (auto currArg = args.cbegin(); currArg != lastArg; ++currArg)
            {
                if (currArg->empty())
                    continue;

                bool ok = false;
#ifdef _MSC_VER
                if (currArg->at(0) == '-' || currArg->at(0) == '/')
#else
                if (currArg->at(0) == '-')
#endif
                    ok = ProcessOptions(currArg, lastArg);
                else if (nextRequired != required_.end())
                    ok = ProcessRequired(currArg, lastArg, nextRequired);
                else if (list_)
                    ok = ProcessList(currArg, lastArg);

                if (!ok)
                {
                    error = "Invalid argument: ";
                    error += *currArg;
                    return false;
                }
            }

            if (nextRequired != required_.end())
            {
                // One or more missing required arguments
                error = "Missing argument: ";
                error += (*nextRequired)->getName();
                return false;
            }

            return true;
        }

        bool FindHelpOrVersion(const stringlist& args) const
        {
            for (auto& arg : args)
            {
                if (helpEnabled_)
                {
                    bool help = false;
#ifdef _MSC_VER
                    help = (strcompare(arg.c_str(), "/?") == 0) || (strcompare(arg.c_str(), "/help") == 0);
#endif
                    if (!help)
                        help = (strcompare(arg.c_str(), "-?") == 0) || (strcompare(arg.c_str(), "--help") == 0);
                    if (help)
                    {
                        DisplayHelp();
                        return true;
                    }
                }

                if (!version_.empty())
                {
                    bool version = false;
#ifdef _MSC_VER
                    version = (strcompare(arg.c_str(), "/?") == 0) || (strcompare(arg.c_str(), "/version") == 0);
#endif
                    if (!version)
                        version = (strcompare(arg.c_str(), "-?") == 0) || (strcompare(arg.c_str(), "--version") == 0);
                    if (version)
                    {
                        DisplayVersion();
                        return true;
                    }
                }
            }

            return false;
        }

        void DisplayHelp() const
        {
            if (!header_.empty())
                std::cout << header_ << "\n\n";

            std::cout << "Usage:";
            if (!name_.empty())
                std::cout << ' ' << name_;
            if (!options_.empty())
                std::cout << " [OPTION...]";
            for (const auto& required : required_)
                required->OutputUsage();
            if (list_)
                list_->OutputUsage();
            std::cout << '\n';

            if (!required_.empty() || list_)
            {
                std::cout << '\n';
                for (const auto& required : required_)
                    required->OutputHelp();
                if (list_)
                    list_->OutputHelp();
            }

            if (!options_.empty())
            {
                std::cout << "\nOptions:\n";
                for (const auto& option : options_)
                    option->OutputHelp();
                if (helpEnabled_)
                    detail::OptionBase::OutputHelpImpl('\0', "help", "display this help and exit", true);
                if (!version_.empty())
                    detail::OptionBase::OutputHelpImpl('\0', "version", "output version information and exit", true);
            }

            if (!footer_.empty())
                std::cout << '\n' << footer_ << '\n';

            std::cout << std::flush;
        }

        void DisplayVersion() const
        {
            std::cout << version_ << std::endl;
        }

        bool ProcessOptions(stringlist_iter& currArg, const stringlist_iter& lastArg) const
        {
            int ch = 0;

#ifdef _MSC_VER //only allow 'slash' arguments on Windows
            if (currArg->at(ch) == '/')
            {
                ++ch;
                if (ProcessWord(currArg, lastArg, ch))
                    return true;
                else
                    return ProcessLetters(currArg, lastArg, ch);
            }
#endif

            //'dash' arguments on all OSes
            assert(currArg->at(ch) == '-');
            ++ch;
            if (currArg->at(ch) == '-')
            {
                ++ch;
                return ProcessWord(currArg, lastArg, ch);
            }
            else
                return ProcessLetters(currArg, lastArg, ch);
        }

        bool ProcessWord(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch) const
        {
            for (const auto& option : options_)
            {
                bool error;
                if (option->Process(currArg, lastArg, ch, true, error))
                    return true;
                if (error)
                    return false;
            }
            return false;
        }

        bool ProcessLetters(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch) const
        {
            stringlist_iter startArg = currArg;
            while ((startArg == currArg) && (ch < (int)currArg->size()))
            {
                int chBak = ch;
                for (const auto& option : options_)
                {
                    bool error;
                    if (option->Process(currArg, lastArg, ch, false, error))
                        break;
                    if (error)
                        return false;
                }
                if ((startArg == currArg) && (chBak == ch))
                    return false;
            }
            return (ch == (int)currArg->size()); //true if at end of current arg
        }

        bool ProcessRequired(stringlist_iter& currArg, const stringlist_iter& lastArg, detail::RequiredList::const_iterator& it) const
        {
            int ch = 0;
            bool error;
            if ((*it)->Process(currArg, lastArg, ch, true, error))
            {
                ++it;
                return true;
            }
            else
                return false;
        }

        bool ProcessList(stringlist_iter& currArg, const stringlist_iter& lastArg) const
        {
            int ch = 0;
            bool error;
            list_->Process(currArg, lastArg, ch, true, error); //TODO: CHECK RETURN CODE?
            return true;
        }
    };
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
