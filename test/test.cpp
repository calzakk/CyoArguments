/*
[CyoArguments] test.cpp

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

#include "cyoarguments.hpp"

#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

using namespace cyoarguments;

///////////////////////////////////////////////////////////////////////////////

#define CHECK(expected, actual) \
    if (expected != actual) \
    { \
        std::cout << "\nline " << __LINE__ << ": value=" << actual << " expected=" << expected; \
        throw std::runtime_error("Unexpected value"); \
    }

#define TEST(name, index, setup) \
    class Test##name##index : public setup \
        { \
    public: \
        static Test##name##index instance; \
        Test##name##index() \
        { \
            TestRunner::instance().AddTest(this); \
        } \
    private: \
        std::string getNameImpl() const override \
        { \
            return "Test" ## #name ## #index; \
        } \
        void RunImpl() override; \
    }; \
    Test##name##index Test##name##index::instance; \
    void Test##name##index::RunImpl()

namespace
{
    class Test
    {
    public:
        std::string getName() const { return getNameImpl(); }
        void Run() { RunImpl(); }

    protected:
        Arguments arguments;

        void ProcessArgs(const std::vector<const char*>& args, bool expectedResult, const char* expectedError)
        {
            std::vector<const char*> fullArgs{ "exe_pathname" };
            std::copy(begin(args), end(args), std::back_inserter(fullArgs));

            std::string actualError;
            bool actualResult = arguments.Process(fullArgs.size(), (char**)&fullArgs[0], actualError);

            if (actualResult != expectedResult)
            {
                std::cout << "\nresult=" << actualResult << " expected=" << expectedResult;
                throw std::runtime_error("Unexpected result");
            }

            if (actualError.compare(expectedError) != 0)
            {
                std::cout << "\nerror=\"" << actualError << "\" expected=\"" << expectedError << "\"";
                throw std::runtime_error("Unexpected error");
            }
        }

    private:
        virtual std::string getNameImpl() const = 0;
        virtual void RunImpl() = 0;
    };

    class TestRunner
    {
    public:
        static TestRunner& instance()
        {
            static TestRunner testRunner;
            return testRunner;
        }

        void AddTest(Test* test)
        {
            tests_.push_back(test);
        }

        void RunAllTests()
        {
            int testNum = 0;
            int failures = 0;
            for (auto& test : tests_)
            {
                try
                {
                    std::cout << "TEST " << ++testNum << ": " << test->getName() << ":";
                    test->Run();
                    std::cout << " success" << std::endl;
                }
                catch (...)
                {
                    ++failures;
                    std::cout << "\n*** FAILED ***" << std::endl;
                }
            }
            std::cout << '\n' << failures << " failure(s)" << std::endl;
        }

    private:
        std::list<Test*> tests_;
    };

    ///////////////////////////////////////////////////////////////////////////

    // Required
    namespace
    {
        class RequiredBase : public Test
        {
        public:
            RequiredBase()
            {
                arguments.AddRequired("first", "description", first);
                arguments.AddRequired("second", "description", second);
            }
            std::string first, second;
        };

        TEST(Required, 1, RequiredBase)
        {
            ProcessArgs({}, false, "Missing argument: first");
        }

        TEST(Required, 2, RequiredBase)
        {
            ProcessArgs({ "alpha" }, false, "Missing argument: second");
        }

        TEST(Required, 3, RequiredBase)
        {
            ProcessArgs({ "alpha", "beta" }, true, "");
        }

        TEST(Required, 4, RequiredBase)
        {
            ProcessArgs({ "alpha", "beta", "gamma" }, false, "Invalid argument: gamma");
        }
    }

    // Letter
    namespace
    {
        class LetterBase : public Test
        {
        public:
            LetterBase()
            {
                arguments.AddOption('a', "description", a);
                arguments.AddOption('b', "description", b);
            }
            int a = 0;
            int b = 0;
        };

        TEST(Letter, 1, LetterBase)
        {
            ProcessArgs({ "-a1" }, true, "");
            CHECK(1, a);
            CHECK(0, b);
        }

        TEST(Letter, 2, LetterBase)
        {
            ProcessArgs({ "-a=2" }, true, "");
            CHECK(2, a);
            CHECK(0, b);
        }

        TEST(Letter, 3, LetterBase)
        {
            ProcessArgs({ "-a", "3" }, true, "");
            CHECK(3, a);
            CHECK(0, b);
        }

        TEST(Letter, 4, LetterBase)
        {
            ProcessArgs({ "-a=", "4" }, true, "");
            CHECK(4, a);
            CHECK(0, b);
        }

        TEST(Letter, 5, LetterBase)
        {
            ProcessArgs({ "-a" }, false, "Invalid argument: -a");
            CHECK(0, a);
            CHECK(0, b);
        }

        TEST(Letter, 6, LetterBase)
        {
            ProcessArgs({ "-a=" }, false, "Invalid argument: -a=");
            CHECK(0, a);
            CHECK(0, b);
        }
    }

    // Letters
    namespace
    {
        class LettersBase : public Test
        {
        public:
            LettersBase()
            {
                arguments.AddOption('x', "description", x);
                arguments.AddOption('y', "description", y);
            }
            int x = 0;
            int y = 0;
        };

        TEST(Letters, 1, LettersBase)
        {
            ProcessArgs({ "-x1y2" }, true, "");
            CHECK(1, x);
            CHECK(2, y);
        }

        TEST(Letters, 2, LettersBase)
        {
            ProcessArgs({ "-x3", "-y4" }, true, "");
            CHECK(3, x);
            CHECK(4, y);
        }

        TEST(Letters, 3, LettersBase)
        {
            ProcessArgs({ "-x", "5", "-y", "6" }, true, "");
            CHECK(5, x);
            CHECK(6, y);
        }

        TEST(Letters, 4, LettersBase)
        {
            ProcessArgs({ "-xy" }, false, "Invalid argument: -xy");
            CHECK(0, x);
            CHECK(0, y);
        }
    }

    // Word
    namespace
    {
        class WordBase : public Test
        {
        public:
            WordBase()
            {
                arguments.AddOption("num", "description", num);
            }
            int num = 0;
        };

        TEST(Word, 1, WordBase)
        {
            ProcessArgs({ "--num1" }, true, "");
            CHECK(1, num);
        }

        TEST(Word, 2, WordBase)
        {
            ProcessArgs({ "--num=2" }, true, "");
            CHECK(2, num);
        }

        TEST(Word, 3, WordBase)
        {
            ProcessArgs({ "--num", "3" }, true, "");
            CHECK(3, num);
        }

        TEST(Word, 4, WordBase)
        {
            ProcessArgs({ "--num=", "4" }, true, "");
            CHECK(4, num);
        }

        TEST(Word, 5, WordBase)
        {
            ProcessArgs({ "--num" }, false, "Invalid argument: --num");
            CHECK(0, num);
        }

        TEST(Word, 6, WordBase)
        {
            ProcessArgs({ "--num=" }, false, "Invalid argument: --num=");
            CHECK(0, num);
        }
    }

    // Words
    namespace
    {
        class WordsBase : public Test
        {
        public:
            WordsBase()
            {
                arguments.AddOption("num", "description", num);
                arguments.AddOption("val", "description", val);
            }
            int num = 0;
            int val = 0;
        };

        TEST(Words, 1, WordsBase)
        {
            ProcessArgs({ "--num1", "--val2" }, true, "");
            CHECK(1, num);
            CHECK(2, val);
        }

        TEST(Words, 2, WordsBase)
        {
            ProcessArgs({ "--num=3", "--val=4" }, true, "");
            CHECK(3, num);
            CHECK(4, val);
        }

        TEST(Words, 3, WordsBase)
        {
            ProcessArgs({ "--num", "5", "--val", "6" }, true, "");
            CHECK(5, num);
            CHECK(6, val);
        }

        TEST(Words, 4, WordsBase)
        {
            ProcessArgs({ "--num=", "7", "--val=", "8" }, true, "");
            CHECK(7, num);
            CHECK(8, val);
        }

        TEST(Words, 5, WordsBase)
        {
            ProcessArgs({ "--num", "--val" }, false, "Invalid argument: --num");
            CHECK(0, num);
            CHECK(0, val);
        }

        TEST(Words, 6, WordsBase)
        {
            ProcessArgs({ "--num=", "--val=" }, false, "Invalid argument: --num=");
            CHECK(0, num);
            CHECK(0, val);
        }

        TEST(Words, 7, WordsBase)
        {
            ProcessArgs({ "--num9val10" }, false, "Invalid argument: --num9val10");
            CHECK(0, num);
            CHECK(0, val);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

int main()
{
    TestRunner::instance().RunAllTests();
    return 0;
}
