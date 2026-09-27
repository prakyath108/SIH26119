#include "sop/kkt/kkt_system.hpp"
#include "sop/kkt/ldl_factor.hpp"
#include "sop/core.hpp"
#include <cstdio>

using namespace sop;
using namespace sop::kkt;

int main() {
    // Simple 2x2 system test
    int n = 2;
    std::vector<int64_t> row_ptr = {0, 2, 4};
    std::vector<int64_t> col_ind = {0, 1, 0, 1};
    std::vector<double> values = {4.0, 1.0, 1.0, 3.0};  // [4 1; 1 3]
    
    LdlFactor factor;
    LdlOptions opts;
    opts.regularization = 1e-10;
    
    printf("Factoring 2x2 matrix...\n");
    if (ldl_factorize(row_ptr, col_ind, values, n, factor)) {
        printf("Factorization successful\n");
        printf("D = [%.6f, %.6f]\n", factor.D[0], factor.D[1]);
        
        // Solve
        std::vector<double> b = {5.0, 11.0};
        std::vector<double> x = b;
        ldl_solve_inplace(factor, x);
        printf("Solution: x = [%.6f, %.6f]\n", x[0], x[1]);
        printf("Expected: x = [1, 2] (since 4*1 + 1*2 = 6? Wait...)\n");
        // Actually: 4*x + 1*y = 5, 1*x + 3*y = 11
        // x=1, y=3 gives 4+3=7 != 5
        // Let me check: 4x + y = 5, x + 3y = 11
        // x = 1, y = 1 => 4+1=5, 1+3=4 != 11
        // x = 0.5, y = 3 => 2+3=5, 0.5+9=9.5 != 11
        // x = -1, y = 9 => -4+9=5, -1+27=26 != 11
    } else {
        printf("Factorization failed\n");
    }
    return 0;
}