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

#include <memory>

#include "ngraph/op/non_max_suppression.hpp"
#include "ngraph/op/util/attr_types.hpp"
#include "onnx_import/default_opset.hpp"
#include "onnx_import/exceptions.hpp"
#include "onnx_import/op/non_max_suppression.hpp"
#include "onnx_import/utils/reshape.hpp"

namespace ngraph
{
    namespace onnx_import
    {
        namespace op
        {
            namespace set_1
            {
                OutputVector non_max_suppression(const Node& node)
                {
                    // TODO: this op will not be tested until at least
                    //       a reference implementation is added

                    const auto ng_inputs = node.get_ng_inputs();
                    const Output<ngraph::Node> boxes = ng_inputs.at(0);
                    const Output<ngraph::Node> scores = ng_inputs.at(1);

                    Output<ngraph::Node> max_output_boxes_per_class;
                    if (ng_inputs.size() > 2)
                    {
                        max_output_boxes_per_class =
                            ngraph::onnx_import::reshape::interpret_as_scalar(ng_inputs.at(2));
                    }
                    else
                    {
                        max_output_boxes_per_class =
                            default_opset::Constant::create(element::i64, Shape{}, {0});
                    }

                    Output<ngraph::Node> iou_threshold;
                    if (ng_inputs.size() > 3)
                    {
                        iou_threshold =
                            ngraph::onnx_import::reshape::interpret_as_scalar(ng_inputs.at(3));
                    }
                    else
                    {
                        iou_threshold =
                            default_opset::Constant::create(element::f32, Shape{}, {.0f});
                    }

                    Output<ngraph::Node> score_threshold;
                    if (ng_inputs.size() > 4)
                    {
                        score_threshold =
                            ngraph::onnx_import::reshape::interpret_as_scalar(ng_inputs.at(4));
                    }
                    else
                    {
                        score_threshold =
                            default_opset::Constant::create(element::f32, Shape{}, {.0f});
                    }

                    const auto center_point_box =
                        node.get_attribute_value<std::int64_t>("center_point_box", 0);

                    CHECK_VALID_NODE(
                        node,
                        center_point_box == 0 || center_point_box == 1,
                        "Allowed values of the 'center_point_box' attribute are 0 and 1.");

                    const auto box_encoding =
                        center_point_box == 0
                            ? default_opset::NonMaxSuppression::BoxEncodingType::CORNER
                            : default_opset::NonMaxSuppression::BoxEncodingType::CENTER;

                    return {std::make_shared<default_opset::NonMaxSuppression>(
                        boxes,
                        scores,
                        max_output_boxes_per_class,
                        iou_threshold,
                        score_threshold,
                        box_encoding,
                        false)};
                }

            } // namespace set_1

        } // namespace op

    } // namespace onnx_import

} // namespace ngraph
