# 4ti2 -- A software package for algebraic, geometric and combinatorial
# problems on linear spaces.
# 
# Copyright (C) 2006 4ti2 team.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 

# 4ti2 always uses g++ as the c++ compiler.
COMPILER=g++

# Turn on all warnings.
CXXFLAGS=-Wall
# The libraries.
LIBS=
# The include path for source files.
INCLUDE=
# The include path for libraries.
LIBINCLUDE=

# 4ti2 uses the glpk linear programming library (GLPL).
LIBS+=
# Add include path to the file glpk.h if necessary.
INCLUDE+=
# Add library include path for the library libglpk if necessary.
LIBINCLUDE+=

# The optimisation flags.
CXXFLAGS += -O3 -DNDEBUG
# Uncomment the next line for debugging.
#CXXFLAGS += -g
# Uncomment the next line for profiling.
#CXXFLAGS += -pg
