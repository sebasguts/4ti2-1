/*
4ti2 -- A software package for algebraic, geometric and combinatorial
problems on linear spaces.

Copyright (C) 2006 4ti2 team.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
*/

#include "Globals.h"
#include "DataType.h"
#include "Timer.h"
#include "groebner_main.h"
#include "markov_main.h"
#include "qsolve_main.h"
#include "rays_main.h"
#include "circuits_main.h"
#include "zbasis_main.h"
#include "minimize_main.h"
#include "normalform_main.h"
#include "walk_main.h"

#include <new>
#include <iostream>
#include <string>
#include <map>
#include <csignal>

using namespace _4ti2_;

typedef std::map<std::string,int (*)(int,char**)> ExecutableMap;

// The following signal handler should be called when there is arithmetic
// overflow. It is only active when arithmetic overflow detection has been
// turned on.
void signal_handler(int)
{
    std::cerr << "\nERROR: Arithmetic exception.\n";
    exit(1);
}

int main(int argc, char** argv)
{
    std::cout << "\
------------------------------------------------\n\
4ti2 version 1.3, Copyright (C) 2006 4ti2 team.\n\
4ti2 comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome\n\
to redistribute it under certain conditions.\n\
For details, see the file COPYING.\n\
------------------------------------------------\n\
";

#if defined(_4ti2_INT32_) || defined(_4ti2_INT64_)
    std::cout << "Using " << sizeof(IntegerType)*CHAR_BIT << " bit integers.\n";
#elif defined(_4ti2_GMP_)
    std::cout << "Using arbitrary precision integers.\n";
#endif
 
try
{
#ifndef _4ti2_GMP_
    // Set the signal handler to catch arithmetic signals.
    // TODO: Unfortunately, arithmetic overflow exceptions send a generic
    // SIGABRT signal.
    signal(SIGABRT, &signal_handler);
    signal(SIGFPE, &signal_handler);
#endif

    if (argc == 1)
    {
        std::cerr << "Usage: 4ti2 <exec> ...\n";
        exit(1);
    }

    // We associate executable names with their corresponding main functions.
    ExecutableMap executables;
    executables.insert(ExecutableMap::value_type("groebner", &groebner_main));
    executables.insert(ExecutableMap::value_type("markov", &markov_main));
    executables.insert(ExecutableMap::value_type("rays", &rays_main));
    executables.insert(ExecutableMap::value_type("circuits", &circuits_main));
    executables.insert(ExecutableMap::value_type("qsolve", &qsolve_main));
    executables.insert(ExecutableMap::value_type("zbasis", &zbasis_main));
    executables.insert(ExecutableMap::value_type("minimize", &minimize_main));
    executables.insert(ExecutableMap::value_type("normalform", &normalform_main));
    executables.insert(ExecutableMap::value_type("walk", &walk_main));

    Globals::exec = argv[1];
    ExecutableMap::iterator i = executables.find(Globals::exec);
    if (i == executables.end())
    {
        std::cerr << "Error: Unrecognized executable: " << argv[1] << "\n";
        exit(1);
    }
    int status = (*i->second)(argc-1, argv+1);

    std::cout << "4ti2 Total Time: " << Timer::global << " secs.\n";
    return status;
}
catch(const std::bad_alloc& x)
{
    std::cerr << "Error: Out of memory: " << x.what() << std::endl;
    exit(1);
}
catch(...)
{
    std::cerr << "Error: Unhandled exception.\n";
    exit(1);
}
    return 1;
}