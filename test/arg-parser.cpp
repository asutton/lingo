// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lingo/cli.hpp"

#include <iostream>
#include <fstream>
#include <iterator>

using namespace lingo;

// -debug       // OK: true
// -debug=true  // OK: true
// -debug=false // OK: false
// -nodebug     // OK: false
// -debug true  // error

int
main(int argc, char* argv[])
{
  cli::Parameter_list parms {
    { cli::flag, "version,v" },
    { cli::flag, "help,h",   },
    { cli::value, "file,f"   }
  };

  cli::Parser parse(parms);
  auto const& result = parse(argc, argv);
  print(&result.positional_arguments());
  print(&result.named_arguments());
}
