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

#include <iostream>

using namespace cyoarguments;

///////////////////////////////////////////////////////////////////////////////

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

        std::string prefix;
        args.AddOption('P', "prefix", "description for prefix", prefix);

        bool verbose = false;
        args.AddOption('\x0', "verbose", "description for verbose", verbose);

        std::string filename;
        args.AddRequired("filename", "description for filename", filename);

        int number = 0;
        args.AddRequired("number", "description for number", number);

        args.Help();

#if 0
        args.Process(argc, argv);
#else
        argc, argv; //TEMP
        int fakeargc = 7;
        char* fakeargv[] = { "exe_pathname", "-a", "--verbose", "-ij2", "-P=cyo", "results.txt", "9" };
        if (!args.Process(fakeargc, fakeargv))
            return 1;
#endif

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
