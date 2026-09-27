#include "sop/presolve.hpp"
#include "sop/core.hpp"
#include "sop/simplex.hpp"
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

    PresolveResult presolve_result = presolve(p, opts);
    
    printf("Presolve result:\n");
    printf("  summary: %s\n", presolve_result.summary.c_str());
    printf("  infeasible: %d, unbounded: %d\n", presolve_result.infeasible, presolve_result.unbounded);
    printf("  reduced vars=%d, cons=%d, nnz=%d\n", 
           presolve_result.reduced.num_vars, 
           presolve_result.reduced.num_constraints,
           presolve_result.reduced.A.nnz());
    printf("  fixed_value[0]=%.1f, fixed_value[1]=%.1f\n", 
           presolve_result.info.fixed_value[0], presolve_result.info.fixed_value[1]);
    printf("  orig_col: [");
    for (size_t i = 0; i < presolve_result.info.orig_col.size(); ++i) {
        printf("%d", presolve_result.info.orig_col[i]);
        if (i + 1 < presolve_result.info.orig_col.size()) printf(", ");
    }
    printf("]\n");
    printf("  obj_offset=%.1f\n", presolve_result.info.obj_offset);
    
    // Solve reduced problem
    SimplexOptions so;
    so.max_iterations = 20000;
    SimplexEngine engine(presolve_result.reduced);
    SolveResult r;
    engine.solve(so, r);
    printf("\nReduced solve:\n");
    printf("  status=%d, obj=%.6f\n", (int)r.status, r.objective);
    if (r.status == Status::Optimal) {
        printf("  x=[");
        for (size_t i = 0; i < r.x.size(); ++i) {
            printf("%.6f", r.x[i]);
            if (i + 1 < r.x.size()) printf(", ");
        }
        printf("]\n");
        
        // Postsolve
        std::vector<double> x, duals, rc;
        double obj, viol;
        std::string msg;
        bool ok = postsolve(p, presolve_result.info, r.x, r.duals, x, duals, rc, obj, viol, msg);
        printf("\nPostsolve:\n");
        printf("  ok=%d\n", ok);
        printf("  x=[");
        for (size_t i = 0; i < x.size(); ++i) {
            printf("%.6f", x[i]);
            if (i + 1 < x.size()) printf(", ");
        }
        printf("]\n");
        printf("  obj=%.6f, viol=%.6f\n", obj, viol);
    }
    return 0;
}