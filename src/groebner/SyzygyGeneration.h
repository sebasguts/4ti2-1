/*
4ti2 -- A software package for algebraic, geometric and combinatorial
problems on linear spaces.

Copyright (C) 2006 4ti2 team.
Main author(s): Peter Malkin.

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

#ifndef _4ti2_groebner__SyzygyGeneration_
#define _4ti2_groebner__SyzygyGeneration_

#include "groebner/Generation.h"
#include "groebner/BinomialSet.h"
#include "groebner/WeightedBinomialSet.h"

namespace _4ti2_
{

class SyzygyGeneration : public Generation
{
public:
    virtual void generate(
                    const BinomialSet& bs,
                    Index i,
                    BinomialCollection& bc);
    virtual void generate(
                    const BinomialSet& bs,
                    Index start,
                    Index end,
                    BinomialCollection& bc);

private:
    static bool dominated(
                std::vector<int>& syzergies,
                const BinomialSet& bs,
                const Binomial& b0,
                const Binomial& b1);
    static bool dominated(
                std::vector<int>& syzergies,
                std::vector<int>& back_syzergies,
                const BinomialSet& bs,
                const Binomial& b0,
                const Binomial& b1);
};

} // namespace _4ti2_

#endif
