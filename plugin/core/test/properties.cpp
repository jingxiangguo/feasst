#include <gtest/gtest.h>
#include "core/include/properties.h"
#include "core/include/debug.h"

TEST(Properties, properties) {
  feasst::Properties properties;
  try {
    properties.value("bananas");
    CATCH_PHRASE("property not found");
  }
  double value;
  EXPECT_FALSE(properties.value("bananas", &value));
  properties.add_or_set("bananas", 12);
  EXPECT_TRUE(properties.value("bananas", &value));
  EXPECT_NEAR(properties.value("bananas"), 12, 1e-15);
  try {
    properties.set("apples", 2.3);
    CATCH_PHRASE("property not found");
  }
  properties.set("bananas", 2.3);
  EXPECT_NEAR(properties.value("bananas"), 2.3, 1e-15);
  try {
    properties.add("bananas", 12);
    CATCH_PHRASE("already exists");
  }
}