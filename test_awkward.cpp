#include "sop/simplex.hpp"
#include "sop/presolve.hpp"
#include "sop/core.hpp"
#include <cstdio>

using namespace sop;

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
    
    // Test with presolve disabled
    SolveOptions off;
    off.presolve = false;
    off.scale = true;
    const SolveResult raw = lp::solve_lp(orig, off);
    printf("Without presolve: status=%d, obj=%.10g\n", (int)raw.status, raw.objective);
    if (raw.status == Status::Optimal) {
        printf("  x = [%.6f, %.6f, %.6f]\n", raw.x[0], raw.x[1], raw.x[2]);
    }
    
    // Test with presolve enabled (default)
    SolveOptions on;
    on.presolve = true;
    on.scale = true;
    const SolveResult scaled = lp::solve_lp(orig, on);
    printf("With presolve: status=%d, obj=%.10g\n", (int)scaled.status, scaled.objective);
    if (scaled.status == Status::Optimal) {
        printf("  x = [%.6f, %.6f, %.6f]\n", scaled.x[0], scaled.x[1], scaled.x[2]);
    }
    return 0;
}