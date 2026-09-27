#include "sop/simplex.hpp"
#include "sop/presolve.hpp"
#include "sop/core.hpp"
#include <cstdio>

using namespace sop;
using namespace sop::presolve_internal;
using namespace sop::lp;

Problem make_problem(int n, int m, const std::vector<double>& A,
                     const std::vector<double>& rhs, const std::vector<Sense>& sense,
                     const std::vector<double>& obj, const std::vector<double>& lb,
                     const std::vector<double>& ub) {
    Problem p;
    p.resize_variables(n);
    p.resize_constraints(m);
    p.A.resize(m, n);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            const double v = A[static_cast<size_t>(i) * n + j];
            if (v != 0.0) p.A.add(i, j, v);
        }
    }
    p.rhs       = rhs;
    p.row_sense = sense;
    p.obj       = obj;
    p.col_lb    = lb;
    p.col_ub    = ub;
    p.var_type.assign(n, VarType::Continuous);
    p.sense     = ObjSense::Minimize;
    return p;
}

Problem awkward_model() {
    return make_problem(3, 3,
        {1e3,  1e-3,  2e4,
         4e-5, 7e2,   3e-1,
         2,    5,     1e-6},
        {5e3, 9e2, -5e1},
        {Sense::LessEqual, Sense::LessEqual, Sense::GreaterEqual},
        {3, -2, 0.5},
        {0, -kInf, 0},
        {40, kInf, 5});
}

int main() {
    const Problem orig = awkward_model();
    
    // Test presolve directly
    PresolveOptions ps_opts;
    ps_opts.max_passes       = 10;
    ps_opts.fix_tol          = 1e-12;
    ps_opts.dup_tol          = 1e-12;
    ps_opts.bound_propagation = true;
    ps_opts.singleton_rows    = true;
    ps_opts.duplicate_rows    = true;
    ps_opts.remove_empty      = true;
    ps_opts.ruiz_scaling     = true;
    ps_opts.record_postsolve = true;

    PresolveResult presolve_result = presolve(orig, ps_opts);
    
    printf("Presolve summary: %s\n", presolve_result.summary.c_str());
    printf("Infeasible: %d, Unbounded: %d\n", presolve_result.infeasible, presolve_result.unbounded);
    printf("Reduced model: vars=%d, cons=%d, nnz=%d\n", 
           presolve_result.reduced.num_vars, 
           presolve_result.reduced.num_constraints,
           presolve_result.reduced.A.nnz());
    
    if (!presolve_result.infeasible && !presolve_result.unbounded) {
        printf("Reduced obj: [%.6f, %.6f, %.6f]\n", 
               presolve_result.reduced.obj[0], 
               presolve_result.reduced.obj[1], 
               presolve_result.reduced.obj[2]);
        printf("Reduced lb: [%.6f, %.6f, %.6f]\n", 
               presolve_result.reduced.col_lb[0], 
               presolve_result.reduced.col_lb[1], 
               presolve_result.reduced.col_lb[2]);
        printf("Reduced ub: [%.6f, %.6f, %.6f]\n", 
               presolve_result.reduced.col_ub[0], 
               presolve_result.reduced.col_ub[1], 
               presolve_result.reduced.col_ub[2]);
        printf("Reduced rhs: [%.6f, %.6f, %.6f]\n", 
               presolve_result.reduced.rhs[0], 
               presolve_result.reduced.rhs[1], 
               presolve_result.reduced.rhs[2]);
        printf("Reduced sense: [%d, %d, %d]\n", 
               (int)presolve_result.reduced.row_sense[0], 
               (int)presolve_result.reduced.row_sense[1], 
               (int)presolve_result.reduced.row_sense[2]);
        
        // Print matrix
        printf("Reduced A matrix:\n");
        for (int j = 0; j < presolve_result.reduced.num_vars; ++j) {
            for (int k = presolve_result.reduced.A.col_ptr[j]; k < presolve_result.reduced.A.col_ptr[j+1]; ++k) {
                printf("  A[%d,%d] = %.6f\n", presolve_result.reduced.A.row_ind[k], j, presolve_result.reduced.A.values[k]);
            }
        }
        
        // Solve the reduced model
        SimplexOptions so;
        so.max_iterations = 20000;
        so.primal_tol = 1e-9;
        so.dual_tol = 1e-9;
        
        SimplexEngine engine(presolve_result.reduced);
        SolveResult out;
        engine.solve(so, out);
        printf("Simplex status: %d, message: %s\n", (int)out.status, out.message.c_str());
        if (out.status == Status::Optimal) {
            printf("x = [%.6f, %.6f, %.6f]\n", out.x[0], out.x[1], out.x[2]);
            printf("obj = %.10g\n", out.objective);
        }
    }
    return 0;
}