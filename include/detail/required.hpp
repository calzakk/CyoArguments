/*
[CyoArguments] detail/required.hpp

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

#ifndef __CYOARGUMENTS_REQUIRED_HPP
#define __CYOARGUMENTS_REQUIRED_HPP

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
        class RequiredBase : public ArgumentBase
        {
        public:
            RequiredBase() = default;
            virtual ~RequiredBase() = default;

            virtual const std::string& getName() const = 0;
        };

        using RequiredPtr = std::unique_ptr<RequiredBase>;

        using RequiredList = std::list<RequiredPtr>;

        ////////////////////////////////

        template<typename T>
        class Required final : public RequiredBase
        {
        public:
            Required(std::string name, std::string description, T& target)
                : name_(std::move(name)),
                description_(std::move(description)),
                target_(&target)
            {
            }

            const std::string& getName() const override { return name_; }

            void OutputUsage() const override
            {
                std::cout << ' ' << name_;
            }

            void OutputHelp() const override
            {
                std::cout << "  " << name_;
                for (auto i = name_.size(); i < optionWidth_; ++i)
                    std::cout << ' ';
                std::cout << description_;
                std::cout << '\n';
            }

            bool Process(stringlist_iter& currArg, const stringlist_iter& lastArg, int& ch, bool word, bool& error) const override
            {
                UNREFERENCED_PARAMETER(lastArg);
                UNREFERENCED_PARAMETER(word);
                T value;
                int len = GetValue(*currArg, value);
                if ((len >= 1) && (ch + len == (int)currArg->size()))
                {
                    GetValue(*currArg, *target_);
                    ch = 0;
                    error = false;
                    return true;
                }
                error = true;
                return false;
            }

        private:
            const std::string name_;
            const std::string description_;
            T* target_;
        };
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_REQUIRED_HPP
