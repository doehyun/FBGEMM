/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <ATen/ATen.h>
#include <ATen/core/op_registration/op_registration.h>
#include <torch/library.h>

#include "fbgemm_gpu/sparse_ops_utils.h"
#include "fbgemm_gpu/split_embeddings_cache_cuda.cuh"

namespace {

// Deprecated for fb namespace! Please use fbgemm namespace instead!
TORCH_LIBRARY_FRAGMENT(fb, m) {
  m.def(
      "linearize_cache_indices(Tensor cache_hash_size_cumsum, Tensor indices, Tensor offsets) -> Tensor");
  DISPATCH_TO_CUDA("linearize_cache_indices", linearize_cache_indices_cuda);
  m.def(
      "lru_cache_populate(Tensor weights, Tensor hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, int time_stamp, Tensor(c!) lru_state, bool stochastic_rounding) -> ()");
  DISPATCH_TO_CUDA("lru_cache_populate", lru_cache_populate_cuda);
  m.def(
      "lru_cache_populate_byte(Tensor weights, Tensor hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor weights_tys, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, int time_stamp, Tensor(c!) lru_state,int row_alignment=16) -> ()");
  DISPATCH_TO_CUDA("lru_cache_populate_byte", lru_cache_populate_byte_cuda);
  m.def(
      "lfu_cache_populate(Tensor weights, Tensor cache_hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, Tensor(c!) lfu_state, bool stochastic_rounding) -> ()");
  DISPATCH_TO_CUDA("lfu_cache_populate", lfu_cache_populate_cuda);
  m.def(
      "lfu_cache_populate_byte(Tensor weights, Tensor cache_hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor weights_tys, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, Tensor(c!) lfu_state, int row_alignment=16) -> ()");
  DISPATCH_TO_CUDA("lfu_cache_populate_byte", lfu_cache_populate_byte_cuda);
  m.def(
      "lxu_cache_lookup(Tensor linear_cache_indices, Tensor lxu_cache_state, int invalid_index = -1) -> Tensor");
  DISPATCH_TO_CUDA("lxu_cache_lookup", lxu_cache_lookup_cuda);
  m.def(
      "lxu_cache_flush(Tensor(a!) uvm_weights, Tensor cache_hash_size_cumsum, Tensor cache_index_table_map, Tensor weights_offsets, Tensor D_offsets, int total_D, Tensor(b!) lxu_cache_state, Tensor(c!) lxu_cache_weights, bool stochastic_rounding) -> ()");
  DISPATCH_TO_CUDA("lxu_cache_flush", lxu_cache_flush_cuda);
  m.def("lxu_cache_slot(int h_in, int C) -> int");
  m.impl(
      "lxu_cache_slot",
      torch::dispatch(
          c10::DispatchKey::CatchAll, TORCH_FN(host_lxu_cache_slot)));
}

TORCH_LIBRARY_FRAGMENT(fbgemm, m) {
  m.def(
      "linearize_cache_indices(Tensor cache_hash_size_cumsum, Tensor indices, Tensor offsets) -> Tensor");
  DISPATCH_TO_CUDA("linearize_cache_indices", linearize_cache_indices_cuda);
  m.def(
      "lru_cache_populate(Tensor weights, Tensor hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, int time_stamp, Tensor(c!) lru_state, bool stochastic_rounding) -> ()");
  DISPATCH_TO_CUDA("lru_cache_populate", lru_cache_populate_cuda);
  m.def(
      "lru_cache_populate_byte(Tensor weights, Tensor hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor weights_tys, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, int time_stamp, Tensor(c!) lru_state, int row_alignment=16) -> ()");
  DISPATCH_TO_CUDA("lru_cache_populate_byte", lru_cache_populate_byte_cuda);
  m.def(
      "lfu_cache_populate(Tensor weights, Tensor cache_hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, Tensor(c!) lfu_state, bool stochastic_rounding) -> ()");
  DISPATCH_TO_CUDA("lfu_cache_populate", lfu_cache_populate_cuda);
  m.def(
      "lfu_cache_populate_byte(Tensor weights, Tensor cache_hash_size_cumsum, int total_cache_hash_size, Tensor cache_index_table_map, Tensor weights_offsets, Tensor weights_tys, Tensor D_offsets, Tensor linear_cache_indices, Tensor(a!) lxu_cache_state, Tensor(b!) lxu_cache_weights, Tensor(c!) lfu_state, int row_alignment=16) -> ()");
  DISPATCH_TO_CUDA("lfu_cache_populate_byte", lfu_cache_populate_byte_cuda);
  m.def(
      "lxu_cache_lookup(Tensor linear_cache_indices, Tensor lxu_cache_state, int invalid_index = -1) -> Tensor");
  DISPATCH_TO_CUDA("lxu_cache_lookup", lxu_cache_lookup_cuda);
  m.def(
      "lxu_cache_flush(Tensor(a!) uvm_weights, Tensor cache_hash_size_cumsum, Tensor cache_index_table_map, Tensor weights_offsets, Tensor D_offsets, int total_D, Tensor(b!) lxu_cache_state, Tensor(c!) lxu_cache_weights, bool stochastic_rounding) -> ()");
  DISPATCH_TO_CUDA("lxu_cache_flush", lxu_cache_flush_cuda);
  m.def("lxu_cache_slot(int h_in, int C) -> int");
  DISPATCH_TO_ALL("lxu_cache_slot", host_lxu_cache_slot);
}

} // namespace
