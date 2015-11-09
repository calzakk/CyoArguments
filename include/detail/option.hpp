/*
[CyoArguments] detail/option.hpp

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

#ifndef __CYOARGUMENTS_OPTION_HPP
#define __CYOARGUMENTS_OPTION_HPP

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
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
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_OPTION_HPP
