/*
[CyoArguments] detail/required.hpp

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

#ifndef __CYOARGUMENTS_REQUIRED_HPP
#define __CYOARGUMENTS_REQUIRED_HPP

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
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

        ////////////////////////////////

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
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_REQUIRED_HPP
