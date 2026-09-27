#include "sop/simplex.hpp"
#include "sop/core.hpp"
#include <cstdio>

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
    // Test product mix (maximise)
    std::printf("product mix (maximise)\n");
    const int n = 2, m = 3;
    Problem p = make_problem(n, m,
        {1, 0, 0, 2, 3, 2},
        {4, 12, 18},
        {Sense::LessEqual, Sense::LessEqual, Sense::LessEqual},
        {3, 5},
        {0, 0},
        {kInf, kInf},
        ObjSense::Maximize);

    SolveOptions o;
    const SolveResult r = lp::solve_lp(p, o);
    std::printf("status=%d, obj=%.10g\n", (int)r.status, r.objective);
    if (r.status == Status::Optimal) {
        std::printf("x=[%.6f, %.6f]\n", r.x[0], r.x[1]);
    }
    return 0;
}