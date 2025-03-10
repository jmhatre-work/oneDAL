/*******************************************************************************
* Copyright 2023 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#pragma once

#include "oneapi/dal/table/common.hpp"
#include "oneapi/dal/detail/array_utils.hpp"

namespace oneapi::dal {
namespace v1 {

class ONEDAL_EXPORT csr_table : public table {
    friend detail::pimpl_accessor;

public:
    /// Returns the unique id of ``csr_table`` class.
    static std::int64_t kind();

    /// Creates a new ``csr_table`` instance from externally-defined data, columns indices
    /// and row offsets memory blocks.
    /// Table object refers to the memory blocks but does not own them. The responsibility to
    /// free the memory blocks remains on the user side.
    /// The :literal:`data` should point to the ``data_pointer`` memory block.
    /// The :literal:`columns_indices` should point to the ``column_indices_pointer`` memory block.
    /// The :literal:`row_offsets` should point to the ``row_offsets_pointer`` memory block.
    ///
    /// @tparam Data        The type of elements in the data block that will be stored into the
    ///                     table. The table initializes data types of metadata with this data type.
    ///                     The feature types should be set to default values for :literal:`Data` type:
    ///                     contiguous for floating-point, ordinal for integer types. The :literal:`Data`
    ///                     type should be at least :expr:`float`, :expr:`double` or :expr:`std::int32_t`.
    ///
    /// @param data_pointer           The pointer to values block in the CSR layout.
    /// @param column_indices_pointer The pointer to column indices block in the CSR layout.
    /// @param row_offsets_pointer    The pointer to row offsets block in CSR layout.
    /// @param row_count              The number of rows in the table.
    /// @param column_count           The number of columns in the table.
    /// @param indexing               The indexing scheme used to access data in the CSR layout.
    ///                               Should be :literal:`sparse_indexing::zero_based` or
    ///                               :literal:`sparse_indexing::one_based`.
    template <typename Data>
    static csr_table wrap(const Data* data_pointer,
                          const std::int64_t* column_indices_pointer,
                          const std::int64_t* row_offsets_pointer,
                          std::int64_t row_count,
                          std::int64_t column_count,
                          sparse_indexing indexing = sparse_indexing::one_based) {
        return csr_table{ data_pointer,
                          column_indices_pointer,
                          row_offsets_pointer,
                          row_count,
                          column_count,
                          dal::detail::empty_delete<const Data>(),
                          dal::detail::empty_delete<const std::int64_t>(),
                          dal::detail::empty_delete<const std::int64_t>(),
                          indexing };
    }

    /// Creates a new ``csr_table`` instance with zero number of rows and columns.
    /// The :expr:`kind` is set to``csr_table::kind()``.
    /// All the properties should be set to default values (see the Properties section).
    csr_table();

    /// Casts an object of the base table type to a CSR table. If cast is
    /// not possible, the operation is equivalent to a default constructor call.
    explicit csr_table(const table& other);

    /// Creates a new ``csr_table`` instance from externally-defined data blocks.
    /// Table object owns the data, row indices and column indices pointers.
    ///
    /// @tparam Data          The type of elements in the data block that will be stored into the table.
    ///                       The :literal:`Data` type should be at least :expr:`float`, :expr:`double` or :expr:`std::int32_t`.
    ///
    /// @param data           The array of values in the CSR layout.
    /// @param column_indices The array of column indices in the CSR layout.
    /// @param row_offsets    The array of row offsets in the CSR layout.
    /// @param column_count   The number of columns in the corresponding dense table.
    /// @param indexing       The indexing scheme used to access data in the CSR layout.
    ///                       Should be :literal:`sparse_indexing::zero_based` or
    ///                       :literal:`sparse_indexing::one_based`.
    template <typename Data>
    csr_table(const dal::array<Data>& data,
              const dal::array<std::int64_t>& column_indices,
              const dal::array<std::int64_t>& row_offsets,
              std::int64_t column_count,
              sparse_indexing indexing = sparse_indexing::one_based) {
        init_impl(data, column_indices, row_offsets, column_count, indexing);
    }

    /// Creates a new ``csr_table`` instance from externally-defined data blocks.
    /// Table object owns the data, column indices and row offsets pointers.
    /// The :literal:`data` should point to the ``data_pointer`` memory block.
    /// The :literal:`column_indices` should point to the ``column_indices_pointer`` memory block.
    /// The :literal:`row_offsets` should point to the ``row_offsets_pointer`` memory block.
    ///
    /// @tparam Data         The type of elements in the data block that will be stored into the table.
    ///                      The :literal:`Data` type should be at least :expr:`float`, :expr:`double`
    ///                      or :expr:`std::int32_t`.
    /// @tparam ConstDataDeleter
    ///                      The type of a deleter called on ``data_pointer`` when
    ///                      the last table that refers it is out of the scope.
    /// @tparam ConstColumnIndicesDeleter
    ///                      The type of a deleter called on ``column_indices_pointer`` when
    ///                      the last table that refers it is out of the scope.
    /// @tparam ConstRowOffsetsDeleter
    ///                      The type of a deleter called on ``row_offsets_pointer`` when
    ///                      the last table that refers it is out of the scope.
    ///
    /// @param data_pointer           The pointer to values block in the CSR layout.
    /// @param column_indices_pointer The pointer to column indices block in the CSR layout.
    /// @param row_offsets_pointer    The pointer to row offsets block in the CSR layout.
    /// @param row_count              The number of rows in the table.
    /// @param column_count           The number of columns in the table.
    /// @param data_deleter           The deleter that is called on the ``data_pointer``
    ///                               when the last table that refers it is out of the scope.
    /// @param column_indices_deleter The deleter that is called on the ``column_indices_pointer``
    ///                               when the last table that refers it is out of the scope.
    /// @param row_offsets_deleter    The deleter that is called on the ``row_offsets_pointer``
    ///                               when the last table that refers it is out of the scope.
    /// @param indexing               The indexing scheme used to access data in the CSR layout.
    ///                               Should be :literal:`sparse_indexing::zero_based` or
    ///                               :literal:`sparse_indexing::one_based`.
    template <typename Data,
              typename ConstDataDeleter,
              typename ConstColumnIndicesDeleter,
              typename ConstRowOffsetsDeleter>
    csr_table(const Data* data_pointer,
              const std::int64_t* column_indices_pointer,
              const std::int64_t* row_offsets_pointer,
              std::int64_t row_count,
              std::int64_t column_count,
              ConstDataDeleter&& data_deleter,
              ConstColumnIndicesDeleter&& column_indices_deleter,
              ConstRowOffsetsDeleter&& row_offsets_deleter,
              sparse_indexing indexing = sparse_indexing::one_based) {
        init_impl(detail::default_host_policy{},
                  data_pointer,
                  column_indices_pointer,
                  row_offsets_pointer,
                  row_count,
                  column_count,
                  std::forward<ConstDataDeleter>(data_deleter),
                  std::forward<ConstColumnIndicesDeleter>(column_indices_deleter),
                  std::forward<ConstRowOffsetsDeleter>(row_offsets_deleter),
                  indexing);
    }

    /// Creates a new ``csr_table`` instance from arrays of data, column indices and row offsets.
    /// The created table shares data ownership with the given arrays.
    ///
    /// @tparam Data        The type of elements in the data block that will be stored into the
    ///                     table. The table initializes data types of metadata with this data type.
    ///                     The feature types should be set to default values for :literal:`Data` type:
    ///                     contiguous for floating-point, ordinal for integer types. The :literal:`Data`
    ///                     type should be at least :expr:`float`, :expr:`double` or :expr:`std::int32_t`.
    ///
    /// @param data             The array that stores values block in the CSR layout.
    /// @param column_indices   The array that stores column indices block in the CSR layout.
    /// @param row_offsets      The array that stores row offsets block in the CSR layout.
    /// @param column_count     The number of columns in the table.
    /// @param indexing         The indexing scheme used to access data in the CSR layout.
    ///                         Should be :literal:`sparse_indexing::zero_based` or
    ///                         :literal:`sparse_indexing::one_based`.
    template <typename Data>
    static csr_table wrap(const dal::array<Data>& data,
                          const dal::array<std::int64_t>& column_indices,
                          const dal::array<std::int64_t>& row_offsets,
                          std::int64_t column_count,
                          sparse_indexing indexing = sparse_indexing::one_based) {
        return csr_table{ data, column_indices, row_offsets, column_count, indexing };
    }

    /// The unique id of the csr table type.
    std::int64_t get_kind() const {
        return kind();
    }

    /// The number of non-zero elements in the table.
    /// @remark default = 0
    std::int64_t get_non_zero_count() const;

    /// The indexing scheme used to access data in the CSR layout.
    /// @remark default = :expr:`sparse_indexing::one_based`
    sparse_indexing get_indexing() const;

    /// Returns the :literal:`data` pointer cast to the :literal:`Data` type. No checks are
    /// performed that this type is the actual type of the data within the table.
    template <typename Data>
    const Data* get_data() const {
        return reinterpret_cast<const Data*>(this->get_data());
    }

    /// The pointer to the data block within the table.
    /// Should be equal to ``nullptr`` when :expr:`row_count == 0` and :expr:`column_count == 0`.
    const void* get_data() const;

    /// Returns the :literal:`column_indices` pointer.
    const std::int64_t* get_column_indices() const;

    /// Returns the :literal:`row_offsets` pointer.
    const std::int64_t* get_row_offsets() const;

private:
    explicit csr_table(detail::csr_table_iface* impl) : table(impl) {}

    void check_indices(const std::int64_t* row_offsets,
                       const std::int64_t row_count,
                       const std::int64_t column_count,
                       const sparse_indexing indexing) const {
        using error_msg = dal::detail::error_messages;
        const std::int64_t min_index = (indexing == sparse_indexing::zero_based) ? 0 : 1;
        const std::int64_t max_row_offset = row_offsets[row_count];

        if (row_count <= 0) {
            throw dal::domain_error(error_msg::rc_leq_zero());
        }

        if (column_count <= 0) {
            throw dal::domain_error(error_msg::cc_leq_zero());
        }

        if (row_offsets[0] != min_index) {
            throw dal::domain_error(error_msg::invalid_first_row_offset());
        }

        if (row_offsets[row_count] > max_row_offset) {
            throw dal::domain_error(error_msg::row_offsets_gt_max_value());
        }
    }

    template <typename Policy,
              typename Data,
              typename ConstDataDeleter,
              typename ConstColumnIndicesDeleter,
              typename ConstRowOffsetsDeleter>
    void init_impl(const Policy& policy,
                   const Data* data_pointer,
                   const std::int64_t* column_indices_pointer,
                   const std::int64_t* row_offsets_pointer,
                   std::int64_t row_count,
                   std::int64_t column_count,
                   ConstDataDeleter&& data_deleter,
                   ConstColumnIndicesDeleter&& column_indices_deleter,
                   ConstRowOffsetsDeleter&& row_offsets_deleter,
                   sparse_indexing indexing) {
        check_indices(row_offsets_pointer, row_count, column_count, indexing);
        const std::int64_t element_count = row_offsets_pointer[row_count] - row_offsets_pointer[0];

        const auto data =
            detail::array_via_policy<Data>::wrap(policy,
                                                 data_pointer,
                                                 element_count,
                                                 std::forward<ConstDataDeleter>(data_deleter));

        const auto column_indices = detail::array_via_policy<std::int64_t>::wrap(
            policy,
            column_indices_pointer,
            element_count,
            std::forward<ConstColumnIndicesDeleter>(column_indices_deleter));

        const auto row_offsets = detail::array_via_policy<std::int64_t>::wrap(
            policy,
            row_offsets_pointer,
            row_count + 1,
            std::forward<ConstRowOffsetsDeleter>(row_offsets_deleter));

        init_impl(policy,
                  detail::reinterpret_array_cast<byte_t>(data),
                  column_indices,
                  row_offsets,
                  column_count,
                  detail::make_data_type<Data>(),
                  indexing);
    }

    template <typename Data>
    void init_impl(const dal::array<Data>& data,
                   const dal::array<std::int64_t>& column_indices,
                   const dal::array<std::int64_t>& row_offsets,
                   std::int64_t column_count,
                   sparse_indexing indexing) {
        std::int64_t row_count = row_offsets.get_count();
        row_count = (row_count ? row_count - 1 : std::int64_t(0));
        check_indices(row_offsets.get_data(), row_count, column_count, indexing);

        return init_impl(detail::default_host_policy{},
                         detail::reinterpret_array_cast<byte_t>(data),
                         column_indices,
                         row_offsets,
                         column_count,
                         detail::make_data_type<Data>(),
                         indexing);
    }

    template <typename Policy>
    void init_impl(const Policy& policy,
                   const dal::array<byte_t>& data,
                   const dal::array<std::int64_t>& column_indices,
                   const dal::array<std::int64_t>& row_offsets,
                   std::int64_t column_count,
                   const data_type& dtype,
                   sparse_indexing indexing);
};

} // namespace v1

using v1::sparse_indexing;
using v1::csr_table;

} // namespace oneapi::dal
