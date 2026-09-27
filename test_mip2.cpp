#include "sop/mip.hpp"
#include "sop/core.hpp"
#include <cstdio>

using namespace sop;

int main() {
    // x = y,  x + y = 1,  x,y in {0,1}
    Problem p;
    p.resize_variables(2);
    p.resize_constraints(2);
    p.A.add(0, 0, 1.0); p.A.add(0, 1, -1.0);   // x - y = 0
    p.A.add(1, 0, 1.0); p.A.add(1, 1, 1.0);   // x + y = 1
    p.rhs[0] = 0.0;  p.row_sense[0] = Sense::Equal;
    p.rhs[1] = 1.0;  p.row_sense[1] = Sense::Equal;
    p.obj = {1.0, 0.0};
    p.col_lb = {0.0, 0.0};
    p.col_ub = {1.0, 1.0};
    p.var_type = {VarType::Binary, VarType::Binary};

    mip::MipOptions opts;
    opts.max_nodes = 100;
    opts.most_fractional = true;
    mip::MipDiagnostics dg;
    
    const SolveResult r = mip::solve_mip(p, opts, &dg);
    
    printf("MIP Result:\n");
    printf("  status = %d (%s)\n", (int)r.status, to_string(r.status));
    printf("  message = %s\n", r.message.c_str());
    printf("  nodes = %d\n", dg.nodes);
    if (r.status == Status::Optimal) {
        printf("  x = [%.10f, %.10f]\n", r.x[0], r.x[1]);
        printf("  obj = %.10f\n", r.objective);
        printf("  violation = %.10f\n", p.max_violation(r.x));
        printf("  integrality violation = %.10f\n", p.max_integrality_violation(r.x));
    }
    
    return 0;
}