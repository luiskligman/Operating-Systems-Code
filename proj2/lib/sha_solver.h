#ifndef PROJ2_LIB_SOLVER_H_
#define PROJ2_LIB_SOLVER_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace proj2 {

// A class referencing a set of Solvers
// It is a Product of the method Factory ShaSolver::Checkout
class SolverHandle {
 public:
  // cardinality of solvers in the set
  std::uint32_t k() const;

  // ComputeSHAs: for each seed, dispatch a solver to compute iterative
  //   SHA256. Returns value in output parameter out_hex
  void ComputeSHAs(const std::vector<std::string>& seeds,
                   const std::vector<std::uint32_t>& iterations,
                   std::vector<std::array<char, 64>>* out_hex) const;

 private:
  std::size_t id_;
  std::uint32_t k_;
  bool valid_;

  SolverHandle() : k_(0), valid_(false) {}
  SolverHandle(std::size_t id, std::uint32_t k) : id_(id), k_(k), valid_(true) {}

  friend class ShaSolvers;
};


class ShaSolvers {
 public:
    // configure once at startup
    static void Init(std::uint32_t total_available);

    // Blocks until exactly k solver slots are allocated.
    static SolverHandle Checkout(std::uint32_t amount_requested);

    // Return resources held by handle.
    static void Checkin(SolverHandle &&handle);
};

} // namespace proj2

#endif // PROJ2_LIB_SOLVER_H_

