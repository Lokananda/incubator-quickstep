/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 **/

#ifndef QUICKSTEP_CATALOG_PARTITION_SCHEME_HEADER_HPP_
#define QUICKSTEP_CATALOG_PARTITION_SCHEME_HEADER_HPP_

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "catalog/Catalog.pb.h"
#include "catalog/CatalogTypedefs.hpp"
#include "types/TypedValue.hpp"
#include "types/operations/comparisons/Comparison.hpp"
#include "types/operations/comparisons/EqualComparison.hpp"
#include "types/operations/comparisons/LessComparison.hpp"
#include "utility/CompositeHash.hpp"
#include "utility/Macros.hpp"

#include "glog/logging.h"

namespace quickstep {

class Type;

/** \addtogroup Catalog
 *  @{
 */

/**
 * @brief The base class which stores the partitioning information for a
 *        particular relation.
 **/
class PartitionSchemeHeader {
 public:
  // A vector for partitioning catalog attributes.
  typedef std::vector<attribute_id> PartitionAttributeIds;

  // The values for partition attributes.
  // PartitionValues.size() should be equal to PartitionAttributeIds.size().
  typedef std::vector<TypedValue> PartitionValues;

  enum PartitionType {
    kHash = 0,
    kRange
  };

  /**
   * @brief Virtual destructor.
   **/
  virtual ~PartitionSchemeHeader() {
  }

  /**
   * @brief Reconstruct a PartitionSchemeHeader from its serialized
   *        Protocol Buffer form.
   *
   * @param proto The Protocol Buffer serialization of a PartitionSchemeHeader,
   *        previously produced by getProto().
   * @return The reconstructed PartitionSchemeHeader object.
   **/
  static PartitionSchemeHeader* ReconstructFromProto(
      const serialization::PartitionSchemeHeader &proto);

  /**
   * @brief Check whether a serialization::PartitionSchemeHeader is fully-formed
   *        and all parts are valid.
   *
   * @param proto A serialized Protocol Buffer representation of a
   *              PartitionSchemeHeader, originally generated by getProto().
   * @return Whether proto is fully-formed and valid.
   **/
  static bool ProtoIsValid(const serialization::PartitionSchemeHeader &proto);

  /**
   * @brief Calculate the partition id into which the attribute value should
   *        be inserted.
   *
   * @param value_of_attributes A vector of attribute values for which the
   *        partition id is to be determined.
   * @return The partition id of the partition for the attribute value.
   **/
  // TODO(gerald): Make this method more efficient since currently this is
  // done for each and every tuple. We can go through the entire set of tuples
  // once using a value accessor and create bitmaps for each partition with
  // tuples that correspond to those partitions.
  virtual partition_id getPartitionId(
      const PartitionValues &value_of_attributes) const = 0;

  /**
   * @brief Serialize the Partition Scheme as Protocol Buffer.
   *
   * @return The Protocol Buffer representation of Partition Scheme.
   **/
  virtual serialization::PartitionSchemeHeader getProto() const;

  /**
   * @brief Get the partition type of the relation.
   *
   * @return The partition type used to partition the relation.
   **/
  inline PartitionType getPartitionType() const {
    return partition_type_;
  }

  /**
   * @brief Get the number of partitions for the relation.
   *
   * @return The number of partitions the relation is partitioned into.
   **/
  inline std::size_t getNumPartitions() const {
    return num_partitions_;
  }

  /**
   * @brief Get the partitioning attributes for the relation.
   *
   * @return The partitioning attributes with which the relation
   *         is partitioned into.
   **/
  inline const PartitionAttributeIds& getPartitionAttributeIds() const {
    return partition_attribute_ids_;
  }

 protected:
  /**
   * @brief Constructor.
   *
   * @param type The type of partitioning to be used to partition the
   *             relation.
   * @param num_partitions The number of partitions to be created.
   * @param attr_ids The attributes on which the partitioning happens.
   **/
  PartitionSchemeHeader(const PartitionType type,
                        const std::size_t num_partitions,
                        PartitionAttributeIds &&attr_ids);  // NOLINT(whitespace/operators)

