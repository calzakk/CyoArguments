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
    template<typename T>
    void output(const char* name, const Argument<T>& value)
    {
        std::cout << "  " << name << " = ";
        if (value())
            std::cout << value.get();
        else
            std::cout << "(blank)";
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Arguments args;

        bool all = false;
        args.AddOption('a', "all", "description for all", all);

        bool ignoreErrors = false;
        args.AddOption('i', nullptr, "description for ignoreErrors", ignoreErrors);

        int jobs = 1;
        args.AddOption('j', "jobs", "description for jobs", jobs);

        int threads = 1;
        args.AddOption('t', "threads", "description for threads", threads);

        Argument<std::string> name;
        args.AddOption('n', "name", "description for name", name);

        std::string block;
        args.AddOption('b', "block", "description for block", block);

        double ratio = 0;
        args.AddOption('\x0', "ratio", "description for ratio", ratio);

        float angle = 0;
        args.AddOption('\x0', "angle", "description for angle", angle);

        std::string prefix;
        args.AddOption('p', "prefix", "description for prefix", prefix);

        std::string suffix;
        args.AddOption('s', "suffix", "description for suffix", suffix);

        bool verbose = false;
        args.AddOption('\x0', "verbose", "description for verbose", verbose);

        Argument<bool> quiet;
        args.AddOption('q', "quiet", "description for quiet", quiet);

        std::string filename;
        args.AddRequired("filename", "description for filename", filename);

        int count = 0;
        args.AddRequired("count", "description for count", count);

        args.Help();

        argc; argv; //TEMP

        //char* testargv[] = { "exe_pathname", "-ij20a", "-a", "--verbose", "-t5p=LV426", "-nbah", "--blockfive", "--suffix", "ine", "--ratio=9.8", "--angle5.6", "-q", "results.txt", "9" };
        //char* testargv[] = { "exe_pathname", "/ij20a", "/a", "/verbose", "/t5p=LV426", "/nbah", "/blockfive", "/suffix", "ine", "/ratio=9.8", "/angle5.6", "/q", "results.txt", "9" };
        //char* testargv[] = { "exe_pathname", "/IJ20A", "/A", "/VERBOSE", "/T5P=LV426", "/Nbah", "/BLOCKfive", "/SUFFIX", "ine", "/RATIO=9.8", "/ANGLE5.6", "/Q", "results.txt", "9" };
        char* testargv[] = { "exe_pathname", "/IJ20A", "/A", "/VERBOSE", "/T5P=LV426", "/Nbah", "/BLOCKfive", "/SUFFIX", "ine", "/RATIO=9.8", "/ANGLE5.6", "results.txt", "9" };
        //char* testargv[] = { "exe_pathname", "results.txt" };
        //char* testargv[] = { "exe_pathname", "results.txt", "9" };
        //char* testargv[] = { "exe_pathname", "results.txt", "9", "extra" };
        int testargc = (sizeof(testargv) / sizeof(testargv[0]));
        if (!args.Process(testargc, testargv))
            return 1;

        std::cout << "\nRESULTS:" << std::endl;
        std::cout << "  all = " << all << std::endl;
        std::cout << "  ignoreErrors = " << ignoreErrors << std::endl;
        std::cout << "  jobs = " << jobs << std::endl;
        std::cout << "  threads = " << threads << std::endl;
        output("name", name);
        std::cout << "  block = " << block << std::endl;
        std::cout << "  ratio = " << ratio << std::endl;
        std::cout << "  angle = " << angle << std::endl;
        std::cout << "  prefix = " << prefix << std::endl;
        std::cout << "  suffix = " << suffix << std::endl;
        std::cout << "  verbose = " << verbose << std::endl;
        output("quiet", quiet);
        std::cout << "  filename = " << filename << std::endl;
        std::cout << "  count = " << count << std::endl;
        std::cout << std::endl;
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
