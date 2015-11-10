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

using namespace cyoarguments;

///////////////////////////////////////////////////////////////////////////////

namespace
{
    const char* indent = "  ";

    template<typename T>
    void output(const char* name, const T& value)
    {
        std::cout << indent << name << " = " << value << std::endl;
    }

    template<typename T>
    void output(const char* name, const Argument<T>& value)
    {
        std::cout << indent << name << " = " << (value() ? value.get() : "(blank)") << std::endl;
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

        bool all = false;
        args.AddOption('a', "all", "description for all", all);

        bool ignoreErrors = false;
        args.AddOption('i', "description for ignoreErrors", ignoreErrors);

        int jobs = 1;
        args.AddOption('j', "jobs", "description for jobs", jobs);

        int threads = 1;
        args.AddOption('t', "threads", "description for threads", threads);

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

        std::string suffix;
        args.AddOption('s', "suffix", "description for suffix", suffix);

        bool verbose = false;
        args.AddOption("verbose", "description for verbose", verbose);

        Argument<bool> quiet;
        args.AddOption('q', "quiet", "description for quiet", quiet);

        std::string filename;
        args.AddRequired("filename", "description for filename", filename);

        int count = 0;
        args.AddRequired("count", "description for count", count);

#if 0
        bool temp1;
        args.AddRequired("temp1", "description for temp1", temp1);
        Argument<bool> temp2;
        args.AddRequired("temp2", "description for temp2", temp2);
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
        //const char* testargv[] = { "exe_pathname", "-ij20a", "-a", "--verbose", "-t5p=LV426", "-n=bah", "--block=five", "--suffix", "ine", "--ratio=9.8", "--angle=5.6", "-q", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "/ij20a", "/a", "/verbose", "/t5p=LV426", "/n=bah", "/block=five", "/suffix", "ine", "/ratio=9.8", "/angle=5.6", "/q", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "/IJ20A", "/A", "/VERBOSE", "/T5P=LV426", "/N=bah", "/BLOCK=five", "/SUFFIX", "ine", "/RATIO=9.8", "/ANGLE=5.6", "/Q", "results.txt", "9" };
        const char* testargv[] = { "exe_pathname", "/IJ20A", "/A", "/VERBOSE", "/T5P=LV426", "/N=bah", "/BLOCK=five", "/SUFFIX", "ine", "/RATIO=9.8", "/ANGLE=5.6", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "/IJ20A", "/A", "/VERBOSE", "/T5P=LV426", "/N=bah", "/BLOCK=five", "/SUFFIX", "ine", "/RATIO=9.8", "/ANGLE=5.6" };
        //const char* testargv[] = { "exe_pathname", "results.txt" };
        //const char* testargv[] = { "exe_pathname", "results.txt", "9" };
        //const char* testargv[] = { "exe_pathname", "results.txt", "9", "extra" };
        int testargc = (sizeof(testargv) / sizeof(testargv[0]));
        if (!args.Process(testargc, (char**)testargv))
            return 1;
#endif

        std::cout << "RESULTS:" << std::endl;
        output("all", all);
        output("ignoreErrors", ignoreErrors);
        output("jobs", jobs);
        output("threads", threads);
        output("name", name);
        output("block", block);
        output("ratio", ratio);
        output("angle", angle);
        output("prefix", prefix);
        output("suffix", suffix);
        output("verbose", verbose);
        output("quiet", quiet);
        output("filename", filename);
        output("count", count);
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
