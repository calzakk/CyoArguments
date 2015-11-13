# CyoArguments

CyoArguments is a C++ library for processing command-line arguments.

From the outset the intention was an easy-to-use library, developed using modern C++ features (C++11 and C++14).

# Features

* Easy to use;
* Allows optional and/or required arguments;
* Supports dash (-v) and double-dash (--verbose) arguments on all platforms;
* Also supports case-insensitive slash (/v) arguments on Windows;
* Ability to add version information;
* Easily extensible to support additional data types;
* Implemented using header files only;
* Uses modern C++.

## Usage

CyoArguments is implemented using header files only, so doesn't need to be separately built prior to use. One header provides the interface, and several headers (in the 'details' subdirectory) provide implementation details that are only indirectly #included.

    #include "cyoarguments.hpp"
    using namespace cyoarguments;

### Arguments class

Instantiate an Arguments object:

    Arguments args;

Set the program's name (optional, only for displaying in the help):

    args.SetName("example"); 

Arguments come in three types: optional, required, and list. There must be at least one optional argument, one required argument, or one list argument.

Add any **optional arguments**:

    double scale = 1.0;
    args.AddOption( "scale", "scaling factor", scale );

    bool verbose = false;
    args.AddOption( 'v', "verbose", "output additional info", verbose );

Add any **required arguments**:

    std::string filename;
    args.AddRequired( "filename", "the file to process", filename );

    int threads = 0;
    args.AddRequired( "threads", "number of threads to use", threads );

Required arguments cannot be omitted when the program is executed, and must be specified in the order they were added to the object.

Add any **list argument**:

    std::list<std::int> numbers;
    args.AddList( "number", "initialization numbers", numbers );

A list follows the required arguments, and is entirely optional. Multiple lists are not permitted. 

Pass the command-line options to the object:

    args.Process(argc, argv);

The above variables will be assigned accordingly.

Please refer to the example program that demonstrates CyoArguments in action.

### Argument class

The above AddOption() examples have one flaw: they can't tell you *when* an option wasn't specified. If a variable is defaulted to 0, but the user passes --opt=0, then how do you know that the user *did* pass a value?

The solution is the Argument class:

	Argument<int> opt;
	args.AddOption( "opt", "description of opt", opt );

After calling args.Process(), the object can be queried:

	if (opt()) {
		int value = opt.get();
		//use value
	}
	//else --opt wasn't used on the command line

## Help

By using an argument of /? (Windows only) or -? or --help (all platforms) the above options will produce this output:

    Usage: example [OPTION...] filename threads number...
    
      filename            the file to process
      threads             number of threads to use
      number...           initialization numbers
    
    Options:
          --scale         scaling factor
      -v, --verbose       output additional info

(Note that /scale, /v, and /verbose will be output on Windows.)

Help is enabled by default, but can be disabled with:

    args.DisableHelp();

## Tests

CyoArguments comes with a test suite to help verify the correctness of the library. Simply build with ./build.sh on Linux, or compile with Visual Studio on Windows.

## Platforms

CyoArguments requires a C++14 compiler, and is known to build on the following platforms:

* Linux using GCC 4.9;
* Windows using Visual Studio 2013.

## License

### The MIT License (MIT)

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