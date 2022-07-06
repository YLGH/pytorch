#include <gtest/gtest.h>
#include <torch/csrc/autograd/generated/variable_factories.h>
#include <torch/csrc/utils/schema_info.h>

namespace torch {
namespace utils {
TEST(FunctionSchemaIsMutableTest, Basic) {
  c10::FunctionSchema schema =
      torch::jit::getOperatorForLiteral(
          "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))")
          ->schema();
  ASSERT_TRUE(schema.is_mutable(0));
  ASSERT_TRUE(schema.is_mutable("self"));
  ASSERT_FALSE(schema.is_mutable(1));
  ASSERT_FALSE(schema.is_mutable("other"));
  ASSERT_FALSE(schema.is_mutable(2));
  ASSERT_FALSE(schema.is_mutable("alpha"));
}

TEST(FunctionSchemaIsMutableTest, InvalidArgument) {
  c10::FunctionSchema schema =
      torch::jit::getOperatorForLiteral(
          "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))")
          ->schema();
  ASSERT_THROW(schema.is_mutable(4), c10::Error);
  ASSERT_THROW(schema.is_mutable("named_argument"), c10::Error);
}

TEST(SchemaInfoIsMutableTest, Basic) {
  SchemaInfo schema(
      "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))");
  ASSERT_TRUE(schema.is_mutable(0));
  ASSERT_TRUE(schema.is_mutable("self"));
  ASSERT_FALSE(schema.is_mutable(1));
  ASSERT_FALSE(schema.is_mutable("other"));
  ASSERT_FALSE(schema.is_mutable(2));
  ASSERT_FALSE(schema.is_mutable("alpha"));
}

TEST(SchemaInfoIsMutableTest, InvalidArgument) {
  SchemaInfo schema(
      "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))");
  ASSERT_THROW(schema.is_mutable(4), c10::Error);
  ASSERT_THROW(schema.is_mutable("named_argument"), c10::Error);
}

TEST(SchemaInfoIsMutableTest, AliasingInputs) {
  SchemaInfo schema(
      "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))");
  ASSERT_TRUE(schema.is_mutable(0));
  ASSERT_TRUE(schema.is_mutable("self"));
  ASSERT_FALSE(schema.is_mutable(1));
  ASSERT_FALSE(schema.is_mutable("other"));
  at::Tensor input = at::randn({3, 3});
  schema.addArgumentValue("self", input);
  schema.addArgumentValue("other", input);
  ASSERT_TRUE(schema.is_mutable(1));
  ASSERT_TRUE(schema.is_mutable("other"));
}

TEST(FunctionSchemaAreAliasingTest, Basic) {
  c10::FunctionSchema schema =
      torch::jit::getOperatorForLiteral(
          "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))")
          ->schema();
  ASSERT_TRUE(schema.areAliasing({c10::input, 0}, {c10::output, 0}));
  ASSERT_FALSE(schema.areAliasing({c10::input, 1}, {c10::output, 0}));
  ASSERT_FALSE(schema.areAliasing({c10::input, 1}, {c10::input, 0}));
}

TEST(FunctionSchemaAreAliasingTest, InvalidArgument) {
  c10::FunctionSchema schema =
      torch::jit::getOperatorForLiteral(
          "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))")
          ->schema();
  ASSERT_THROW(
      schema.areAliasing({c10::input, 15}, {c10::output, 0}), c10::Error);
  ASSERT_THROW(
      schema.areAliasing({c10::input, 0}, {c10::output, 15}), c10::Error);
}

TEST(FunctionSchemaAreAliasingTest, Wildcard) {
  c10::FunctionSchema schema =
      torch::jit::getOperatorForLiteral(
          "aten::split.Tensor(Tensor(a -> *) self, int split_size, int dim=0) -> Tensor(a)[]")
          ->schema();
  ASSERT_TRUE(schema.areAliasing({c10::input, 0}, {c10::output, 0}, true));
  ASSERT_FALSE(schema.areAliasing({c10::input, 0}, {c10::output, 0}, false));
}

TEST(SchemaInfoAreAliasingTest, AliasingInputs) {
  SchemaInfo schema(
      "aten::sub.Tensor(Tensor self, Tensor other, *, Scalar alpha=1) -> Tensor");
  ASSERT_FALSE(schema.areAliasing({c10::input, 0}, {c10::input, 1}, true));
  at::Tensor input = at::randn({3, 3});
  schema.addArgumentValue("self", input);
  schema.addArgumentValue("other", input);
  ASSERT_TRUE(schema.areAliasing({c10::input, 0}, {c10::input, 1}, true));
}

TEST(SchemaInfoAreAliasingTest, AliasingOutputs) {
  SchemaInfo schema(
      "aten::aminmax.out(Tensor self, *, int? dim=None, bool keepdim=False, Tensor(a!) min, Tensor(b!) max) -> (Tensor(a!) min, Tensor(b!) max)");
  ASSERT_FALSE(schema.areAliasing({c10::output, 0}, {c10::output, 1}, true));
  at::Tensor input = at::randn({3, 3});
  schema.addArgumentValue("min", input);
  schema.addArgumentValue("max", input);
  ASSERT_TRUE(schema.areAliasing({c10::output, 0}, {c10::output, 1}, true));
}

TEST(SchemaInfoAreAliasingTest, AliasingInputOutput) {
  SchemaInfo schema(
      "aten::sub_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> (Tensor(a!))");
  ASSERT_TRUE(schema.areAliasing({c10::input, 0}, {c10::output, 0}, true));
  ASSERT_FALSE(schema.areAliasing({c10::input, 1}, {c10::output, 0}, true));
  at::Tensor input = at::randn({3, 3});
  schema.addArgumentValue("self", input);
  schema.addArgumentValue("other", input);
  ASSERT_TRUE(schema.areAliasing({c10::input, 0}, {c10::output, 0}, true));
  ASSERT_TRUE(schema.areAliasing({c10::input, 1}, {c10::output, 0}, true));
  ASSERT_FALSE(schema.areAliasing({c10::input, 1}, {c10::output, 0}, false));
}

} // namespace utils
} // namespace torch
