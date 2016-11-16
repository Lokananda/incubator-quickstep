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

#ifndef QUICKSTEP_VIZ_CONFIGS_VIZ_CONFIG_HPP_
#define QUICKSTEP_VIZ_CONFIGS_VIZ_CONFIG_HPP_

#include "catalog/CatalogTypedefs.hpp"
#include "utility/Macros.hpp"
#include "viz/VizContext.hpp"

#include "json.hpp"

namespace quickstep {

class CatalogRelation;
class CatalogRelationSchema;
class StorageManager;

namespace viz {

/** \addtogroup Viz
 *  @{
 */

class VizConfig {
 public:
  virtual ~VizConfig() {}

  virtual nlohmann::json toJSON() = 0;

 protected:
  explicit VizConfig(const VizContextPtr &context);


  nlohmann::json copyColumn(const attribute_id column_id);
  nlohmann::json copySchema(const std::vector<attribute_id> &attr_ids);
  nlohmann::json copyTrace();

  const VizContextPtr context_;
  const CatalogRelation *relation_;
  StorageManager *storage_manager_;

 private:
  DISALLOW_COPY_AND_ASSIGN(VizConfig);
 };

/** @} */

}  // namespace viz
}  // namespace quickstep

#endif  // QUICKSTEP_VIZ_CONFIGS_VIZ_CONFIG_HPP_