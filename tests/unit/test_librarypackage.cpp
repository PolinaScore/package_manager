#include <gtest/gtest.h>
#include "model/librarypackage.h"

class LibraryPackageTest : public ::testing::Test {
protected:
    void SetUp() override {
        library = new LibraryPackage("openssl", "3.0.7", "OpenSSL Foundation");
        library->addDependency("zlib");
        library->addDependency("libssl");
    }

    void TearDown() override {
        delete library;
    }

    LibraryPackage* library;
};

TEST_F(LibraryPackageTest, LibrarySpecificProperties) {
    EXPECT_EQ(library->getType(), PackageType::LIBRARY);
    EXPECT_EQ(library->getTypeName(), "LIBRARY");
    EXPECT_FALSE(library->canBeInstalled()); 
}

TEST_F(LibraryPackageTest, SplitLibrary) {
    std::vector<std::string> newParts = { "openssl-runtime", "openssl-dev", "openssl-debug" };
    std::vector<std::string> publishers = { "OpenSSL", "OpenSSL Dev", "OpenSSL Debug" };

    auto splitResult = library->split(newParts, publishers);
    EXPECT_EQ(splitResult.size(), 3);

    for (size_t i = 0; i < splitResult.size(); ++i) {
        LibraryPackage* part = splitResult[i];
        EXPECT_EQ(part->getName(), newParts[i]);
        EXPECT_EQ(part->getLatestVersion(), "3.0.7");
        EXPECT_EQ(part->getPublisher(), publishers[i]);
        EXPECT_EQ(part->getDependencies().size(), 2); 
        EXPECT_TRUE(part->dependsOn("zlib"));
        EXPECT_TRUE(part->dependsOn("libssl"));

        delete part; 
    }
}

TEST_F(LibraryPackageTest, SplitWithDefaultPublisher) {
    std::vector<std::string> newParts = { "part1", "part2" };

    auto splitResult = library->split(newParts);
    EXPECT_EQ(splitResult.size(), 2);

    for (size_t i = 0; i < splitResult.size(); ++i) {
        LibraryPackage* part = splitResult[i];
        EXPECT_EQ(part->getPublisher(), "OpenSSL Foundation");
        delete part;
    }
}

TEST_F(LibraryPackageTest, SplitEmptyNames) {
    std::vector<std::string> empty;

    auto splitResult = library->split(empty);
    EXPECT_TRUE(splitResult.empty());
}

TEST_F(LibraryPackageTest, IsUsedMethod) {
    EXPECT_FALSE(library->isUsed());
}

TEST_F(LibraryPackageTest, CloneLibrary) {
    LibraryPackage* cloned = dynamic_cast<LibraryPackage*>(library->clone());
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->getName(), "openssl");
    EXPECT_EQ(cloned->getLatestVersion(), "3.0.7");
    EXPECT_EQ(cloned->getPublisher(), "OpenSSL Foundation");
    EXPECT_EQ(cloned->getDependencies().size(), 2);

    delete cloned;
}