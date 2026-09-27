#include "sop/lp/ipm.hpp"
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
    // min x + 2y
    // s.t. x + y >= 3
    //      x - y <= 1
    //      x, y >= 0
    const int n = 2, m = 2;
    Problem p = make_problem(n, m,
        {1, 1, 1, -1},
        {3, 1},
        {Sense::GreaterEqual, Sense::LessEqual},
        {1, 2},
        {0, 0},
        {kInf, kInf});

    ipm::IpOptions opts;
    opts.verbose = 4;  // Maximum verbosity
    opts.max_iterations = 10;
    opts.eps_primal = 1e-7;
    opts.eps_dual = 1e-7;
    opts.eps_complement = 1e-7;

    printf("Starting IPM...\n");
    const ipm::IpResult r = ipm::solve_ipm(p, opts);

    printf("Final status=%d, iter=%d, obj=%.6f\n", r.status, r.iterations, r.primal_obj);
    if (r.status == 0) {
        std::printf("x=[%.6f, %.6f]\n", r.final_point.x[0], r.final_point.x[1]);
    }
    return 0;
}