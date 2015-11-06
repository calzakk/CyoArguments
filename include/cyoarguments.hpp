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

#include "detail/make_unique.hpp"

#ifdef _MSC_VER //case insensitivity only on Windows
//#   define strcompare _stricmp
#   define strncompare _strnicmp
#else
//#   define strcompare std::strcmp
#   define strncompare std::strncmp
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef UNREFERENCED_PARAMETER
#   define UNREFERENCED_PARAMETER(p) (void)(p)
#endif

namespace cyoarguments
{
    class Arguments;

    ////////////////////////////////////

    template<typename T>
    class Argument final
    {
    public:
        Argument() = default;
        Argument(const T& value) : value_(value) { }
        bool operator()() const { return !blank_; }
        T get() const { return value_; }
        void set(T value) { value_ = std::move(value); }

    protected:
        bool blank_ = true;
        T value_;
        Argument(Argument& src, T&& value) { src.blank_ = false; src.value_ = value; }
    };

    ////////////////////////////////////

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

        template<typename T>
        int GetValue(const char* arg, T& target)
        {
            UNREFERENCED_PARAMETER(arg);
            UNREFERENCED_PARAMETER(target);
            throw std::logic_error("Unsupported argument type");
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

        class OptionBase
        {
        public:
            OptionBase(Arguments& parent) : parent_(&parent) { }
            virtual ~OptionBase() { }

            OptionBase& operator=(const OptionBase& src) { parent_ = src.parent_; return *this; }

            virtual void Output() const = 0;
            virtual bool Process(int argc, char* argv[], int& index, int& ch, bool word, bool& error) const = 0;

        protected:
            Arguments* parent_;
        };

        using OptionPtr = std::unique_ptr<OptionBase>;

        template<typename T>
        class Option final : public OptionBase
        {
        public:
            Option(Arguments& parent, char letter, const char* word, const char* description, T& target)
                : OptionBase(parent), letter_(letter), word_(word), description_(description), target_(&target)
            {
            }

            void Output() const override
            {
                std::cout << "OPTION:";
#ifdef _MSC_VER
                if (letter_ != '\x0')
                    std::cout << " /" << letter_;
                if (word_ != nullptr)
                    std::cout << " /" << word_;
#else
                if (letter_ != '\x0')
                    std::cout << " -" << letter_;
                if (word_ != nullptr)
                    std::cout << " --" << word_;
#endif
                std::cout << "  " << description_;
                std::cout << std::endl;
            }

            bool Process(int argc, char* argv[], int& index, int& ch, bool word, bool& error) const override
            {
                error = false;

                if (word && word_ != nullptr)
                {
                    char* argStart = argv[index] + ch;
                    int wordLen = (int)std::strlen(word_);
                    if (strncompare(word_, argStart, wordLen) == 0)
                    {
                        if (argStart[wordLen] == '\x0')
                        {
                            // The argument matches the word
                            if (valueless_)
                            {
                                //bool
                                detail::GetValue(argv[index], *target_);
                                return true;
                            }

                            if (index + 1 < argc)
                            {
                                // Get the value from the next argument...
                                int newIndex = (index + 1);
#ifdef _MSC_VER
                                if (argv[newIndex][0] == '/')
                                {
                                    error = true;
                                    return false;
                                }
#endif
                                if (argv[newIndex][0] == '-')
                                {
                                    error = true;
                                    return false;
                                }

                                T value;
                                if (detail::GetValue(argv[newIndex], value) >= 1)
                                {
                                    *target_ = value;
                                    index = newIndex;
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            // The argument starts with the word
                            if (valueless_)
                            {
                                //bool
                                error = true;
                                return false;
                            }

                            if (requiresEquals_)
                            {
                                if (argStart[wordLen] != '=')
                                {
                                    error = true;
                                    return false;
                                }
                                ++wordLen;
                            }
                            else if (argStart[wordLen] == '=')
                                ++wordLen;

                            T value;
                            int len = detail::GetValue(argStart + wordLen, value);
                            if (len >= 1)
                            {
                                if (argStart[wordLen + len] == '\x0')
                                {
                                    *target_ = value;
                                    return true;
                                }
                            }
                            else if (len == 0)
                            {
                                if (index + 1 < argc)
                                {
                                    // Get the value from the next argument...
                                    int newIndex = (index + 1);
#ifdef _MSC_VER
                                    if (argv[newIndex][0] == '/')
                                    {
                                        error = true;
                                        return false;
                                    }
#endif
                                    if (argv[newIndex][0] == '-')
                                    {
                                        error = true;
                                        return false;
                                    }
                                    T value;
                                    int len = detail::GetValue(argv[newIndex], value);
                                    if (len >= 1)
                                    {
                                        *target_ = value;
                                        index = newIndex;
                                        return true;
                                    }
                                }
                            }
                        }
                        error = true;
                    }
                }
                else if (!word && Matches(letter_, argv[index][ch]))
                {
                    ++ch;

                    if (valueless_)
                    {
                        // bool
                        if (argv[index][ch] == '=')
                        {
                            error = true;
                            return false;
                        }
                        else
                        {
                            detail::GetValue(argv[index], *target_);
                            return true;
                        }
                    }
                    else
                    {
                        if (requiresEquals_)
                        {
                            //non-int
                            if ((argv[index][ch] != '\0') && (argv[index][ch] != '='))
                            {
                                error = true;
                                return false;
                            }
                            else if (argv[index][ch] == '=')
                                ++ch;
                        }
                        else
                        {
                            //int
                            if (argv[index][ch] == '=')
                                ++ch;
                        }
                    }

                    T value;
                    int len = detail::GetValue(argv[index] + ch, value);
                    if (len >= 1)
                    {
                        *target_ = value;
                        ch += len;
                        return true;
                    }
                    else if (index + 1 < argc)
                    {
                        // Get value from the next argument...
                        ch = detail::GetValue(argv[++index], *target_);
                        return true;
                    }
                    else
                        error = true;
                }

                return false;
            }

        private:
            const bool valueless_ = detail::valueless<T>::value;
            const bool requiresEquals_ = detail::requiresEquals<T>::value;
            char letter_;
            const char* word_;
            const char* description_;
            T* target_;

            bool Matches(char ch1, char ch2) const
            {
#ifdef _MSC_VER //case insensitivity only on Windows
                if ('A' <= ch1 && ch1 <= 'Z')
                    ch1 |= (1 << 5); //make lowercase
                if ('A' <= ch2 && ch2 <= 'Z')
                    ch2 |= (1 << 5); //make lowercase
#endif
                return (ch1 == ch2);
            }
        };

        class RequiredBase : public OptionBase
        {
        public:
            RequiredBase(Arguments& parent, const char* name, const char* description)
                : OptionBase(parent), name_(name), description_(description)
            {
            }

            virtual ~RequiredBase() { }

            const char* getName() const { return name_; }

        protected:
            const char* name_;
            const char* description_;
        };

        template<typename T>
        class Required final : public RequiredBase
        {
        public:
            Required(Arguments& parent, const char* name, const char* description, T& target)
                : RequiredBase(parent, name, description), target_(&target)
            {
            }

            void Output() const override
            {
                std::cout << "REQUIRED:";
                std::cout << " " << name_;
                std::cout << "  " << description_;
                std::cout << std::endl;
            }

            bool Process(int argc, char* argv[], int& index, int& ch, bool word, bool& error) const override
            {
                UNREFERENCED_PARAMETER(argc);
                UNREFERENCED_PARAMETER(word);
                detail::GetValue(argv[index], *target_);
                ch = 0;
                error = false;
                return false;
            }

        private:
            T* target_;
        };
    }

    ////////////////////////////////////

    class Arguments final
    {
    public:
        Arguments(const Arguments&) = delete;
        Arguments& operator =(const Arguments&) = delete;

        Arguments() = default;

        template<typename T>
        void AddOption(char letter, const char* word, const char* description, T& target)
        {
            if (!std::isalnum(letter))
                throw std::runtime_error(std::string("Option is not alphanumeric: ") + std::string(1, letter));
            options_.push_back(std::make_unique<detail::Option<T>>(*this, letter, word, description, target));
        }

        template<typename T>
        void AddOption(char letter, const char* description, T& target)
        {
            if (!std::isalnum(letter))
                throw std::runtime_error(std::string("Option is not alphanumeric: ") + std::string(1, letter));
            options_.push_back(std::make_unique<detail::Option<T>>(*this, letter, nullptr, description, target));
        }

        template<typename T>
        void AddOption(const char* word, const char* description, T& target)
        {
            if (word == nullptr)
                throw std::runtime_error("Option is null");
            auto wordLen = std::strlen(word);
            if (wordLen < 2)
                throw std::runtime_error(std::string("Option's length must be two or more characters: ") + word);
            if (std::find_if(word, word + wordLen, [](char ch){ return !std::isalnum(ch); }) != word + wordLen)
                throw std::runtime_error(std::string("Option contains a non-alphanumeric character: ") + word);
            options_.push_back(std::make_unique<detail::Option<T>>(*this, '\x0', word, description, target));
        }

        template<typename T>
        void AddRequired(const char* name, const char* description, T& target)
        {
            static_assert(detail::allowRequired<T>::value, "Disallowed type of required argument");
            if (name == nullptr)
                throw std::runtime_error("Name of required argument is null");
            if (std::strlen(name) < 1)
                throw std::runtime_error("Name of required argument has insufficient length");
            required_.push_back(std::make_unique<detail::Required<T>>(*this, name, description, target));
        }

        void Help() const
        {
            HelpImpl();
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
            std::cerr << error << std::endl;
            return false;
        }

    private:
        using OptionsList = std::list<detail::OptionPtr>;
        OptionsList options_;
        OptionsList required_;
        bool allowEmpty_ = false;

        void HelpImpl() const
        {
            for (const auto& required : required_)
                required->Output();
            for (const auto& option : options_)
                option->Output();
        }

        bool ProcessImpl(int argc, char* argv[], std::string& error) const
        {
            error.clear();

            auto nextRequired = required_.begin();

            for (int index = 1; index < argc; ++index)
            {
                bool ok;
#ifdef _MSC_VER
                if (argv[index][0] == '-' || argv[index][0] == '/')
#else
                if (argv[index][0] == '-')
#endif
                    ok = ProcessOptions(argc, argv, index);
                else
                    ok = ProcessRequired(argc, argv, index, nextRequired);

                if (!ok)
                {
                    error = "Invalid argument: ";
                    error += argv[index];
                    return false;
                }
            }

            if (nextRequired != required_.end())
            {
                auto& req = *nextRequired;

                detail::RequiredBase* pReq = (detail::RequiredBase*)req.get();

                error = "Missing argument: ";
                error += pReq->getName();
                return false;
            }

            return true;
        }

        bool ProcessOptions(int argc, char* argv[], int& index) const
        {
            int ch = 0;

#ifdef _MSC_VER //only allow 'slash' arguments on Windows
            if (argv[index][ch] == '/')
            {
                ++ch;
                if (ProcessWord(argc, argv, index, ch))
                    return true;
                else
                    return ProcessLetters(argc, argv, index, ch);
            }
#endif

            //'dash' arguments on all OSes
            assert(argv[index][ch] == '-');
            ++ch;
            if (argv[index][ch] == '-')
            {
                ++ch;
                return ProcessWord(argc, argv, index, ch);
            }
            else
                return ProcessLetters(argc, argv, index, ch);
        }

        bool ProcessWord(int argc, char* argv[], int& index, int& ch) const
        {
            for (const auto& option : options_)
            {
                bool error;
                if (option->Process(argc, argv, index, ch, true, error))
                    return true;
                if (error)
                    return false;
            }
            return false;
        }

        bool ProcessLetters(int argc, char* argv[], int& index, int& ch) const
        {
            int indexBak = index;
            while ((indexBak == index) && (argv[index][ch] != '\x0'))
            {
                int chBak = ch;
                for (const auto& option : options_)
                {
                    bool error;
                    if (option->Process(argc, argv, index, ch, false, error))
                        break;
                    if (error)
                        return false;
                }
                if ((indexBak == index) && (chBak == ch))
                    return false;
            }
            return (argv[index][ch] == '\x0'); //true if at end of arg
        }

        bool ProcessRequired(int argc, char* argv[], int& index, OptionsList::const_iterator& it) const
        {
            if (it == required_.end())
                return false;

            int ch = 0;
            bool error;
            (*it)->Process(argc, argv, index, ch, true, error); //TODO: CHECK RETURN CODE?
            ++it;
            return true;
        }
    };
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
