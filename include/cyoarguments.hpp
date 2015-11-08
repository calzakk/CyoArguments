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
        void set(T value) {
            value_ = std::move(value);
        }

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
            virtual ~OptionBase() { }

            virtual void Output() const = 0;
            virtual bool Process(int argc, char* argv[], int& index, int& ch, bool word, bool& error) const = 0;

        protected:
            static const int optionWidth_ = 20;
        };

        using OptionPtr = std::unique_ptr<OptionBase>;

        template<typename T>
        class Option final : public OptionBase
        {
        public:
            Option(char letter, const char* word, const char* description, T& target)
                : letter_(letter), word_(word), description_(description), target_(&target)
            {
            }

            Option(char letter, const char* description, T& target)
                : letter_(letter), word_(nullptr), description_(description), target_(&target)
            {
            }

            Option(const char* word, const char* description, T& target)
                : letter_('\0'), word_(word), description_(description), target_(&target)
            {
            }

            void Output() const override
            {
                std::cout << "  ";

                int len = 4;

#ifdef _MSC_VER
                if (letter_ != '\0')
                    std::cout << '/' << letter_ << (word_ != nullptr ? ", " : "  ");
                else
                    std::cout << "    "; //4 spaces

                if (word_ != nullptr)
                {
                    std::cout << '/' << word_;
                    len += ((int)std::strlen(word_) + 1);
                    if (!valueless_)
                    {
                        std::cout << "=X";
                        len += 2;
                    }
                }
#else
                if (letter_ != '\0')
                    std::cout << '-' << letter_ << (word_ != nullptr ? ", " : "  ");
                else
                    std::cout << "    "; //4 spaces

                if (word_ != nullptr)
                {
                    std::cout << "--" << word_;
                    len += ((int)std::strlen(word_) + 2);
                    if (!valueless_)
                    {
                        std::cout << "=X";
                        len += 2;
                    }
                }
#endif

                for (int i = len; i < optionWidth_; ++i)
                    std::cout << ' ';

                std::cout << description_;
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
                        if (argStart[wordLen] == '\0')
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
                                if (argStart[wordLen + len] == '\0')
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
            RequiredBase(const char* name, const char* description)
                : name_(name), description_(description)
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
            Required(const char* name, const char* description, T& target)
                : RequiredBase(name, description), target_(&target)
            {
            }

            void Output() const override
            {
                std::cout << "  " << name_;
                for (auto i = std::strlen(name_); i < optionWidth_; ++i)
                    std::cout << ' ';
                std::cout << description_;
                std::cout << '\n';
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

        void SetName(std::string name) { name_ = std::move(name); }

        void SetHeader(std::string header) { header_ = std::move(header); }

        void SetFooter(std::string footer) { footer_ = std::move(footer); }

        template<typename T>
        void AddOption(char letter, const char* word, const char* description, T& target)
        {
            VerifyLetter(letter);
            VerifyWord(word);
            options_.push_back(std::make_unique<detail::Option<T>>(letter, word, description, target));
        }

        template<typename T>
        void AddOption(char letter, const char* description, T& target)
        {
            VerifyLetter(letter);
            options_.push_back(std::make_unique<detail::Option<T>>(letter, nullptr, description, target));
        }

        template<typename T>
        void AddOption(const char* word, const char* description, T& target)
        {
            VerifyWord(word);
            options_.push_back(std::make_unique<detail::Option<T>>('\0', word, description, target));
        }

        template<typename T>
        void AddRequired(const char* name, const char* description, T& target)
        {
            static_assert(detail::allowRequired<T>::value, "Disallowed type of required argument");
            VerifyRequired(name);
            required_.push_back(std::make_unique<detail::Required<T>>(name, description, target));
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

        std::string name_;
        std::string header_;
        std::string footer_;
        OptionsList options_;
        OptionsList required_;
        bool allowEmpty_ = false;

        void VerifyLetter(char letter)
        {
            if (!std::isalnum(letter))
                throw std::runtime_error(std::string("Option is not alphanumeric: ") + std::string(1, letter));
        }

        void VerifyWord(const char* word)
        {
            if (word == nullptr)
                throw std::runtime_error("Option cannot be null");
            auto wordLen = std::strlen(word);
            if (wordLen < 2)
                throw std::runtime_error(std::string("Option's length must be two or more characters: ") + word);
            if (std::find_if(word, word + wordLen, [](char ch){ return !std::isalnum(ch); }) != word + wordLen)
                throw std::runtime_error(std::string("Option contains a non-alphanumeric character: ") + word);
        }

        void VerifyRequired(const char* name)
        {
            if (name == nullptr)
                throw std::runtime_error("Name of required argument is null");
            if (std::strlen(name) < 1)
                throw std::runtime_error("Name of required argument has insufficient length");
        }

        void HelpImpl() const
        {
            if (!header_.empty())
                std::cout << header_ << "\n\n";

            std::cout << "Usage:";
            if (!name_.empty())
                std::cout << ' ' << name_;
            if (!options_.empty())
                std::cout << " [OPTION...]";
            for (const auto& required : required_)
                std::cout << ' ' << ((detail::RequiredBase*)required.get())->getName();
            std::cout << "\n\n";

            if (!required_.empty())
            {
                for (const auto& required : required_)
                    required->Output();
                std::cout << '\n';
            }

            if (!options_.empty())
            {
                std::cout << "Options:\n";
                for (const auto& option : options_)
                    option->Output();
                std::cout << '\n';
            }

            if (!footer_.empty())
                std::cout << footer_ << '\n';

            std::cout << std::endl;
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
            while ((indexBak == index) && (argv[index][ch] != '\0'))
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
            return (argv[index][ch] == '\0'); //true if at end of arg
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

    class Version final
    {
    public:
        static const char* Str() { return "0.1.0"; }
        static int Major() { return 0; }
        static int Minor() { return 1; }
        static int Patch() { return 0; }
    };
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
