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

#include <iostream>
#include <list>
#include <memory>

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
        class OptionBase
        {
        public:
            virtual void Output() const = 0;
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
                std::cout << " -" << letter_;
                std::cout << " --" << word_;
                std::cout << "  " << description_;
                std::cout << std::endl;
            }
        private:
            char letter_;
            const char* word_;
            const char* description_;
            T* target_;
        };

        template<typename T>
        class Fixed final : public OptionBase
        {
        public:
            Fixed(const char* name, const char* description, T& target)
                : name_(name), description_(description), target_(&target)
            {
            }
            void Output() const override
            {
                std::cout << "FIXED:";
                std::cout << " " << name_;
                std::cout << "  " << description_;
                std::cout << std::endl;
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

        Arguments() { }

        template<typename T>
        void AddOption(char letter, const char* word, const char* description, T& target)
        {
            options_.push_back(std::make_unique<detail::Option<T>>(letter, word, description, target));
        }

        template<typename T>
        void AddFixed(const char* name, const char* description, T& target)
        {
            options_.push_back(std::make_unique<detail::Fixed<T>>(name, description, target));
        }

        void Help() const
        {
            for (auto& option : options_)
                option->Output();
        }

        void Process(int argc, char* argv[])
        {
            for (int i = 1; i < argc; ++i)
                std::cout << argv[i] << std::endl;
        }

    private:
        std::list<detail::OptionPtr> options_;
    };
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
