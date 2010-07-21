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

#include "qsolve/QSolveAlgorithm.h"
#include "qsolve/Globals.h"
#include "qsolve/HermiteAlgorithm.h"
#include "qsolve/VectorStream.h"
#include "qsolve/Debug.h"
#include "qsolve/Stream.h"
#include "qsolve/IndexSetD.h"
#include "qsolve/IndexSetStream.h"
#include "4ti2/4ti2.h"
#include "qsolve/MatrixAlgorithm.h"
#include "qsolve/SupportAlgorithm.h"
#include "qsolve/RayState.h"

#undef DEBUG_4ti2
#define DEBUG_4ti2(X) //X

using namespace _4ti2_;

template <class T>
QSolveAlgorithm<T>::QSolveAlgorithm()
{
}

template <class T>
QSolveAlgorithm<T>::QSolveAlgorithm(QSolveVariant v, QSolveConsOrder o)
    : variant(v)
{
    order.set_constraint_order(o);
}

template <class T>
QSolveAlgorithm<T>::~QSolveAlgorithm()
{
}

template <class T>
void
QSolveAlgorithm<T>::compute(
                            const ConeT<T>& cone,
                            VectorArrayT<T>& rays,
                            VectorArrayT<T>& cirs,
                            VectorArrayT<T>& subspace)
{
    DEBUG_4ti2(*out << "MATRIX:\n" << cone.get_matrix() << "\n";)
    //DEBUG_4ti2(*out << "RELS:\n" << rels << "\n";)
    //DEBUG_4ti2(*out << "SIGN:\n" << sign << "\n";)

    Size n = cone.num_vars();
    Size m = cone.num_cons();
    Size num_cons = n+m;

    IndexSetD full_rs(num_cons,0);
    cone.constraint_set(_4ti2_LB, full_rs);
    IndexSetD full_cir(num_cons,0);
    cone.constraint_set(_4ti2_DB, full_cir);
    IndexSetD full_eq(num_cons,0);
    cone.constraint_set(_4ti2_EQ, full_eq);

    DEBUG_4ti2(*out << "RS:\n" << full_rs << "\n";)
    DEBUG_4ti2(*out << "CIR:\n" << full_cir << "\n";)
    DEBUG_4ti2(*out << "EQ:\n" << full_eq << "\n";)

    // If there are only ray components...
    // TODO: This could be better.
    if (full_rs.full()) {
        // Construct initial rays.
        std::vector<Index> ray_ineqs;
        for (Index i = 0; i < cone.num_vars(); ++i) {
            VectorT<T> ray(n,0);
            ray.set(i,1);
            rays.insert(ray);
            ray_ineqs.push_back(i);
        }
        std::vector<Index> cir_ineqs;
        compute(cone, rays, ray_ineqs, cirs, cir_ineqs);
        return;
    }

    // If there are only circuit components...
    // TODO: This could be better.
    if (full_cir.full()) {
        // Construct initial circuits.
        std::vector<Index> cir_ineqs;
        for (Index i = 0; i < cone.num_vars(); ++i) {
            VectorT<T> cir(n,0);
            cir.set(i,1);
            cirs.insert(cir);
            cir_ineqs.push_back(i);
        }
        std::vector<Index> ray_ineqs;
        compute(cone, rays, ray_ineqs, cirs, cir_ineqs);
        return;
    }

    ConeT<T> proj_cone;
    VectorArrayT<T> map;
    // Construct projected cone.
    cone.canonize(proj_cone, subspace, map);

    // Construct initial rays.
    VectorArrayT<T> proj_rays(0, proj_cone.num_vars());
    VectorArrayT<T> proj_cirs(0, proj_cone.num_vars());
    std::vector<Index> proj_ray_ineqs;
    std::vector<Index> proj_cir_ineqs;
    for (Index i = 0; i < proj_cone.num_vars(); ++i) {
        if (proj_cone.get_constraint_type(i) == _4ti2_LB) {
            VectorT<T> ray(proj_cone.num_vars(),0);
            ray.set(i,1);
            proj_rays.insert(ray);
            proj_ray_ineqs.push_back(i);
        }
        else {
            VectorT<T> cir(proj_cone.num_vars(),0);
            cir.set(i,1);
            proj_cirs.insert(cir);
            proj_cir_ineqs.push_back(i);
        }
    }
    DEBUG_4ti2(*out << "CONE CONS:\n" << proj_cone.get_constraint_types() << "\n";)

    compute(proj_cone, proj_rays, proj_ray_ineqs, proj_cirs, proj_cir_ineqs);

    // Lift rays back to original space.
    // TODO: This could be done faster.
    rays.init(proj_rays.get_number(), n);
    VectorArrayT<T>::dot(map, proj_rays, rays);
    rays.normalise();

    // Lift circuits back to original space.
    // TODO: This could be done faster.
    cirs.init(proj_cirs.get_number(), n);
    VectorArrayT<T>::dot(map, proj_cirs, cirs);
    cirs.normalise();

    return;
}

