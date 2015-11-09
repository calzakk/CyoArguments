# CyoArguments

CyoArguments is a C++ library for processing command-line arguments.

From the outset the intention was an easy-to-use library, developed using modern C++ features (C++11 and C++14).

## Usage

CyoArguments is implemented using header files only, so doesn't need to be separately built prior to use. One header provides the interface, and several headers (in the 'details' subdirectory) provide implementation details that are only indirectly #included.

    #include "cyoarguments.hpp"
    using namespace cyoarguments;

Instantiate an Arguments object:

    Arguments args;

Add any optional arguments:

    bool ignoreErrors = false;
    args.AddOption( "ignore-errors", "continue processing on error", ignoreErrors );

    bool verbose = false;
    args.AddOption( 'v', "verbose", "output additional info", verbose );

Add any required arguments:

    std::string filename;
    args.AddRequired( "filename", "the file to process", filename );

    int threads = 0;
    args.AddRequired( "threads", "number of threads to use", threads );

Required arguments cannot be omitted when the program is executed, and must be specified in the order they were added to the object. Note that there must be at least one optional argument or at least one required argument.

Pass the command-line options to the object:

    arguments.Process(argc, argv);

The above variables will be assigned accordingly.

Please refer to the example program that demonstrates CyoArguments in action.

## Tests

CyoArguments comes with a test suite to help verify the correctness of the library. Simply build with ./build.sh on Linux, or compile with Visual Studio on Windows.

## Platforms

CyoArguments is known to build on the following platforms:

* Linux using GCC 4.9;
* Windows using Visual Studio 2013.

A C++14 compiler is required.