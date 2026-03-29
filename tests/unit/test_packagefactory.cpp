#include <gtest/gtest.h>
#include "model/packagefactory.h"
#include "model/package.h"

TEST(PackageFactoryTest, CreateMainPackage) {
    Package* pkg = createPackage(PackageType::MAIN, "test-app", "1.0.0", "Test Publisher");
    ASSERT_NE(pkg, nullptr);

    EXPECT_EQ(pkg->getType(), PackageType::MAIN);
    EXPECT_EQ(pkg->getName(), "test-app");
    EXPECT_EQ(pkg->getLatestVersion(), "1.0.0");
    EXPECT_EQ(pkg->getPublisher(), "Test Publisher");

    delete pkg;
}

TEST(PackageFactoryTest, CreateLibraryPackage) {
    Package* pkg = createPackage(PackageType::LIBRARY, "test-lib", "2.0.0", "Lib Publisher");
    ASSERT_NE(pkg, nullptr);

    EXPECT_EQ(pkg->getType(), PackageType::LIBRARY);
    EXPECT_EQ(pkg->getName(), "test-lib");
    EXPECT_EQ(pkg->getLatestVersion(), "2.0.0");
    EXPECT_EQ(pkg->getPublisher(), "Lib Publisher");

    delete pkg;
}

TEST(PackageFactoryTest, CreateMetaPackage) {
    Package* pkg = createPackage(PackageType::META, "test-meta", "latest", "Meta Publisher");
    ASSERT_NE(pkg, nullptr);

    EXPECT_EQ(pkg->getType(), PackageType::META);
    EXPECT_EQ(pkg->getName(), "test-meta");
    EXPECT_EQ(pkg->getLatestVersion(), "latest");
    EXPECT_EQ(pkg->getPublisher(), "Meta Publisher");

    delete pkg;
}

TEST(PackageFactoryTest, CreateFromString) {
    Package* pkg1 = createPackage("MAIN", "app", "1.0");
    ASSERT_NE(pkg1, nullptr);
    EXPECT_EQ(pkg1->getType(), PackageType::MAIN);
    delete pkg1;

    Package* pkg2 = createPackage("LIBRARY", "lib", "2.0");
    ASSERT_NE(pkg2, nullptr);
    EXPECT_EQ(pkg2->getType(), PackageType::LIBRARY);
    delete pkg2;

    Package* pkg3 = createPackage("META", "meta", "latest");
    ASSERT_NE(pkg3, nullptr);
    EXPECT_EQ(pkg3->getType(), PackageType::META);
    delete pkg3;

    Package* pkg4 = createPackage("UNKNOWN", "unknown", "1.0");
    if (pkg4) {
        EXPECT_EQ(pkg4->getType(), PackageType::MAIN);
        delete pkg4;
    }
}