  // The type of partitioning: Hash or Range.
  const PartitionType partition_type_;
  // The number of partitions.
  const std::size_t num_partitions_;
  // The attribute of partioning.
  const PartitionAttributeIds partition_attribute_ids_;

 private:
  DISALLOW_COPY_AND_ASSIGN(PartitionSchemeHeader);
};

/**
 * @brief Implementation of PartitionSchemeHeader that partitions the tuples in
 *        a relation based on a hash function on the partitioning attribute.
**/
class HashPartitionSchemeHeader final : public PartitionSchemeHeader {
 public:
  /**
   * @brief Constructor.
   *
   * @param num_partitions The number of partitions to be created.
   * @param attributes A vector of attributes on which the partitioning happens.
   **/
  HashPartitionSchemeHeader(const std::size_t num_partitions,
                            PartitionAttributeIds &&attributes)  // NOLINT(whitespace/operators)
      : PartitionSchemeHeader(PartitionType::kHash, num_partitions, std::move(attributes)) {
  }

  /**
   * @brief Destructor.
   **/
  ~HashPartitionSchemeHeader() override {
  }

  partition_id getPartitionId(
      const PartitionValues &value_of_attributes) const override {
    DCHECK_EQ(partition_attribute_ids_.size(), value_of_attributes.size());
    // TODO(gerald): Optimize for the case where the number of partitions is a
    // power of 2. We can just mask out the lower-order hash bits rather than
    // doing a division operation.
    return HashCompositeKey(value_of_attributes) % num_partitions_;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(HashPartitionSchemeHeader);
};

/**
 * @brief Implementation of PartitionSchemeHeader that partitions the tuples in
 *        a relation based on a given value range on the partitioning attribute.
**/
class RangePartitionSchemeHeader final : public PartitionSchemeHeader {
 public:
  /**
   * @brief Constructor.
   *
   * @param num_partitions The number of partitions to be created.
   * @param attributes A vector of attribute_ids on which the partitioning
   *        happens.
   * @param partition_attribute_types The types of CatalogAttributes used for
   *        partitioning.
   * @param partition_ranges The mapping between the partition ids and the upper
   *        bound of the range boundaries. If two ranges R1 and R2 are separated
   *        by a vector of boundary values V, then V would fall into range R2.
   *        For creating a range partition scheme with n partitions, you need to
   *        specify n-1 range boundaries. The first partition will have all the
   *        values less than the first item of range boundaries and the last
   *        partition would have all values greater than or equal to the last
   *        item of range boundaries.
   **/
  RangePartitionSchemeHeader(const std::size_t num_partitions,
                             PartitionAttributeIds &&attributes,  // NOLINT(whitespace/operators)
                             std::vector<const Type*> &&partition_attribute_types,
                             std::vector<PartitionValues> &&partition_ranges)
      : PartitionSchemeHeader(PartitionType::kRange, num_partitions, std::move(attributes)),
        partition_attr_types_(std::move(partition_attribute_types)),
        partition_range_boundaries_(std::move(partition_ranges)) {
    DCHECK_EQ(partition_attribute_ids_.size(), partition_attr_types_.size());
    DCHECK_EQ(num_partitions - 1, partition_range_boundaries_.size());

    const Comparison &less_comparison_op(LessComparison::Instance());
    for (const Type *type : partition_attr_types_) {
      std::unique_ptr<UncheckedComparator> less_unchecked_comparator(
          less_comparison_op.makeUncheckedComparatorForTypes(*type, *type));
      less_unchecked_comparators_.emplace_back(std::move(less_unchecked_comparator));
    }

    const Comparison &equal_comparison_op = EqualComparison::Instance();
    for (const Type *type : partition_attr_types_) {
      std::unique_ptr<UncheckedComparator> equal_unchecked_comparator(
          equal_comparison_op.makeUncheckedComparatorForTypes(*type, *type));
      equal_unchecked_comparators_.emplace_back(std::move(equal_unchecked_comparator));
    }

#ifdef QUICKSTEP_DEBUG
    checkPartitionRangeBoundaries();
#endif
  }

