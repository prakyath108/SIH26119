#include "sop/simplex.hpp"
#include "sop/core.hpp"
#include <cstdio>
#include <cmath>

using namespace sop;

Problem make_problem(int n, int m, const std::vector<double>& A,
                     const std::vector<double>& rhs, const std::vector<Sense>& sense,
                     const std::vector<double>& obj, const std::vector<double>& lb,
                     const std::vector<double>& ub,
                     ObjSense os = ObjSense::Minimize) {
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
    p.sense     = os;
    return p;
}

int main() {
    std::printf("fixed variable\n");
    const int n = 2, m = 2;
    // minimise x + y  s.t.  x = 3,  x + y <= 10 ;  x fixed to 3, y >= 0
    Problem p = make_problem(n, m,
        {1, 0, 1, 1},
        {3, 10},
        {Sense::Equal, Sense::LessEqual},
        {1, 1},
        {3, 0},
        {3, kInf});

    printf("Problem built, solving...\n");
    
    SolveOptions opts;
    opts.presolve = false;  // Disable presolve
    
    const SolveResult r = lp::solve_lp(p, opts);
    printf("Solve returned\n");
    std::printf("status=%d, obj=%.10g\n", (int)r.status, r.objective);
    if (r.status == Status::Optimal) {
        std::printf("x=[%.6f, %.6f]\n", r.x[0], r.x[1]);
        if (std::fabs(r.x[0] - 3.0) > 1e-9) printf("FAIL: x not 3\n");
        if (std::fabs(r.x[1] - 0.0) > 1e-7) printf("FAIL: y not 0\n");
        if (std::fabs(r.objective - 3.0) > 1e-7) printf("FAIL: obj not 3\n");
    }
    return 0;
}