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
        class OptionBase : public ArgumentBase
        {
        public:
            OptionBase() = default;
            virtual ~OptionBase() = default;

            static void OutputHelpImpl(char letter, const char* word, bool isNumeric, const char* description, bool isValueless)
            {
                std::cout << "  ";

                int len = 4;

#ifdef _MSC_VER
                const char letterPrefix = '/';
                const char* wordPrefix = "/";
#else
                const char letterPrefix = '-';
                const char* wordPrefix = "--";
#endif

                if (letter != '\0')
                    std::cout << letterPrefix << letter << (word != nullptr ? ", " : "  ");
                else
                    std::cout << "    "; //4 spaces

                if (word != nullptr && *word != '\0')
                {
                    std::cout << wordPrefix << word;
                    len += (int)(std::strlen(wordPrefix) + std::strlen(word));
                    if (!isValueless)
                    {
                        if (isNumeric)
                        {
                            std::cout << "=NUM";
                            len += 4;
                        }
                        else
                        {
                            std::cout << "=VALUE";
                            len += 6;
                        }
                    }
                }

                for (int i = len; i < optionWidth_; ++i)
                    std::cout << ' ';

                std::cout << description;
                std::cout << std::endl;
            }
        };

        using OptionPtr = std::unique_ptr<OptionBase>;

        using OptionsList = std::list<OptionPtr>;

        ////////////////////////////////

        template<typename T>
        class Option final : public OptionBase
        {
        public:
            Option(char letter, std::string word, std::string description, T& target)
                : letter_(letter),
                word_(std::move(word)),
                description_(std::move(description)),
                target_(&target)
            {
            }

            Option(char letter, std::string description, T& target)
                : letter_(letter),
                word_(),
                description_(std::move(description)),
                target_(&target)
            {
            }

            Option(std::string word, std::string description, T& target)
                : letter_('\0'),
                word_(std::move(word)),
                description_(std::move(description)),
                target_(&target)
            {
            }

            void OutputUsage() const override { } //nothing to do (options aren't listed individually)

            void OutputHelp() const override
            {
                OutputHelpImpl(letter_, word_.c_str(), isNumeric_, description_.c_str(), isValueless_);
            }

            bool Process(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch, bool word, bool& error) const override
            {
                error = false;

                if (word)
                {
                    if (!word_.empty()
                        && ProcessWord(currArg, lastArg, ch, error))
                        return true;
                }
                else
                {
                    if (Matches(letter_, currArg->at(ch))
                        && ProcessLetter(currArg, lastArg, ch, error))
                        return true;
                }

                return false;
            }

        private:
            const bool isValueless_ = is_valueless<T>::value;
            const bool isNumeric_ = is_numeric<T>::value;
            const bool requiresAssignment_ = requires_assignment<T>::value;
            char letter_;
            const std::string word_;
            const std::string description_;
            T* target_;

            bool ProcessWord(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch, bool& error) const
            {
                if (strcompare(currArg->c_str() + ch, word_.c_str()) == 0)
                {
                    // The argument matches the word
                    if (isValueless_) //bool
                    {
                        GetValue(*currArg, *target_);
                        return true;
                    }

                    if (GetValueFromNextArg(currArg, lastArg))
                        return true;

                    error = true;
                    return false;
                }

                auto wordLen = word_.size();
                if (strncompare(currArg->c_str() + ch, word_.c_str(), wordLen) == 0)
                {
                    // The argument starts with the word
                    if (isValueless_) //bool
                    {
                        error = true;
                        return false;
                    }

                    if (requiresAssignment_)
                    {
                        if (currArg->at(ch + wordLen) != '=')
                        {
                            error = true;
                            return false;
                        }
                        ++wordLen;
                    }
                    else if (currArg->at(ch + wordLen) == '=')
                        ++wordLen;

                    T value;
                    int len = GetValue(currArg->substr(ch + wordLen), value);
                    if (len >= 1)
                    {
                        if (ch + wordLen + len == (int)currArg->size())
                        {
                            GetValue(currArg->substr(ch + wordLen), *target_);
                            return true;
                        }
                    }
                    else if (GetValueFromNextArg(currArg, lastArg))
                        return true;

                    error = true;
                    return false;
                }

                return false; //not an error!
            }

            bool ProcessLetter(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch, bool& error) const
            {
                ++ch;

                if (isValueless_) //bool
                {
                    if ((ch < (int)currArg->size()) && (currArg->at(ch) == '='))
                    {
                        error = true;
                        return false;
                    }

                    GetValue(*currArg, *target_);
                    return true;
                }

                if (ch < (int)currArg->size())
                {
                    if (requiresAssignment_) //non-int
                    {
                        if (currArg->at(ch) != '=')
                        {
                            error = true;
                            return false;
                        }
                    }

                    if (currArg->at(ch) == '=')
                        ++ch;

                    if (ch < (int)currArg->size())
                    {
                        T value;
                        int len = GetValue(currArg->substr(ch), value);
                        if (len >= 1)
                        {
                            GetValue(currArg->substr(ch), *target_);
                            ch += len;
                            return true;
                        }
                    }
                }

                // Get value from the next argument...
                stringlist_iter nextArg = std::next(currArg);
                if (nextArg != lastArg)
                {
                    currArg = nextArg;
                    ch = GetValue(*currArg, *target_);
                    return true;
                }

                // No more arguments
                error = true;
                return false;
            }

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

            bool GetValueFromNextArg(stringlist_iter& currArg, const stringlist_iter& lastArg) const
            {
                stringlist_iter nextArg = std::next(currArg);
                if (nextArg != lastArg)
                {
                    // Get the value from the next argument...
                    if (!nextArg->empty())
                    {
#ifdef _MSC_VER
                        if (nextArg->at(0) == '/')
                            return false;
#endif
                        if (nextArg->at(0) == '-')
                            return false;
                    }

                    T value;
                    if (GetValue(*nextArg, value) >= 1)
                    {
                        GetValue(*nextArg, *target_);
                        currArg = nextArg;
                        return true;
                    }
                }

                return false;
            }
        };
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_OPTION_HPP
