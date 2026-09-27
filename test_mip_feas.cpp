#include "sop/simplex.hpp"
#include "sop/core.hpp"
#include <cstdio>

using namespace sop;

Problem make_problem(int n, int m, const std::vector<double>& A,
                     const std::vector<double>& rhs, const std::vector<Sense>& sense,
                     const std::vector<double>& obj, const std::vector<double>& lb,
                     const std::vector<double>& ub,
                     const std::vector<VarType>& var_type) {
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
    p.var_type  = var_type;
    p.sense     = ObjSense::Minimize;
    return p;
}

int main() {
    // x=y, x+y=1, 0<=x,y<=1 (binary relaxed)
    Problem p = make_problem(2, 2,
        {1, -1, 1, 1},
        {0, 1},
        {Sense::Equal, Sense::Equal},
        {1, 0},
        {0, 0},
        {1, 1},
        {VarType::Binary, VarType::Binary});

    printf("LP Relaxation:\n");
    printf("  A = [[1, -1], [1, 1]]\n");
    printf("  rhs = [0, 1]\n");
    printf("  sense = [=, =]\n");
    printf("  bounds: 0 <= x,y <= 1\n");
    
    SolveOptions opts;
    opts.presolve = true;
    opts.scale = true;
    const SolveResult r = lp::solve_lp(p, opts);
    
    printf("Result:\n");
    printf("  status = %d (%s)\n", (int)r.status, to_string(r.status));
    if (r.status == Status::Optimal) {
        printf("  x = [%.10f, %.10f]\n", r.x[0], r.x[1]);
        printf("  obj = %.10f\n", r.objective);
        printf("  violation = %.10f\n", p.max_violation(r.x));
        printf("  integrality violation = %.10f\n", p.max_integrality_violation(r.x));
    } else {
        printf("  message: %s\n", r.message.c_str());
    }
    
    // Check rounded solutions
    std::vector<double> rounded1 = {1.0, 1.0};
    std::vector<double> rounded2 = {0.0, 0.0};
    printf("\nRounded [1,1]:\n");
    printf("  violation = %.10f\n", p.max_violation(rounded1));
    printf("  integrality violation = %.10f\n", p.max_integrality_violation(rounded1));
    printf("\nRounded [0,0]:\n");
    printf("  violation = %.10f\n", p.max_violation(rounded2));
    printf("  integrality violation = %.10f\n", p.max_integrality_violation(rounded2));
    
    return 0;
}