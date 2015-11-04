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

#define CHECK_THROW(expected, func) \
    try \
    { \
        func; \
        throw std::runtime_error("Exception not caught"); \
    } \
    catch (const std::exception& ex) \
    { \
        if (std::string(expected) != ex.what()) \
        { \
            std::cout << "\nline " << __LINE__ << ": exception=" << ex.what() << " expected=" << expected; \
            throw std::runtime_error("Exception not caught"); \
        } \
    } \
    catch (...) \
    { \
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

    // bool
    namespace
    {
        // Required
        namespace
        {
            TEST(RequiredBool, 1, Test)
            {
                bool first = false;
                CHECK_THROW("Disallowed type of required argument: first", arguments.AddRequired("first", "description", first));
            }
        }

        // Letter
        namespace
        {
            class LetterBoolBase : public Test
            {
            public:
                LetterBoolBase()
                {
                    arguments.AddOption('a', "description", a);
                    arguments.AddOption('b', "description", b);
                }
                bool a = false;
                bool b = false;
            };

            TEST(LetterBool, 1, LetterBoolBase)
            {
                ProcessArgs({ "-a1" }, false, "Invalid argument: -a1");
                CHECK(true, a);
                CHECK(false, b);
            }

            TEST(LetterBool, 2, LetterBoolBase)
            {
                ProcessArgs({ "-a=2" }, false, "Invalid argument: -a=2");
                CHECK(false, a);
                CHECK(false, b);
            }

            TEST(LetterBool, 3, LetterBoolBase)
            {
                ProcessArgs({ "-a", "3" }, false, "Invalid argument: 3");
                CHECK(true, a);
                CHECK(false, b);
            }

            TEST(LetterBool, 4, LetterBoolBase)
            {
                ProcessArgs({ "-a=", "4" }, false, "Invalid argument: -a=");
                CHECK(false, a);
                CHECK(false, b);
            }

            TEST(LetterBool, 5, LetterBoolBase)
            {
                ProcessArgs({ "-a" }, true, "");
                CHECK(true, a);
                CHECK(false, b);
            }

            TEST(LetterBool, 6, LetterBoolBase)
            {
                ProcessArgs({ "-a=" }, false, "Invalid argument: -a=");
                CHECK(false, a);
                CHECK(false, b);
            }
        }

        // Letters
        namespace
        {
            class LettersBoolBase : public Test
            {
            public:
                LettersBoolBase()
                {
                    arguments.AddOption('x', "description", x);
                    arguments.AddOption('y', "description", y);
                }
                bool x = false;
                bool y = false;
            };

            TEST(LettersBool, 1, LettersBoolBase)
            {
                ProcessArgs({ "-x1y2" }, false, "Invalid argument: -x1y2");
                CHECK(true, x);
                CHECK(false, y);
            }

            TEST(LettersBool, 2, LettersBoolBase)
            {
                ProcessArgs({ "-x3", "-y4" }, false, "Invalid argument: -x3");
                CHECK(true, x);
                CHECK(false, y);
            }

            TEST(LettersBool, 3, LettersBoolBase)
            {
                ProcessArgs({ "-x", "5", "-y", "6" }, false, "Invalid argument: 5");
                CHECK(true, x);
                CHECK(false, y);
            }

            TEST(LettersBool, 4, LettersBoolBase)
            {
                ProcessArgs({ "-xy" }, true, "");
                CHECK(true, x);
                CHECK(true, y);
            }
        }

        // Word
        namespace
        {
            class WordBoolBase : public Test
            {
            public:
                WordBoolBase()
                {
                    arguments.AddOption("num", "description", num);
                }
                bool num = false;
            };

            TEST(WordBool, 1, WordBoolBase)
            {
                ProcessArgs({ "--num1" }, false, "Invalid argument: --num1");
                CHECK(false, num);
            }

            TEST(WordBool, 2, WordBoolBase)
            {
                ProcessArgs({ "--num=2" }, false, "Invalid argument: --num=2");
                CHECK(false, num);
            }

            TEST(WordBool, 3, WordBoolBase)
            {
                ProcessArgs({ "--num", "3" }, false, "Invalid argument: 3");
                CHECK(true, num);
            }

            TEST(WordBool, 4, WordBoolBase)
            {
                ProcessArgs({ "--num=", "4" }, false, "Invalid argument: --num=");
                CHECK(false, num);
            }

            TEST(WordBool, 5, WordBoolBase)
            {
                ProcessArgs({ "--num" }, true, "");
                CHECK(true, num);
            }

            TEST(WordBool, 6, WordBoolBase)
            {
                ProcessArgs({ "--num=" }, false, "Invalid argument: --num=");
                CHECK(false, num);
            }
        }

        // Words
        namespace
        {
            class WordsBoolBase : public Test
            {
            public:
                WordsBoolBase()
                {
                    arguments.AddOption("num", "description", num);
                    arguments.AddOption("val", "description", val);
                }
                bool num = false;
                bool val = false;
            };

            TEST(WordsBool, 1, WordsBoolBase)
            {
                ProcessArgs({ "--num1", "--val2" }, false, "Invalid argument: --num1");
                CHECK(false, num);
                CHECK(false, val);
            }

            TEST(WordsBool, 2, WordsBoolBase)
            {
                ProcessArgs({ "--num=3", "--val=4" }, false, "Invalid argument: --num=3");
                CHECK(false, num);
                CHECK(false, val);
            }

            TEST(WordsBool, 3, WordsBoolBase)
            {
                ProcessArgs({ "--num", "5", "--val", "6" }, false, "Invalid argument: 5");
                CHECK(true, num);
                CHECK(false, val);
            }

            TEST(WordsBool, 4, WordsBoolBase)
            {
                ProcessArgs({ "--num=", "7", "--val=", "8" }, false, "Invalid argument: --num=");
                CHECK(false, num);
                CHECK(false, val);
            }

            TEST(WordsBool, 5, WordsBoolBase)
            {
                ProcessArgs({ "--num", "--val" }, true, "");
                CHECK(true, num);
                CHECK(true, val);
            }

            TEST(WordsBool, 6, WordsBoolBase)
            {
                ProcessArgs({ "--num=", "--val=" }, false, "Invalid argument: --num=");
                CHECK(false, num);
                CHECK(false, val);
            }

            TEST(WordsBool, 7, WordsBoolBase)
            {
                ProcessArgs({ "--num9val10" }, false, "Invalid argument: --num9val10");
                CHECK(false, num);
                CHECK(false, val);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    // int
    namespace
    {
        // Required
        namespace
        {
            class RequiredIntBase : public Test
            {
            public:
                RequiredIntBase()
                {
                    arguments.AddRequired("first", "description", first);
                    arguments.AddRequired("second", "description", second);
                }
                int first = 0;
                int second = 0;
            };

            TEST(RequiredInt, 1, RequiredIntBase)
            {
                ProcessArgs({}, false, "Missing argument: first");
            }

            TEST(RequiredInt, 2, RequiredIntBase)
            {
                ProcessArgs({ "123" }, false, "Missing argument: second");
            }

            TEST(RequiredInt, 3, RequiredIntBase)
            {
                ProcessArgs({ "123", "456" }, true, "");
            }

            TEST(RequiredInt, 4, RequiredIntBase)
            {
                ProcessArgs({ "123", "456", "789" }, false, "Invalid argument: 789");
            }
        }

        // Letter
        namespace
        {
            class LetterIntBase : public Test
            {
            public:
                LetterIntBase()
                {
                    arguments.AddOption('a', "description", a);
                    arguments.AddOption('b', "description", b);
                }
                int a = 0;
                int b = 0;
            };

            TEST(LetterInt, 1, LetterIntBase)
            {
                ProcessArgs({ "-a1" }, true, "");
                CHECK(1, a);
                CHECK(0, b);
            }

            TEST(LetterInt, 2, LetterIntBase)
            {
                ProcessArgs({ "-a=2" }, true, "");
                CHECK(2, a);
                CHECK(0, b);
            }

            TEST(LetterInt, 3, LetterIntBase)
            {
                ProcessArgs({ "-a", "3" }, true, "");
                CHECK(3, a);
                CHECK(0, b);
            }

            TEST(LetterInt, 4, LetterIntBase)
            {
                ProcessArgs({ "-a=", "4" }, true, "");
                CHECK(4, a);
                CHECK(0, b);
            }

            TEST(LetterInt, 5, LetterIntBase)
            {
                ProcessArgs({ "-a" }, false, "Invalid argument: -a");
                CHECK(0, a);
                CHECK(0, b);
            }

            TEST(LetterInt, 6, LetterIntBase)
            {
                ProcessArgs({ "-a=" }, false, "Invalid argument: -a=");
                CHECK(0, a);
                CHECK(0, b);
            }
        }

        // Letters
        namespace
        {
            class LettersIntBase : public Test
            {
            public:
                LettersIntBase()
                {
                    arguments.AddOption('x', "description", x);
                    arguments.AddOption('y', "description", y);
                }
                int x = 0;
                int y = 0;
            };

            TEST(LettersInt, 1, LettersIntBase)
            {
                ProcessArgs({ "-x1y2" }, true, "");
                CHECK(1, x);
                CHECK(2, y);
            }

            TEST(LettersInt, 2, LettersIntBase)
            {
                ProcessArgs({ "-x3", "-y4" }, true, "");
                CHECK(3, x);
                CHECK(4, y);
            }

            TEST(LettersInt, 3, LettersIntBase)
            {
                ProcessArgs({ "-x", "5", "-y", "6" }, true, "");
                CHECK(5, x);
                CHECK(6, y);
            }

            TEST(LettersInt, 4, LettersIntBase)
            {
                ProcessArgs({ "-xy" }, false, "Invalid argument: -xy");
                CHECK(0, x);
                CHECK(0, y);
            }
        }

        // Word
        namespace
        {
            class WordIntBase : public Test
            {
            public:
                WordIntBase()
                {
                    arguments.AddOption("num", "description", num);
                }
                int num = 0;
            };

            TEST(WordInt, 1, WordIntBase)
            {
                ProcessArgs({ "--num1" }, true, "");
                CHECK(1, num);
            }

            TEST(WordInt, 2, WordIntBase)
            {
                ProcessArgs({ "--num=2" }, true, "");
                CHECK(2, num);
            }

            TEST(WordInt, 3, WordIntBase)
            {
                ProcessArgs({ "--num", "3" }, true, "");
                CHECK(3, num);
            }

            TEST(WordInt, 4, WordIntBase)
            {
                ProcessArgs({ "--num=", "4" }, true, "");
                CHECK(4, num);
            }

            TEST(WordInt, 5, WordIntBase)
            {
                ProcessArgs({ "--num" }, false, "Invalid argument: --num");
                CHECK(0, num);
            }

            TEST(WordInt, 6, WordIntBase)
            {
                ProcessArgs({ "--num=" }, false, "Invalid argument: --num=");
                CHECK(0, num);
            }
        }

        // Words
        namespace
        {
            class WordsIntBase : public Test
            {
            public:
                WordsIntBase()
                {
                    arguments.AddOption("num", "description", num);
                    arguments.AddOption("val", "description", val);
                }
                int num = 0;
                int val = 0;
            };

            TEST(WordsInt, 1, WordsIntBase)
            {
                ProcessArgs({ "--num1", "--val2" }, true, "");
                CHECK(1, num);
                CHECK(2, val);
            }

            TEST(WordsInt, 2, WordsIntBase)
            {
                ProcessArgs({ "--num=3", "--val=4" }, true, "");
                CHECK(3, num);
                CHECK(4, val);
            }

            TEST(WordsInt, 3, WordsIntBase)
            {
                ProcessArgs({ "--num", "5", "--val", "6" }, true, "");
                CHECK(5, num);
                CHECK(6, val);
            }

            TEST(WordsInt, 4, WordsIntBase)
            {
                ProcessArgs({ "--num=", "7", "--val=", "8" }, true, "");
                CHECK(7, num);
                CHECK(8, val);
            }

            TEST(WordsInt, 5, WordsIntBase)
            {
                ProcessArgs({ "--num", "--val" }, false, "Invalid argument: --num");
                CHECK(0, num);
                CHECK(0, val);
            }

            TEST(WordsInt, 6, WordsIntBase)
            {
                ProcessArgs({ "--num=", "--val=" }, false, "Invalid argument: --num=");
                CHECK(0, num);
                CHECK(0, val);
            }

            TEST(WordsInt, 7, WordsIntBase)
            {
                ProcessArgs({ "--num9val10" }, false, "Invalid argument: --num9val10");
                CHECK(0, num);
                CHECK(0, val);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    // std::string
    namespace
    {
        // Required
        namespace
        {
            class RequiredStringBase : public Test
            {
            public:
                RequiredStringBase()
                {
                    arguments.AddRequired("first", "description", first);
                    arguments.AddRequired("second", "description", second);
                }
                std::string first;
                std::string second;
            };

            TEST(RequiredString, 1, RequiredStringBase)
            {
                ProcessArgs({}, false, "Missing argument: first");
            }

            TEST(RequiredString, 2, RequiredStringBase)
            {
                ProcessArgs({ "alpha" }, false, "Missing argument: second");
            }

            TEST(RequiredString, 3, RequiredStringBase)
            {
                ProcessArgs({ "alpha", "beta" }, true, "");
            }

            TEST(RequiredString, 4, RequiredStringBase)
            {
                ProcessArgs({ "alpha", "beta", "gamma" }, false, "Invalid argument: gamma");
            }
        }

        // Letter
        namespace
        {
            class LetterStringBase : public Test
            {
            public:
                LetterStringBase()
                {
                    arguments.AddOption('a', "description", a);
                    arguments.AddOption('b', "description", b);
                }
                std::string a;
                std::string b;
            };

            TEST(LetterString, 1, LetterStringBase)
            {
                ProcessArgs({ "-a1" }, false, "Invalid argument: -a1");
                CHECK("", a);
                CHECK("", b);
            }

            TEST(LetterString, 2, LetterStringBase)
            {
                ProcessArgs({ "-a=2" }, true, "");
                CHECK("2", a);
                CHECK("", b);
            }

            TEST(LetterString, 3, LetterStringBase)
            {
                ProcessArgs({ "-a", "3" }, true, "");
                CHECK("3", a);
                CHECK("", b);
            }

            TEST(LetterString, 4, LetterStringBase)
            {
                ProcessArgs({ "-a=", "4" }, true, "");
                CHECK("4", a);
                CHECK("", b);
            }

            TEST(LetterString, 5, LetterStringBase)
            {
                ProcessArgs({ "-a" }, false, "Invalid argument: -a");
                CHECK("", a);
                CHECK("", b);
            }

            TEST(LetterString, 6, LetterStringBase)
            {
                ProcessArgs({ "-a=" }, false, "Invalid argument: -a=");
                CHECK("", a);
                CHECK("", b);
            }
        }

        // Letters
        namespace
        {
            class LettersStringBase : public Test
            {
            public:
                LettersStringBase()
                {
                    arguments.AddOption('x', "description", x);
                    arguments.AddOption('y', "description", y);
                }
                std::string x;
                std::string y;
            };

            TEST(LettersString, 1, LettersStringBase)
            {
                ProcessArgs({ "-x1y2" }, false, "Invalid argument: -x1y2");
                CHECK("", x);
                CHECK("", y);
            }

            TEST(LettersString, 2, LettersStringBase)
            {
                ProcessArgs({ "-x3", "-y4" }, false, "Invalid argument: -x3");
                CHECK("", x);
                CHECK("", y);
            }

            TEST(LettersString, 3, LettersStringBase)
            {
                ProcessArgs({ "-x", "5", "-y", "6" }, true, "");
                CHECK("5", x);
                CHECK("6", y);
            }

            TEST(LettersString, 4, LettersStringBase)
            {
                ProcessArgs({ "-xy" }, false, "Invalid argument: -xy");
                CHECK("", x);
                CHECK("", y);
            }
        }

        // Word
        namespace
        {
            class WordStringBase : public Test
            {
            public:
                WordStringBase()
                {
                    arguments.AddOption("val", "description", val);
                }
                std::string val;
            };

            TEST(WordString, 1, WordStringBase)
            {
                ProcessArgs({ "--val1" }, false, "Invalid argument: --val1");
                CHECK("", val);
            }

            TEST(WordString, 2, WordStringBase)
            {
                ProcessArgs({ "--val=2" }, true, "");
                CHECK("2", val);
            }

            TEST(WordString, 3, WordStringBase)
            {
                ProcessArgs({ "--val", "3" }, true, "");
                CHECK("3", val);
            }

            TEST(WordString, 4, WordStringBase)
            {
                ProcessArgs({ "--val=", "4" }, true, "");
                CHECK("4", val);
            }

            TEST(WordString, 5, WordStringBase)
            {
                ProcessArgs({ "--val" }, false, "Invalid argument: --val");
                CHECK("", val);
            }

            TEST(WordString, 6, WordStringBase)
            {
                ProcessArgs({ "--val=" }, false, "Invalid argument: --val=");
                CHECK("", val);
            }
        }

        // Words
        namespace
        {
            class WordsStringBase : public Test
            {
            public:
                WordsStringBase()
                {
                    arguments.AddOption("name", "description", name);
                    arguments.AddOption("val", "description", val);
                }
                std::string name;
                std::string val;
            };

            TEST(WordsString, 1, WordsStringBase)
            {
                ProcessArgs({ "--name1", "--val2" }, false, "Invalid argument: --name1");
                CHECK("", name);
                CHECK("", val);
            }

            TEST(WordsString, 2, WordsStringBase)
            {
                ProcessArgs({ "--name=3", "--val=4" }, true, "");
                CHECK("3", name);
                CHECK("4", val);
            }

            TEST(WordsString, 3, WordsStringBase)
            {
                ProcessArgs({ "--name", "5", "--val", "6" }, true, "");
                CHECK("5", name);
                CHECK("6", val);
            }

            TEST(WordsString, 4, WordsStringBase)
            {
                ProcessArgs({ "--name=", "7", "--val=", "8" }, true, "");
                CHECK("7", name);
                CHECK("8", val);
            }

            TEST(WordsString, 5, WordsStringBase)
            {
                ProcessArgs({ "--name", "--val" }, false, "Invalid argument: --name");
                CHECK("", name);
                CHECK("", val);
            }

            TEST(WordsString, 6, WordsStringBase)
            {
                ProcessArgs({ "--name=", "--val=" }, false, "Invalid argument: --name=");
                CHECK("", name);
                CHECK("", val);
            }

            TEST(WordsString, 7, WordsStringBase)
            {
                ProcessArgs({ "--name9val10" }, false, "Invalid argument: --name9val10");
                CHECK("", name);
                CHECK("", val);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

int main()
{
    TestRunner::instance().RunAllTests();
    return 0;
}
