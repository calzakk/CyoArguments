/*
[CyoArguments] detail/list.hpp

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

#ifndef __CYOARGUMENTS_LIST_HPP
#define __CYOARGUMENTS_LIST_HPP

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
        class ListBase : public ArgumentBase
        {
        public:
            ListBase() = default;
            virtual ~ListBase() = default;
        };

        using ListPtr = std::unique_ptr<ListBase>;

        ////////////////////////////////

        template<typename T>
        class List final : public ListBase
        {
        public:
            List(std::string name, std::string description, T& target)
                : name_(std::move(name)),
                description_(std::move(description)),
                target_(&target)
            {
            }

            void OutputUsage() const override
            {
                std::cout << ' ' << name_ << "...";
            }

            void OutputHelp() const override
            {
                std::cout << "  " << name_ << "...";
                for (auto i = name_.size() + 3; i < optionWidth_; ++i)
                    std::cout << ' ';
                std::cout << description_;
                std::cout << '\n';
            }

            bool Process(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch, bool word, bool& error) const override
            {
                UNREFERENCED_PARAMETER(lastArg);
                UNREFERENCED_PARAMETER(word);
                detail::GetValue(*currArg, *target_);
                ch = 0;
                error = false;
                return false;
            }

        private:
            std::string name_;
            std::string description_;
            T* target_;
        };
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_LIST_HPP
