//
// Created by be107admin on 9/25/17.
//

#ifndef CIMPLE_POLYTOPE_LIBRARY_CIMPLE_H
#define CIMPLE_POLYTOPE_LIBRARY_CIMPLE_H

#include <math.h>
#include <gsl/gsl_vector_double.h>
#include <gsl/gsl_matrix.h>
#include "cimple_gsl_library_extension.h"
#include <gurobi_c.h>
#include "setoper.h"
#include <cdd.h>
#include "cimple_auxiliary_functions.h"
#include "cimple_minksum_wrapper.h"


/**
 * H left side of polytope (sometimes noted A or L)
 * G right side of polytope (sometimes noted b or M)
 *
 *      H.x <= G
 *
 * Chebyshev center is a possible definition  of the "center" of the polytope.
 */
typedef struct polytope{

    gsl_matrix * H;
    gsl_vector * G;
    double *chebyshev_center;

}polytope;


/**
 * @brief "Constructor" Dynamically allocates the space a polytope needs
 * @param k H.size1 == G.size
 * @param n H.size2
 * @return
 */
struct polytope *polytope_alloc(size_t k, size_t n);

/**
 * @brief "Destructor" Deallocates the dynamically allocated memory of the polytope
 * @param polytope
 */
void polytope_free(polytope *polytope);


/**
 * Subdivision of abstract state containing additionally to the polytope also safe mode instructions
 * The array of polytopes "polytope **safe_mode" contains N polytopes the system has to go through to reach the invariant set
 */
typedef struct cell{

    polytope **safe_mode;
    polytope *polytope_description;

}cell;

/**
 * @brief "Constructor" Dynamically allocates the space a cell needs
 * @param k cell.polytope.H.size1 == G.size
 * @param n cell.polytope.HH.size2
 * @return
 */
struct cell *cell_alloc(size_t k,
                        size_t n,
                        int time_horizon);

/**
 * @brief "Destructor" Deallocates the dynamically allocated memory of the cell
 * @param cell
 */
void cell_free(cell *cell);


/**
 * Convex region of several (cells_count) polytopes (array of polytopes**)
 * hull_over_polytopes is the convex hull of polytopes in that abstract state
 * hull.A = NULL if only one polytope exists in that region
 */
typedef struct abstract_state{

    polytope *hull_over_polytopes;

    int cells_count;
    cell **cells;

    int transitions_in_count;
    struct abstract_state** transitions_in;
    int transitions_out_count;
    struct abstract_state** transitions_out;

    int distance_invariant_set;
    struct abstract_state * next_state;
    cell *invariant_set;


}abstract_state;

/**
 * @brief "Constructor" Dynamically allocates the space a region of polytope needs
 *
 * Allocates memory space according to the cells_count and their respective sizes
 *
 * @param k Array with the number of rows of each polytope dim([cells_count])
 * @param k_hull number of rows of convex hull polytope of the region
 * @param n system_dynamics size (e.g. s_dyn.A.size1)
 * @param cells_count
 * @return
 */
struct abstract_state *abstract_state_alloc(size_t *k,
                                            size_t k_hull,
                                            size_t n,
                                            int transitions_in_count,
                                            int transitions_out_count,
                                            int cells_count,
                                            int time_horizon);

/**
 * @brief "Destructor" Deallocates the dynamically allocated memory of the region of polytopes
 * @param abstract_state
 */
void abstract_state_free(abstract_state * abstract_state);

/**
 * @brief Converts two C arrays to a polytope consistent of a left side matrix (i.e. H) and right side vector (i.e. G)
 * @param polytope empty polytope with allocated memory
 * @param k number of rows of polytope (H.size1 or G.size)
 * @param n dimension of polytope = system_dynamics size (e.g. s_dyn.A.size1)
 * @param left_side C array to be converted to left side matrix
 * @param right_side C array to be converted to right side vector
 * @param name name of polytope, will be displayed to user terminal to inform about successfull initialization
 */
void polytope_from_arrays(polytope *polytope,
                          double *left_side,
                          double *right_side,
                          double *cheby,
                          char*name);

/**
 * @brief Converts a polytope in gsl form to cdd constraint form
 * @param original
 * @param err
 * @return
 */
dd_PolyhedraPtr polytope_to_cdd(polytope *original,
                                dd_ErrorType *err);

/**
 * @brief Converts a polytope in cdd constraint form to gsl form
 * @param original
 * @return
 */
polytope* cdd_to_polytope(dd_PolyhedraPtr *original);

/**
 * @brief Generate a polytope representing a scaled unit cube
 * @param scale
 * @param dimensions
 * @return gsl polytope
 */
polytope * polytope_scaled_unit_cube(double scale,
                                     int dimensions);

/**
 * @brief Checks whether a state is in a certain polytope
 * @param polytope
 * @param x state to be checked
 * @return 0 if state is not in polytope or 1 if it is
 */
bool polytope_check_state(polytope *polytope, gsl_vector *x);

/**
 * @brief Check whether P1 \ subset P2
 * @param P1
 * @param P2
 * @return
 */
bool polytope_is_subset(polytope *P1,
                        polytope *P2);

/**
 * @brief Unite inequalities of P1 and P2 in new polytope and remove redundancies
 * @param P1
 * @param P2
 * @return
 */
polytope * polytope_unite_inequalities(polytope *P1,
                                       polytope *P2);


/**
 * @brief Project gsl polytope of n+ dimensions to the first n dimensions
 * @param original
 * @param n
 * @return gsl polytope
 */
polytope* polytope_projection(polytope * original,
                              size_t n);

/**
 * @brief Remove redundancies from gsl polytope inequalities
 * @param original
 * @return
 */
polytope * polytope_minimize(polytope *original);

/**
 * @brief Compute Minkowski sum of two polytopes
 * @param P1
 * @param P2
 * @return
 */
polytope * polytope_minkowski(polytope *P1,
                              polytope *P2);

/**
 * @brief Compute Pontryagin difference of two polytopes C=A-B s.t.:
 * A-B = {c \in A-B| c+b \in A, \forall b \in B}
 * @param P1
 * @param P2
 * @return
 */
polytope * polytope_pontryagin(polytope* P1,
                               polytope* P2);

/**
 * @brief Set up constraints in quadratic problem for GUROBI
 * @param constraints
 * @param model
 * @param N
 * @return
 */
int polytope_to_constraints_gurobi(polytope *constraints,
                                   GRBmodel *model,
                                   size_t N);

/**
 * @brief Generate a polytope representing a scaled unit cube
 * @param scale
 * @param dimensions
 * @return cdd polytope
 */
dd_PolyhedraPtr cdd_scaled_unit_cube(double scale,
                                     int dimensions);


/**
 * @brief Project cdd polytope of n+ dimensions to the first n dimensions
 * @param original
 * @param n
 * @return cdd polytope
 */
void cdd_projection(dd_PolyhedraPtr *original,
                    dd_PolyhedraPtr *new,
                    size_t n,
                    dd_ErrorType *err);

/**
 * @brief Remove redundancies from cdd polytope inequalities
 * @param original
 * @return cdd polytope
 */
dd_PolyhedraPtr cdd_minimize(dd_PolyhedraPtr *original,
                             dd_ErrorType *err);


#endif //CIMPLE_POLYTOPE_LIBRARY_CIMPLE_H
