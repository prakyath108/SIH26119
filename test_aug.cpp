#include "sop/kkt/kkt_system.hpp"
#include "sop/kkt/ldl_factor.hpp"
#include "sop/core.hpp"
#include "sop/lp/ipm.hpp"
#include <cstdio>

int main() {
    // Test the augmented system for the simple LP
    // min x + 2y
    // s.t. x + y >= 3
    //      x - y <= 1
    //      x, y >= 0
    
    int n = 2, m = 2;
    sop::Problem p;
    p.resize_variables(n);
    p.resize_constraints(m);
    p.A.resize(m, n);
    p.A.add(0, 0, 1.0); p.A.add(0, 1, 1.0);
    p.A.add(1, 0, 1.0); p.A.add(1, 1, -1.0);
    p.rhs = {3.0, 1.0};
    p.row_sense = {sop::Sense::GreaterEqual, sop::Sense::LessEqual};
    p.obj = {1.0, 2.0};
    p.col_lb = {0.0, 0.0};
    p.col_ub = {sop::kInf, sop::kInf};
    
    // Initial point
    sop::ipm::IpData data;
    data.x = {1.0, 1.0};
    data.y = {0.0, 0.0};
    data.s = {1.0, 1.0};
    data.z = {0.0, 0.0};
    
    int N = 2 + 2 + 2;  // n + m + n = 6
    
    // Build augmented system
    std::vector<double> Q_diag(2, 0.0);
    std::vector<int64_t> a_row_ptr, a_col_ind;
    std::vector<double> a_values;
    sop::kkt::assemble_augmented_system(p.A, Q_diag, data.x, data.s, a_row_ptr, a_col_ind, a_values);
    
    printf("Augmented system (%dx%d):\n", 6, 6);
    printf("row_ptr: ");
    for (int64_t v : a_row_ptr) printf("%lld ", v);
    printf("\ncol_ind: ");
    for (int64_t v : a_col_ind) printf("%lld ", v);
    printf("\nvalues: ");
    for (double v : a_values) printf("%.3f ", v);
    printf("\n");
    
    // Test LDL factorization
    sop::kkt::LdlFactor factor;
    sop::kkt::LdlOptions opts;
    opts.regularization = 1e-10;
    
    if (sop::kkt::ldl_factorize(a_row_ptr, a_col_ind, a_values, 6, factor, opts)) {
        printf("Factorization successful!\n");
        printf("D = ");
        for (double d : factor.D) printf("%.6f ", d);
        printf("\n");
        
        // Test solve with a simple RHS
        std::vector<double> rhs(6, 1.0);
        std::vector<double> sol = rhs;
        sop::kkt::ldl_solve_inplace(factor, sol);
        printf("Solution: ");
        for (double v : sol) printf("%.6f ", v);
        printf("\n");
    } else {
        printf("Factorization failed!\n");
    }
    
    return 0;
}