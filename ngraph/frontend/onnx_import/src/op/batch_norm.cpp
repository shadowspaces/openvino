//*****************************************************************************
// Copyright 2017-2021 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include <cstdint>
#include <memory>

#include "onnx_import/core/null_node.hpp"
#include "onnx_import/default_opset.hpp"
#include "onnx_import/exceptions.hpp"
#include "onnx_import/op/batch_norm.hpp"

namespace ngraph
{
    namespace onnx_import
    {
        namespace op
        {
            namespace set_1
            {
                OutputVector batch_norm(const Node& node)
                {
                    OutputVector inputs{node.get_ng_inputs()};
                    auto x = inputs.at(0);
                    auto scale = inputs.at(1);
                    auto bias = inputs.at(2);
                    Output<ngraph::Node> mean;
                    Output<ngraph::Node> var;

                    std::int64_t is_test{node.get_attribute_value<std::int64_t>("is_test", 1)};
                    double epsilon{node.get_attribute_value<double>("epsilon", 1e-5)};

                    // TODO: Implement learning mode support
                    // float momentum{node.get_attribute_value<float>("momentum", 0.9f)};
                    CHECK_VALID_NODE(node, is_test, "only 'is_test' mode is supported.");

                    // optional outputs
                    auto after_bn_mean = std::make_shared<NullNode>();
                    auto after_bn_var = std::make_shared<NullNode>();
                    auto saved_mean = std::make_shared<NullNode>();
                    auto saved_var = std::make_shared<NullNode>();

                    if (inputs.size() >= 5)
                    {
                        mean = inputs.at(3);
                        var = inputs.at(4);
                        return {std::make_shared<default_opset::BatchNormInference>(
                                    x, scale, bias, mean, var, epsilon),
                                after_bn_mean,
                                after_bn_var,
                                saved_mean,
                                saved_var};
                    }

                    throw ngraph_error(
                        "Cannot create nGraph batch norm with unsupported number of inputs");
                }

            } // namespace set_1

        } // namespace op

    } // namespace onnx_import

} // namespace ngraph
