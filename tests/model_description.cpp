#include <filesystem>
#include <functional>
#include <vector>

#include <gtest/gtest.h>
#include <xtypes/idl/idl.hpp>

#include "model-descriptor.hpp"


TEST(ModelDescriptor, NameGenerator)
{
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
       };
    };

)~~~";

  eprosima::xtypes::idl::Context context = eprosima::xtypes::idl::parse(my_idl);

  auto space_type = context.module().structure("Space::Sun");
  eprosima::xtypes::DynamicData data(space_type);

  std::vector<std::string> names;

  data.for_each([&](eprosima::xtypes::DynamicData::ReadableNode& node){
    bool is_leaf = (node.type().is_primitive_type() || node.type().is_enumerated_type());
    bool is_string = node.type().kind() == eprosima::xtypes::TypeKind::STRING_TYPE;
    if(is_leaf || is_string){
      std::string ret;
      name_generator(ret, node);
      names.push_back(ret);
    }
  });

  for(const auto& name : names) {
    std::cout << name << std::endl;
  }

  ASSERT_EQ(names.size(), 4);
  EXPECT_EQ(names[0], std::string("distance"));
  EXPECT_EQ(names[1], std::string("universe[0].my_inner.my_uint32"));
  EXPECT_EQ(names[2], std::string("universe[1].my_inner.my_uint32"));
  EXPECT_EQ(names[3], std::string("name"));
  // TODO: support sequence type, wstring type, map type
}

TEST(ModelDescriptor, ScalarVariable)
{
  rapidxml::xml_document<> doc;
  rapidxml::xml_node<>* root_node = doc.allocate_node(rapidxml::node_element, "ModelVariables");
  doc.append_node(root_node);

  model_variable_generator(doc, root_node, "distance", "output", 0, ddsfmu::Real);
  model_variable_generator(doc, root_node, "distance", "output", 0, ddsfmu::Integer);
  model_variable_generator(doc, root_node, "distance", "output", 0, ddsfmu::Boolean);
  model_variable_generator(doc, root_node, "distance", "output", 0, ddsfmu::String);
  model_variable_generator(doc, root_node, "distance", "input", 0, ddsfmu::Real);


  EXPECT_EQ(print_xml(doc), std::string("<ModelVariables>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<Real start=\"0.0\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<Integer start=\"0\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<Boolean start=\"false\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"output\" initial=\"exact\">\n\t\t<String start=\"\"/>\n\t</ScalarVariable>\n\t<ScalarVariable name=\"distance\" valueReference=\"0\" variability=\"discrete\" causality=\"input\">\n\t\t<Real start=\"0.0\"/>\n\t</ScalarVariable>\n</ModelVariables>\n\n"));

  std::cout << print_xml(doc) << std::endl;

}

TEST(ModelDescriptor, ModelStructure)
{
  rapidxml::xml_document<> doc;
  model_structure_outputs_generator(doc, &doc, 3);

  EXPECT_EQ(print_xml(doc), std::string("<ModelStructure>\n\t<Outputs>\n\t\t<Unknown index=\"1\"/>\n\t\t<Unknown index=\"2\"/>\n\t\t<Unknown index=\"3\"/>\n\t</Outputs>\n</ModelStructure>\n\n"));

}
