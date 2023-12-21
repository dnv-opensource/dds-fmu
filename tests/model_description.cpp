/*
  Copyright 2023, SINTEF Ocean
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>

#include "model-descriptor.hpp"


TEST(ModelDescriptor, NameGenerator) {
  std::string my_idl = R"~~~(
    struct Inner
    {
        uint32 my_uint32;
    };

    struct Outer
    {
        Inner my_inner;
    };

    module Space
    {
       struct Sun
       {
         int32 distance;
         Outer universe[2];
         string name;
         uint32 matrix[3][2];
       };
    };

)~~~";

  eprosima::xtypes::idl::Context context;
  context.preprocess = false;

  context = eprosima::xtypes::idl::parse(my_idl, context);

  auto space_type = context.module().structure("Space::Sun");
  eprosima::xtypes::DynamicData data(space_type);

  std::vector<std::string> names;

  data.for_each([&](eprosima::xtypes::DynamicData::ReadableNode& node) {
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
    if (is_leaf || is_string) {
      std::string ret;
      ddsfmu::config::name_generator(ret, node);
      names.push_back(ret);
    }
  });

  for (const auto& name : names) { std::cout << name << std::endl; }

  ASSERT_EQ(names.size(), 10);
  EXPECT_EQ(names[0], std::string("distance"));
  EXPECT_EQ(names[1], std::string("universe[0].my_inner.my_uint32"));
  EXPECT_EQ(names[2], std::string("universe[1].my_inner.my_uint32"));
  EXPECT_EQ(names[3], std::string("name"));
  EXPECT_EQ(names[4], std::string("matrix[0,0]"));
  EXPECT_EQ(names[5], std::string("matrix[0,1]"));
  EXPECT_EQ(names[6], std::string("matrix[1,0]"));
  EXPECT_EQ(names[7], std::string("matrix[1,1]"));
  EXPECT_EQ(names[8], std::string("matrix[2,0]"));
  EXPECT_EQ(names[9], std::string("matrix[2,1]"));

  // TODO: support sequence type, wstring type, map type
}

TEST(ModelDescriptor, ScalarVariable) {
  rapidxml::xml_document<> doc;
  rapidxml::xml_node<>* root_node = doc.allocate_node(rapidxml::node_element, "ModelVariables");
  doc.append_node(root_node);

  namespace ddsconf = ddsfmu::config;
  ddsconf::model_variable_generator(
    doc, root_node, "distance", "output", 0, ddsfmu::config::ScalarVariableType::Real);
  ddsconf::model_variable_generator(
    doc, root_node, "distance", "output", 0, ddsfmu::config::ScalarVariableType::Integer);
  ddsconf::model_variable_generator(
    doc, root_node, "distance", "output", 0, ddsfmu::config::ScalarVariableType::Boolean);
  ddsconf::model_variable_generator(
    doc, root_node, "distance", "output", 0, ddsfmu::config::ScalarVariableType::String);
  ddsconf::model_variable_generator(
    doc, root_node, "distance", "input", 0, ddsfmu::config::ScalarVariableType::Real);

  // clang-format off
  EXPECT_EQ(ddsconf::print_xml(doc), std::string("<ModelVariables>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<Real start=\"0.0\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<Integer start=\"0\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<Boolean start=\"false\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<String start=\"\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"input\">\n\t\t<Real start=\"0.0\"/>\n\t</ScalarVariable>\n</ModelVariables>\n\n"));
  // clang-format on

  std::cout << ddsconf::print_xml(doc) << std::endl;
}

TEST(ModelDescriptor, ModelStructure) {
  rapidxml::xml_document<> doc;
  ddsfmu::config::model_structure_outputs_generator(doc, &doc, 3);

  // clang-format off
  EXPECT_EQ(
    ddsfmu::config::print_xml(doc),
    std::string("<ModelStructure>\n\t<Outputs>\n\t\t<Unknown index=\"1\"/>\n\t\t<Unknown index=\"2\"/>\n\t\t<Unknown index=\"3\"/>\n\t</Outputs>\n</ModelStructure>\n\n"));
  // clang-format on
}
