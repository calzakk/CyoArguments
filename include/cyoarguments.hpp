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

#include <cassert>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

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
    namespace detail
    {
        class OptionBase
        {
        public:
            virtual void Output() const = 0;
            virtual bool Process(int argc, char* argv[], int& index, int& ch, bool word) const = 0;

        protected:
            template<typename T>
            int GetValue(const char* arg, T& target) const
            {
                UNREFERENCED_PARAMETER(arg);
                UNREFERENCED_PARAMETER(target);
                throw std::logic_error("Unsupported argument type");
            }
            template<>
            int GetValue(const char* arg, bool& target) const
            {
                UNREFERENCED_PARAMETER(arg);
                target = true;
                return 0;
            }
            template<>
            int GetValue(const char* arg, int& target) const
            {
                char* endptr = nullptr;
                target = (int)std::strtol(arg, &endptr, 0);
                return (int)(endptr - arg);
            }
            template<>
            int GetValue(const char* arg, float& target) const
            {
                char* endptr = nullptr;
                target = std::strtof(arg, &endptr);
                return (int)(endptr - arg);
            }
            template<>
            int GetValue(const char* arg, double& target) const
            {
                char* endptr = nullptr;
                target = std::strtod(arg, &endptr);
                return (int)(endptr - arg);
            }
            template<>
            int GetValue(const char* arg, std::string& target) const
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
            bool Process(int argc, char* argv[], int& index, int& ch, bool word) const override
            {
                argc, argv; index; ch; word; //TEMP

                if (word && word_ != nullptr)
                {
                    char* argStart = argv[index] + ch;

                    if (strcompare(word_, argStart) == 0)
                    {
                        int newIndex = (index + 1);
                        if (GetValue(argv[newIndex], *target_) >= 1)
                            index = newIndex;
                        return true;
                    }

                    int wordLen = (int)std::strlen(word_);
                    if (strncompare(word_, argStart, wordLen) == 0)
                    {
                        if (argStart[wordLen] == '=')
                        {
                            int val = GetValue(argStart + wordLen + 1, *target_);
                            (val);
                            //todo
                        }
                        else
                        {
                            int val = GetValue(argStart + wordLen, *target_);
                            (val);
                            //todo
                        }
                        return true;
                    }
                }

                if (!word && Matches(letter_, argv[index][ch]))
                {
                    ++ch;
                    int len = GetValue(argv[index] + ch, *target_);
                    ch += len;
                    return true;
                }

                return false;
            }
        private:
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

        template<typename T>
        class Required final : public OptionBase
        {
        public:
            Required(const char* name, const char* description, T& target)
                : name_(name), description_(description), target_(&target)
            {
            }
            void Output() const override
            {
                std::cout << "REQUIRED:";
                std::cout << " " << name_;
                std::cout << "  " << description_;
                std::cout << std::endl;
            }
            bool Process(int argc, char* argv[], int& index, int& ch, bool word) const override
            {
                UNREFERENCED_PARAMETER(argc);
                UNREFERENCED_PARAMETER(word);
                GetValue(argv[index], *target_);
                ch = 0;
                return false;
            }
        private:
            const char* name_;
            const char* description_;
            T* target_;
        };
    }

    ///////////////////////////////////

    class Arguments final
    {
    public:
        Arguments(const Arguments&) = delete;
        Arguments& operator =(const Arguments&) = delete;

        Arguments() = default;

        template<typename T>
        void AddOption(char letter, const char* word, const char* description, T& target)
        {
            options_.push_back(std::make_unique<detail::Option<T>>(letter, word, description, target));
        }

        template<typename T>
        void AddRequired(const char* name, const char* description, T& target)
        {
            required_.push_back(std::make_unique<detail::Required<T>>(name, description, target));
        }

        void Help() const
        {
            HelpImpl();
        }

        bool Process(int argc, char* argv[]) const
        {
            return ProcessImpl(argc, argv);
        }

    private:
        using OptionsList = std::list<detail::OptionPtr>;
        OptionsList options_;
        OptionsList required_;

        void HelpImpl() const
        {
            for (const auto& required : required_)
                required->Output();
            for (const auto& option : options_)
                option->Output();
        }

        bool ProcessImpl(int argc, char* argv[]) const
        {
#ifdef _DEBUG //TEMP
            std::cout << std::endl;
            for (int index = 1; index < argc; ++index)
                std::cout << argv[index] << std::endl;
#endif

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
                    std::cerr << "Invalid argument: " << argv[index] << std::endl;
                    return false;
                }
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
                if (option->Process(argc, argv, index, ch, true))
                    return true;
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
                    if (option->Process(argc, argv, index, ch, false))
                        break;
                }
                if (chBak == ch)
                    return false;
            }
            return (argv[index][ch] == '\x0'); //true if at end of arg
        }

        bool ProcessRequired(int argc, char* argv[], int& index, OptionsList::const_iterator& it) const
        {
            if (it == required_.end())
                return false;

            int ch = 0;
            (*it)->Process(argc, argv, index, ch, true); //TODO: CHECK RETURN CODE?
            ++it;
            return true;
        }
    };
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
