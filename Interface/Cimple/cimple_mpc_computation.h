//
// Created by jonathan on 1/5/18.
//

#ifndef CIMPLE_CIMPLE_MPC_COMPUTATION_H
#define CIMPLE_CIMPLE_MPC_COMPUTATION_H

#include <stddef.h>
#include <math.h>
#include "cimple_system.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include "cimple_polytope_library.h"

/**
 * @brief Set up weight matrices for the quadratic problem
 * @param P
 * @param q
 * @param L
 * @param M
 * @param now
 * @param s_dyn
 * @param f_cost
 * @param time_horizon
 * @return
 */
polytope *set_cost_function(gsl_matrix *P,
                            gsl_vector *q,
                            gsl_matrix *L,
                            gsl_vector *M,
                            current_state *now,
                            system_dynamics *s_dyn,
                            cost_function *f_cost,
                            size_t time_horizon);

/**
 * @brief Set up GUROBI environment and solve qp
 * @param low_u
 * @param low_cost
 * @param P
 * @param q
 * @param opt_constraints
 * @param time_horizon
 * @param n
 */
void compute_optimal_control_qp(gsl_matrix *low_u,
                                double *low_cost,
                                gsl_matrix *P,
                                gsl_vector* q,
                                polytope *opt_constraints,
                                size_t time_horizon,
                                size_t n);

/**
 * @brief Calculate (optimal) input that will be applied to take plant from current state (now) to target_abs_state.
 *
 * Global function to compute continuous control input for discrete transition.
 *
 * Computes continuous control input sequence which takes the plant:
 *
 *      - from now
 *      - to (chebyshev center of) target_abs_state
 *      => partitions given by discrete dynamics (d_dyn)
 *
 * Control input is calculated such that it minimizes:
 *
 *      f(x, u) = |Rx|_{ord} + |Qu|_{ord} + r'x + distance_error_weight * |xc - x(N)|_{ord}
 *      with xc == chebyshev center of target_abs_state
 *
 *    Notes
 *    =====
 *    1. The same horizon length as in reachability analysis
 *    should be used in order to guarantee feasibility.
 *
 *    2. If the closed loop algorithm has been used
 *    to compute reachability the input needs to be
 *    recalculated for each time step
 *    (with decreasing horizon length).
 *
 *    In this case only u(0) should be used as
 *    a control signal and u(1) ... u(N-1) discarded.
 *
 *    3. The "conservative" calculation makes sure that
 *    the plant remains inside the convex hull of the
 *    starting region during execution, i.e.::
 *
 *    x(1), x(2) ...  x(N-1) are in starting region.
 *
 *    If the original proposition preserving partition
 *    is not convex, then safety cannot be guaranteed.
 *
 * @param low_u row k contains the control input: u(k) dim[N x m]
 * @param now initial continuous state
 * @param d_dyn discrete abstraction of system
 * @param s_dyn system dynamics (including auxiliary matrices)
 * @param target_abs_state index of target region in discrete dynamics (d_dyn)
 * @param f_cost cost func matrices: f(x, u) = |Rx|_{ord} + |Qu|_{ord} + r'x + distance_error_weight *|xc - x(N)|_{ord}
 */
void get_input (gsl_matrix *u,
                current_state * now,
                discrete_dynamics *d_dyn,
                system_dynamics *s_dyn,
                int target_abs_state,
                cost_function * f_cost,
                size_t current_time_horizon,
                polytope **polytope_list_backup);


/**
 * @brief Calculates (optimal) input to reach desired state (P3) from current state (now) through convex optimization
 *
 * Calculate the sequence low_u such that:
 *
 *      - x(t+1) = A x(t) + B u(t) + K
 *      - x(k) in P1 for k = 0,...,N-1 (if closed loop == 'true')
 *      - x(N) in P3
 *      - [u(k); x(k)] always obey s_dyn.Uset
 *
 * Actual optimality is compared in get_input()
 *
 *
 * @param low_u currently optimal calculated input to target region (input to beat)
 * @param now current state
 * @param s_dyn system dynamics (including auxiliary matrices)
 * @param P1 current polytope (or hull of region) the system is in
 * @param P3 a polytope from the target region
 * @param ord ordinance of the norm that should be minimized ord in {1, 2, INFINITY} (currently only '2' is possible)
 * @param time_horizon
 * @param f_cost predefined cost functions |Rx|_{ord} + |Qu|_{ord} + r'x + mid_weight * |xc - x(N)|_{ord}
 * @param low_cost cost associate to low_u
 */
void search_better_path(gsl_matrix *low_u,
                        current_state *now,
                        system_dynamics *s_dyn,
                        polytope *P1,
                        polytope *P3,
                        int ord,
                        size_t time_horizon,
                        cost_function * f_cost,
                        double* low_cost,
                        polytope **polytope_list_backup,
                        size_t total_time);

/**
 * @brief Compute a polytope that constraints the system over the next N time steps to fullfill the GR(1) specifications
 *
 * @param L_full empty matrix when passed in, left side of constraint polytope at the end
 * @param M_full empty vector when passed in, right side of constraint polytope at the end
 * @param s_dyn system dynamics (including auxiliary matrices)
 * @param list_polytopes list of N+1 polytopes in which the systems needs to be in to reach new desired state at time N
 * @param N time horizon
 *
 * Compute the components of the polytope:
 *
 *      L [x(0)' u(0)' ... u(N-1)']' <= M
 *
 * which stacks the following constraints:
 *
 *      - x(t+1) = A x(t) + B u(t) + E d(t)
 *      - [u(k); x(k)] in s_dyn.Uset for all k => system obeys predefined constraints on how inputs may behave
 *
 * [L_full; M_full] polytope intersection of required and allowed polytopes
 *
 */
polytope * set_path_constraints(current_state * now,
                                system_dynamics * s_dyn,
                                polytope **list_polytopes,
                                size_t N);


#endif //CIMPLE_CIMPLE_MPC_COMPUTATION_H
