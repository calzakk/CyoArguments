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
#include <stdexcept>

///////////////////////////////////////////////////////////////////////////////

namespace cyoarguments
{
    namespace detail
    {
        #ifndef UNREFERENCED_PARAMETER
        #   define UNREFERENCED_PARAMETER(p) (void)(p)
        #endif

        class OptionBase
        {
        public:
            virtual void Output() const = 0;
            virtual void Process(const char* arg) const = 0;

        protected:
            template<typename T>
            void GetValue(const char* arg, T& target) const
            {
                UNREFERENCED_PARAMETER(arg);
                UNREFERENCED_PARAMETER(target);
                throw std::logic_error("Unsupported argument type");
            }
            template<>
            void GetValue(const char* arg, bool& target) const
            {
                UNREFERENCED_PARAMETER(arg);
                target = true;
            }
            template<>
            void GetValue(const char* arg, int& target) const
            {
                target = std::atoi(arg);
            }
            template<>
            void GetValue(const char* arg, std::string& target) const
            {
                target = arg;
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
                if (letter_ != '\x0')
                    std::cout << " -" << letter_;
                if (word_ != nullptr)
                    std::cout << " --" << word_;
                std::cout << "  " << description_;
                std::cout << std::endl;
            }
            void Process(const char* arg) const override
            {
                arg; //TEMP

                //todo
            }
        private:
            char letter_;
            const char* word_;
            const char* description_;
            T* target_;
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
            void Process(const char* arg) const override
            {
                GetValue(arg, *target_);
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
        void AddRequired(const char* name, const char* description, T& target)
        {
            required_.push_back(std::make_unique<detail::Required<T>>(name, description, target));
        }

        void Help() const
        {
            for (auto& required : required_)
                required->Output();
            for (auto& option : options_)
                option->Output();
        }

        bool Process(int argc, char* argv[])
        {
            auto nextRequired = required_.begin();

            for (int i = 1; i < argc; ++i)
            {
                std::cout << argv[i] << std::endl; //TEMP

                bool ok;
#ifdef _MSC_VER
                if (argv[i][0] == '-' || argv[i][0] == '/')
#else
                if (argv[i][0] == '-')
#endif
                    ok = ProcessOption(argv[i], i < argc - 1 ? argv[i + 1] : nullptr);
                else
                    ok = ProcessRequired(argv[i], nextRequired);

                if (!ok)
                {
                    std::cerr << "Invalid argument: " << argv[i] << std::endl;
                    return false;
                }
            }

            return true;
        }

    private:
        using OptionsList = std::list<detail::OptionPtr>;
        OptionsList options_;
        OptionsList required_;

        bool ProcessOption(const char* arg, const char* next) const
        {
            arg; next;
            //TODO

            return true;
        }

        bool ProcessRequired(const char* arg, OptionsList::const_iterator& it) const
        {
            if (it == required_.end())
                return false;

            (*it)->Process(arg);

            ++it;

            return true;
        }
    };
}

///////////////////////////////////////////////////////////////////////////////

#endif //__CYOARGUMENTS_HPP
