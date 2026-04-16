// Copyright SP26 CSCE311-001/002
//
#ifndef PROJ2_LIB_FILE_READER_H_
#define PROJ2_LIB_FILE_READER_H_

#include "sha_solver.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace proj2 {

// Produce class of Factory method FileReaders::Checkout
class ReaderHandle {
 public:
  // Process:
  //  * PARAMETERS:
  //    + file_paths: location of file in filesystem
  //    + rows_per_file: refers to files above
  //    + file_hashes_out: a 2D vector returning hashes of each seed
  //  * PRECONDITION: all top level vectors are in correspondence
  using HashType = std::array<char, 64>;  // ReaderHandle::HashType
  void Process(const std::vector<std::string>& file_paths,
               const std::vector<std::uint32_t>& rows_per_file,
               std::vector<std::vector<HashType>>* file_hashes_out) const;

 private:
  std::size_t id_;
  std::uint32_t n_;
  bool valid_;
  const SolverHandle* solver_;
  ReaderHandle();
  ReaderHandle(std::size_t, std::uint32_t, const SolverHandle* solver);

  friend class FileReaders;  // Factory class
};


// Factory class. Used to grant instances of ReaderHandle
//
class FileReaders {
 public:
  // INVARIENT: FileReaders::Init is called only once
  static void Init(std::uint32_t total_readers);

  // Blocks until exactly n reader slots are allocated (FIFO).
  static ReaderHandle Checkout(std::uint32_t n, const SolverHandle* solver);

  // Return resources held by handle.
  // PRECONDITION: FileReaders::Init(n) s.t. n >= h, must be called prior to
  static void Checkin(ReaderHandle &&h);
};

} // namespace proj2

#endif // PROJ2_LIB_FILE_READER_H_
