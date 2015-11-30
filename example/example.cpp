/*
[CyoArguments] example.cpp

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

#include <iostream>
#include <list>

using namespace cyoarguments;

///////////////////////////////////////////////////////////////////////////////

namespace
{
    const char* indent = "  ";

    template<typename T>
    void output(const char* name, const T& value)
    {
        std::cout << indent << indent << name << " = " << value << std::endl;
    }

    template<typename T>
    void output(const char* name, const std::list<T>& value)
    {
        int index = 0;
        for (auto i : value)
            std::cout << indent << indent << name << "_" << ++index << " = " << i << std::endl;
    }

    template<typename T>
    void output(const char* name, const Argument<T>& value)
    {
        if (value())
            std::cout << indent << indent << name << " = " << value.get() << std::endl;
        else
            std::cout << indent << indent << name << " = (not specified)" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Arguments args;

        args.SetName("example");

        args.SetVersion("Version 1.0.0");

        args.SetHeader("CyoArguments example, version 1.0.0");

        args.SetFooter("See https://github.com/calzakk/CyoArguments");

        // Options...

        bool all = false;
        args.AddOption('a', "all", "description for all", all);

        std::list<int> lengths;
        args.AddOption("length", "description for length", lengths);

        int jobs = 1;
        args.AddOption('j', "jobs", "description for jobs", jobs);

        Argument<std::string> name;
        args.AddOption('n', "name", "description for name", name);

        std::string block;
        args.AddOption('b', "block", "description for block", block);

        double ratio = 0;
        args.AddOption("ratio", "description for ratio", ratio);

        float angle = 0;
        args.AddOption("angle", "description for angle", angle);

        std::string prefix;
        args.AddOption('p', "prefix", "description for prefix", prefix);

        bool verbose = false;
        args.AddOption("verbose", "description for verbose", verbose);

        Argument<bool> quiet;
        args.AddOption('q', "quiet", "description for quiet", quiet);

        // Required arguments...

        std::string filename;
        args.AddRequired("filename", "description for filename", filename);

        int count = 0;
        args.AddRequired("count", "description for count", count);

        // List argument...

        std::list<std::string> words;
        args.AddList("word", "description for word", words);

#if 0
        // Disallowed...

        bool temp1;
        args.AddRequired("temp1", "description for temp1", temp1); //bad type for required (bool)

        Argument<bool> temp2;
        args.AddRequired("temp2", "description for temp2", temp2); //bad type for required (Argument<bool>)

        std::list<int> temp3;
        args.AddRequired("temp3", "description for temp3", temp3); //bad type for required (container)

        std::list<std::string> temp4;
        args.AddRequired("temp4", "description for temp4", temp4); //bad type for required (container)
#endif

#if 0
        std::list<std::string> temp5;
        args.AddList("temp5", "description for temp5", temp5); //can only have one list argument (words)
#endif

#if 1
        if (!args.Process(argc, argv))
            return 1;
#else
        argc; argv; //TEMP
        //const char* testargv[] = { "exe_pathname" };
        //const char* testargv[] = { "exe_pathname", "/?" };
        //const char* testargv[] = { "exe_pathname", "-?" };
        //const char* testargv[] = { "exe_pathname", "--help" };
        //const char* testargv[] = { "exe_pathname", "-aj20", "--verbose", "-p=LV426", "-n=bah", "--block=five", "--ratio=9.8", "--angle=5.6", "-q", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "/aj20", "/verbose", "/p=LV426", "/n=bah", "/block=five", "/ratio=9.8", "/angle=5.6", "/q", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "/aJ20", "/VERBOSE", "/P=LV426", "/N=bah", "/BLOCK=five", "/RATIO=9.8", "/ANGLE=5.6", "/Q", "results.txt", "9" };
        const char* testargv[] = { "exe_pathname", "/AJ20", "/VERBOSE", "/P=LV426", "/N=bah", "/BLOCK=five", "/RATIO=9.8", "/ANGLE=5.6", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "/AJ20", "/VERBOSE", "/P=LV426", "/N=bah", "/BLOCK=five", "/RATIO=9.8", "/ANGLE=5.6" };
        //const char* testargv[] = { "exe_pathname", "results.txt" };
        //const char* testargv[] = { "exe_pathname", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "results.txt", "9", "extra" };
        int testargc = (sizeof(testargv) / sizeof(testargv[0]));
        if (!args.Process(testargc, (char**)testargv))
            return 1;
#endif

        std::cout << "RESULTS:" << std::endl;
        std::cout << indent << "options:" << std::endl;
        output("all", all);
        output("lengths", lengths);
        output("jobs", jobs);
        output("name", name);
        output("block", block);
        output("ratio", ratio);
        output("angle", angle);
        output("prefix", prefix);
        output("verbose", verbose);
        output("quiet", quiet);
        std::cout << indent << "required:" << std::endl;
        output("filename", filename);
        output("count", count);
        std::cout << indent << "list:" << std::endl;
        output("word", words);
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
