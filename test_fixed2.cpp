#include "sop/presolve.hpp"
#include "sop/core.hpp"
#include <cstdio>

using namespace sop;
using namespace sop::presolve_internal;

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

int main() {
    const int n = 2, m = 2;
    Problem p = make_problem(n, m,
        {1, 0, 1, 1},
        {3, 10},
        {Sense::Equal, Sense::LessEqual},
        {1, 1},
        {3, 0},
        {3, kInf});

    printf("Original problem:\n");
    printf("  vars=%d, cons=%d, nnz=%d\n", p.num_vars, p.num_constraints, p.A.nnz());
    fflush(stdout);

    PresolveOptions opts;
    opts.max_passes = 10;
    opts.fix_tol = 1e-12;
    opts.dup_tol = 1e-12;
    opts.bound_propagation = true;
    opts.singleton_rows = true;
    opts.duplicate_rows = true;
    opts.remove_empty = true;
    opts.ruiz_scaling = false;
    opts.record_postsolve = true;

    printf("Calling presolve with all options...\n");
    fflush(stdout);
    PresolveResult presolve_result = presolve(p, opts);
    printf("Presolve returned\n");
    fflush(stdout);
    
    printf("Presolve result:\n");
    printf("  summary: %s\n", presolve_result.summary.c_str());
    printf("  infeasible: %d, unbounded: %d\n", presolve_result.infeasible, presolve_result.unbounded);
    printf("  reduced vars=%d, cons=%d, nnz=%d\n", 
           presolve_result.reduced.num_vars, 
           presolve_result.reduced.num_constraints,
           presolve_result.reduced.A.nnz());
    
    return 0;
}