template <class T>
void
QSolveAlgorithm<T>::compute(const ConeT<T>& cone, 
            VectorArrayT<T>& rays, std::vector<Index>& ray_ineqs, 
            VectorArrayT<T>& cirs, std::vector<Index>& cir_ineqs)
{
    // The number of constraints added so far.
    IndexSetD lbs(cone.num_vars()+cone.num_cons());
    cone.constraint_set(_4ti2_LB, lbs);
    IndexSetD dbs(cone.num_vars()+cone.num_cons());
    cone.constraint_set(_4ti2_DB, dbs);

    if (lbs.count()+2*dbs.count() <= IndexSetDS::max_size) {
        RayState<T,IndexSetDS> state(cone, rays);
        if (variant == MATRIX) {
            MatrixAlgorithm<IndexSetDS> alg(order);
            // Compute ray only constraints first.
            alg.compute_rays(cone, state, ray_ineqs);
            // Compute circuits.
            rays.transfer(cirs, 0, cirs.get_number(), rays.get_number());
            alg.compute_cirs(cone, state, cir_ineqs);
        } else {
            SupportAlgorithm<IndexSetDS> alg(order);
            // Compute ray only constraints first.
            alg.compute_rays(cone, state, ray_ineqs);
            // Compute circuits.
            rays.transfer(cirs, 0, cirs.get_number(), rays.get_number());
            alg.compute_cirs(cone, state, cir_ineqs);
        }
        // Separate the rays from the circuits.
        split_rays(cone, state.supps, rays, cirs);
    } else {
        RayState<T,IndexSetD> state(cone, rays);
        if (variant == MATRIX) {
            MatrixAlgorithm<IndexSetD> alg(order);
            // Compute ray only constraints first.
            alg.compute_rays(cone, state, ray_ineqs);
            // Compute circuits.
            rays.transfer(cirs, 0, cirs.get_number(), rays.get_number());
            alg.compute_cirs(cone, state, cir_ineqs);
        } else {
            SupportAlgorithm<IndexSetD> alg(order);
            // Compute ray only constraints first.
            alg.compute_rays(cone, state, ray_ineqs);
            // Compute circuits.
            rays.transfer(cirs, 0, cirs.get_number(), rays.get_number());
            alg.compute_cirs(cone, state, cir_ineqs);
        }
        // Separate the rays from the circuits.
        split_rays(cone, state.supps, rays, cirs);
    }
}

// Splits rays into rays and circuits.
// NOTE: Does not update supports.
// TODO: cannot assume anything about the size of the supports.
template <class T> template <class IndexSet>
void
QSolveAlgorithm<T>::split_rays(
                const ConeT<T>& cone,
                const std::vector<IndexSet>& supps,
                VectorArrayT<T>& rays,
                VectorArrayT<T>& cirs)
{
    IndexSet ray_mask(cone.num_vars()+cone.num_cons());
    cone.constraint_set(_4ti2_LB, ray_mask);
    int index = 0;
    for (int i = 0; i < rays.get_number(); ++i) {
        if (!ray_mask.set_disjoint(supps[i])) {
            rays.swap_vectors(i,index);
            ++index;
        }
    }
    cirs.transfer(rays, index, rays.get_number(), 0);
}

#if 0
template <class T> template <class IndexSet>
Index
QSolveAlgorithm<T>::next_circuit_constraint(
                const ConeT<T>& cone,
                const VectorArrayT<T>& vs,
                const IndexSet& rem)
{
    Index next_con = *rem.begin();
    if (order.get_constraint_order() == MININDEX) { return next_con; }

    typename IndexSet::Iter it = rem.begin(); ++it;
    if (it == rem.end()) { return next_con; }

    Size next_pos_count, next_neg_count, next_zero_count;
    cone.slack_count(vs, next_con, next_pos_count, next_neg_count, next_zero_count);
    while (it != rem.end()) {
        Size pos_count, neg_count, zero_count;
        cone.slack_count(vs, *it, pos_count, neg_count, zero_count);
        if ((*order.circuit_compare)(next_pos_count, next_neg_count, next_zero_count,
                        pos_count, neg_count, zero_count)) {
            next_con = *it;
            next_pos_count = pos_count;
            next_neg_count = neg_count;
            next_zero_count = zero_count;
        }
        ++it;
    }
    DEBUG_4ti2(*out << "Next Constraint is " << next_con << "\n";)
    return next_con;
}
#endif

inline
IndexRanges::IndexRanges()
{
    pthread_mutex_init(&mutex1, 0);
    r1_start = r1_index = r1_end = r2_start = r2_index = r2_end = increment = 0;
}

inline void
IndexRanges::init(Index _r1_start, Index _r1_end, Index _r2_start, Index _r2_index, Index _r2_end)
{
    r1_start = r1_index = _r1_start;
    r1_end = _r1_end;
    r2_start = _r2_start;
    r2_index = _r2_index;
    r2_end = _r2_end;
    if (Globals::num_threads==1) { increment = r1_end-r1_start; }
    else { increment = 1000; } // TODO
    //increment = (r1_end-r1_start)/Globals::num_threads+1;
}

inline void
IndexRanges::next(Index& _r1_start, Index& _r1_end, Index& _r2_start, Index& _r2_index, Index& _r2_end)
{
    pthread_mutex_lock(&mutex1);
    _r1_start = r1_index;
    r1_index = std::min(r1_index+increment, r1_end);
    _r1_end = r1_index;
    _r2_start = r2_start;
    _r2_index = r2_index;
    _r2_end = r2_end;
    pthread_mutex_unlock(&mutex1);
}

#undef DEBUG_4ti2

