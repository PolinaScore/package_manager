#include <gtest/gtest.h>
#include "model/metapackage.h"

class MetaPackageTest : public ::testing::Test {
protected:
    void SetUp() override {
        meta = new MetaPackage("python-meta", "latest", "Python Foundation");
    }

    void TearDown() override {
        delete meta;
    }

    MetaPackage* meta;
};

TEST_F(MetaPackageTest, LinkedPackageManagement) {
    EXPECT_FALSE(meta->hasLinkedPackage());
    EXPECT_TRUE(meta->getLinkedPackageName().empty());

    meta->setLinkedPackageName("python-3.11");
    EXPECT_TRUE(meta->hasLinkedPackage());
    EXPECT_EQ(meta->getLinkedPackageName(), "python-3.11");

    meta->setLinkedPackageName("");
    EXPECT_FALSE(meta->hasLinkedPackage());
}


TEST_F(MetaPackageTest, CloneMetaPackage) {
    meta->setLinkedPackageName("python-3.11");

    MetaPackage* cloned = dynamic_cast<MetaPackage*>(meta->clone());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->getName(), "python-meta");
    EXPECT_EQ(cloned->getLinkedPackageName(), "python-3.11");
    EXPECT_TRUE(cloned->hasLinkedPackage());

    delete cloned;
}