  /**
   * @brief Destructor.
   **/
  ~RangePartitionSchemeHeader() override {
  }

  partition_id getPartitionId(
      const PartitionValues &value_of_attributes) const override {
    DCHECK_EQ(partition_attribute_ids_.size(), value_of_attributes.size());

    partition_id start = 0, end = partition_range_boundaries_.size() - 1;
    if (!lessThan(value_of_attributes, partition_range_boundaries_[end])) {
      return num_partitions_ - 1;
    }

    while (start < end) {
      const partition_id mid = start + ((end - start) >> 1);
      if (lessThan(value_of_attributes, partition_range_boundaries_[mid])) {
        end = mid;
      } else {
        start = mid + 1;
      }
    }

    return start;
  }

  serialization::PartitionSchemeHeader getProto() const override;

  /**
   * @brief Get the range boundaries for partitions.
   *
   * @return The vector of range boundaries for partitions.
   **/
  inline const std::vector<PartitionValues>& getPartitionRangeBoundaries() const {
    return partition_range_boundaries_;
  }

 private:
  /**
   * @brief Check if the partition range boundaries are in ascending order.
   **/
  void checkPartitionRangeBoundaries() {
    for (const PartitionValues &partition_range_boundary : partition_range_boundaries_) {
      CHECK_EQ(partition_attribute_ids_.size(), partition_range_boundary.size())
          << "A partition boundary has different size than that of partition attributes.";
    }

    for (partition_id part_id = 1; part_id < partition_range_boundaries_.size(); ++part_id) {
      CHECK(lessThan(partition_range_boundaries_[part_id - 1], partition_range_boundaries_[part_id]))
          << "Partition boundaries are not in ascending order.";
    }
  }

  /**
   * @brief Check if the partition values are in the lexicographical order.
   *
   * @note (l_0, l_1, ..., l_n) < (r_0, r_1, ..., r_n) iff l_0 < r_0, or
   *       (l_0 == r_0) && (l_1, ..., l_n) < (r_1, ..., r_n).
   **/
  bool lessThan(const PartitionValues &lhs, const PartitionValues &rhs) const {
    DCHECK_EQ(partition_attribute_ids_.size(), lhs.size());
    DCHECK_EQ(partition_attribute_ids_.size(), rhs.size());

    for (std::size_t attr_index = 0; attr_index < partition_attribute_ids_.size(); ++attr_index) {
      if (less_unchecked_comparators_[attr_index]->compareTypedValues(lhs[attr_index], rhs[attr_index])) {
        break;
      } else if (equal_unchecked_comparators_[attr_index]->compareTypedValues(lhs[attr_index], rhs[attr_index])) {
        if (attr_index == partition_attribute_ids_.size() - 1) {
          return false;
        }
      } else {
        return false;
      }
    }

    return true;
  }

  // The size is equal to 'partition_attribute_ids_.size()'.
  const std::vector<const Type*> partition_attr_types_;

  // The boundaries for each range in the RangePartitionSchemeHeader.
  // The upper bound of the range is stored here.
  const std::vector<PartitionValues> partition_range_boundaries_;

  // Both size are equal to 'partition_attr_types_.size()'.
  std::vector<std::unique_ptr<UncheckedComparator>> less_unchecked_comparators_;
  std::vector<std::unique_ptr<UncheckedComparator>> equal_unchecked_comparators_;

  DISALLOW_COPY_AND_ASSIGN(RangePartitionSchemeHeader);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_CATALOG_PARTITION_SCHEME_HEADER_HPP_
