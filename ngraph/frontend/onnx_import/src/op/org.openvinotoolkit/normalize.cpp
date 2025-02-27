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

#include "onnx_import/op/org.openvinotoolkit/normalize.hpp"
#include "ngraph/op/normalize_l2.hpp"
#include "onnx_import/default_opset.hpp"
#include "onnx_import/utils/common.hpp"

namespace ngraph
{
    namespace onnx_import
    {
        namespace op
        {
            namespace set_1
            {
                OutputVector normalize(const Node& node)
                {
                    auto inputs = node.get_ng_inputs();
                    NGRAPH_CHECK(inputs.size() == 2, "Invalid number of inputs");

                    auto data = inputs[0];
                    float eps = node.get_attribute_value<float>("eps", 0);
                    int64_t across_spatial = node.get_attribute_value<int64_t>("across_spatial", 0);
                    int64_t channel_shared = node.get_attribute_value<int64_t>("channel_shared", 0);

                    std::shared_ptr<ngraph::Node> weights;
                    if (channel_shared)
                    {
                        NGRAPH_CHECK(
                            ngraph::op::is_constant(inputs[1].get_node()),
                            "Weights input must be a constant if channel_shared is set to 1");
                        const auto& shape = inputs[1].get_partial_shape();
                        NGRAPH_CHECK(
                            shape.is_static() && shape.rank().get_length() == 1,
                            "Weights rank must be equal to 1 if channel_shared is set to 1");
                        weights = inputs[1].get_node_shared_ptr();
                    }
                    else
                    {
                        std::vector<int64_t> weights_shape{1};
                        const auto& data_shape = inputs[0].get_partial_shape();
                        if (data_shape[1].is_static())
                        {
                            weights_shape.push_back(data_shape[1].get_length());
                        }
                        else
                        {
                            weights_shape.push_back(0);
                        }
                        for (size_t i = 2; i < data_shape.rank().get_length(); ++i)
                        {
                            weights_shape.push_back(1);
                        }
                        auto new_shape = std::make_shared<default_opset::Constant>(
                            element::i64, Shape{weights_shape.size()}, weights_shape);
                        weights =
                            std::make_shared<default_opset::Reshape>(inputs[1], new_shape, true);
                    }

                    std::shared_ptr<ngraph::Node> axes;
                    if (!across_spatial)
                    {
                        axes = std::make_shared<default_opset::Constant>(
                            element::i64, Shape{1}, std::vector<int64_t>{1});
                    }
                    else
                    {
                        axes = common::get_monotonic_range_along_node_rank(data, 1);
                    }

                    return {std::make_shared<default_opset::Multiply>(
                        std::make_shared<default_opset::NormalizeL2>(
                            data, axes, eps, ngraph::op::EpsMode::ADD),
                        weights)};
                }

            } // namespace set_1
        }     // namespace op

    } // namespace onnx_import

} // namespace ngraph
