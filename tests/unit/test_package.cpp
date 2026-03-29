#include <gtest/gtest.h>
#include "model/package.h"
#include "model/mainpackage.h"
#include "model/librarypackage.h"
#include "model/metapackage.h"

class PackageTest : public ::testing::Test {
protected:
    void SetUp() override {
        mainPackage = new MainPackage("test-main", "1.0.0", "Test Publisher");
        libraryPackage = new LibraryPackage("test-lib", "2.0.0", "Library Publisher");
        metaPackage = new MetaPackage("test-meta", "latest", "Meta Publisher");
    }

    void TearDown() override {
        delete mainPackage;
        delete libraryPackage;
        delete metaPackage;
    }

    Package* mainPackage;
    Package* libraryPackage;
    Package* metaPackage;
};

TEST_F(PackageTest, BasicPackageProperties) {
    EXPECT_EQ(mainPackage->getName(), "test-main");
    EXPECT_EQ(mainPackage->getLatestVersion(), "1.0.0");
    EXPECT_EQ(mainPackage->getPublisher(), "Test Publisher");
    EXPECT_FALSE(mainPackage->isInstalled());
}

TEST_F(PackageTest, PackageTypes) {
    EXPECT_EQ(mainPackage->getType(), PackageType::MAIN);
    EXPECT_EQ(mainPackage->getTypeName(), "MAIN");

    EXPECT_EQ(libraryPackage->getType(), PackageType::LIBRARY);
    EXPECT_EQ(libraryPackage->getTypeName(), "LIBRARY");

    EXPECT_EQ(metaPackage->getType(), PackageType::META);
    EXPECT_EQ(metaPackage->getTypeName(), "META");
}


TEST_F(PackageTest, InstallationStatus) {
    mainPackage->setInstalledVersion("1.0.0");
    EXPECT_TRUE(mainPackage->isInstalled());
    EXPECT_EQ(mainPackage->getInstalledVersion(), "1.0.0");

    mainPackage->setInstalledVersion("");
    EXPECT_FALSE(mainPackage->isInstalled());
}

TEST_F(PackageTest, DependenciesManagement) {
    mainPackage->addDependency("dependency1");
    mainPackage->addDependency("dependency2");
    mainPackage->addDependency("dependency1");

    EXPECT_EQ(mainPackage->getDependencies().size(), 2);
    EXPECT_TRUE(mainPackage->dependsOn("dependency1"));
    EXPECT_TRUE(mainPackage->dependsOn("dependency2"));
    EXPECT_FALSE(mainPackage->dependsOn("dependency3"));

    mainPackage->removeDependency("dependency1");
    EXPECT_EQ(mainPackage->getDependencies().size(), 1);
    EXPECT_FALSE(mainPackage->dependsOn("dependency1"));

    mainPackage->clearDependencies();
    EXPECT_TRUE(mainPackage->getDependencies().empty());
}

TEST_F(PackageTest, ClonePackage) {
    mainPackage->addDependency("dep1");
    mainPackage->setInstalledVersion("1.0.0");

    Package* cloned = mainPackage->clone();
    ASSERT_NE(cloned, nullptr);

    EXPECT_EQ(cloned->getName(), "test-main");
    EXPECT_EQ(cloned->getLatestVersion(), "1.0.0");
    EXPECT_EQ(cloned->getInstalledVersion(), "1.0.0");
    EXPECT_EQ(cloned->getDependencies().size(), 1);
    EXPECT_TRUE(cloned->dependsOn("dep1"));

    delete cloned;
}

TEST_F(PackageTest, ToStringFormat) {
    std::string str = mainPackage->toString();
    EXPECT_NE(str.find("Package: test-main"), std::string::npos);
    EXPECT_NE(str.find("Type: MAIN"), std::string::npos);
    EXPECT_NE(str.find("Latest version: 1.0.0"), std::string::npos);
}

TEST_F(PackageTest, TypeConversion) {
    EXPECT_EQ(Package::typeToString(PackageType::MAIN), "MAIN");
    EXPECT_EQ(Package::typeToString(PackageType::LIBRARY), "LIBRARY");
    EXPECT_EQ(Package::typeToString(PackageType::META), "META");

    EXPECT_EQ(Package::stringToType("MAIN"), PackageType::MAIN);
    EXPECT_EQ(Package::stringToType("LIBRARY"), PackageType::LIBRARY);
    EXPECT_EQ(Package::stringToType("META"), PackageType::META);
    EXPECT_EQ(Package::stringToType("unknown"), PackageType::MAIN); 
}

TEST_F(PackageTest, PackageConstructorWithEmptyPublisher) {
    Package* noPublisher = new MainPackage("no-publisher", "1.0");

    EXPECT_EQ(noPublisher->getName(), "no-publisher");
    EXPECT_EQ(noPublisher->getLatestVersion(), "1.0");
    EXPECT_EQ(noPublisher->getPublisher(), "");
    EXPECT_FALSE(noPublisher->isInstalled());

    delete noPublisher;
}

TEST_F(PackageTest, PackageConstructorWithEmptyVersion) {
    Package* noVersion = new MainPackage("no-version", "", "Publisher");

    EXPECT_EQ(noVersion->getName(), "no-version");
    EXPECT_EQ(noVersion->getLatestVersion(), ""); 
    EXPECT_EQ(noVersion->getPublisher(), "Publisher");
    EXPECT_FALSE(noVersion->isInstalled());

    delete noVersion;
}

TEST_F(PackageTest, SetLatestVersion) {
    EXPECT_EQ(mainPackage->getLatestVersion(), "1.0.0");

    mainPackage->setLatestVersion("2.0.0");
    EXPECT_EQ(mainPackage->getLatestVersion(), "2.0.0");

    mainPackage->setLatestVersion("");
    EXPECT_EQ(mainPackage->getLatestVersion(), "");
}

TEST_F(PackageTest, DependencyRemovalNonExistent) {
    mainPackage->addDependency("existing-dep");
    size_t initialSize = mainPackage->getDependencies().size();

    mainPackage->removeDependency("non-existent-dependency");

    EXPECT_EQ(mainPackage->getDependencies().size(), initialSize);
    EXPECT_TRUE(mainPackage->dependsOn("existing-dep"));
}

TEST_F(PackageTest, RemoveDependencyFromEmptyList) {
    EXPECT_TRUE(mainPackage->getDependencies().empty());
    mainPackage->removeDependency("any");
    EXPECT_TRUE(mainPackage->getDependencies().empty());
}

TEST_F(PackageTest, ClearDependenciesOnEmptyList) {
    EXPECT_TRUE(mainPackage->getDependencies().empty());
    mainPackage->clearDependencies();
    EXPECT_TRUE(mainPackage->getDependencies().empty());
}

TEST_F(PackageTest, DependsOnEmptyList) {
    EXPECT_FALSE(mainPackage->dependsOn("any-package"));
    EXPECT_FALSE(mainPackage->dependsOn(""));
}

TEST_F(PackageTest, DependsOnWithEmptyString) {
    mainPackage->addDependency("valid-dep");
    EXPECT_FALSE(mainPackage->dependsOn(""));
}

TEST_F(PackageTest, DuplicateDependenciesIgnored) {
    mainPackage->addDependency("lib1");
    mainPackage->addDependency("lib1"); 
    mainPackage->addDependency("lib1"); 

    EXPECT_EQ(mainPackage->getDependencies().size(), 1);
    EXPECT_TRUE(mainPackage->dependsOn("lib1"));
}

TEST_F(PackageTest, DependencyOrderPreservation) {
    std::vector<std::string> expectedOrder = { "first", "second", "third", "fourth" };

    for (const auto& dep : expectedOrder) {
        mainPackage->addDependency(dep);
    }

    const auto& deps = mainPackage->getDependencies();
    ASSERT_EQ(deps.size(), expectedOrder.size());

    for (size_t i = 0; i < expectedOrder.size(); ++i) {
        EXPECT_EQ(deps[i], expectedOrder[i]) << "Mismatch at position " << i;
    }
}

TEST_F(PackageTest, LargeNumberOfDependencies) {
    const int NUM_DEPS = 100;

    for (int i = 0; i < NUM_DEPS; ++i) {
        mainPackage->addDependency("dep-" + std::to_string(i));
    }

    EXPECT_EQ(mainPackage->getDependencies().size(), NUM_DEPS);

    EXPECT_TRUE(mainPackage->dependsOn("dep-0"));
    EXPECT_TRUE(mainPackage->dependsOn("dep-50"));
    EXPECT_TRUE(mainPackage->dependsOn("dep-99"));
    EXPECT_FALSE(mainPackage->dependsOn("dep-100")); 
    EXPECT_FALSE(mainPackage->dependsOn("dep-"));
    EXPECT_FALSE(mainPackage->dependsOn(""));
}

TEST_F(PackageTest, PackageToStringWithDependencies) {
    mainPackage->addDependency("lib1");
    mainPackage->addDependency("lib2");
    mainPackage->setInstalledVersion("1.0.0");

    std::string str = mainPackage->toString();

    EXPECT_NE(str.find("test-main"), std::string::npos);
    EXPECT_NE(str.find("MAIN"), std::string::npos);

    EXPECT_FALSE(str.empty());
}

TEST_F(PackageTest, ClonePreservesAllProperties) {
    mainPackage->addDependency("dep1");
    mainPackage->addDependency("dep2");
    mainPackage->setInstalledVersion("1.0.0");
    mainPackage->setLatestVersion("2.0.0");

    Package* cloned = mainPackage->clone();

    EXPECT_EQ(cloned->getName(), "test-main");
    EXPECT_EQ(cloned->getType(), PackageType::MAIN);
    EXPECT_EQ(cloned->getLatestVersion(), "2.0.0");
    EXPECT_EQ(cloned->getInstalledVersion(), "1.0.0");
    EXPECT_EQ(cloned->getPublisher(), "Test Publisher");

    EXPECT_EQ(cloned->getDependencies().size(), 2);
    EXPECT_TRUE(cloned->dependsOn("dep1"));
    EXPECT_TRUE(cloned->dependsOn("dep2"));

    cloned->addDependency("dep3");
    EXPECT_EQ(cloned->getDependencies().size(), 3);
    EXPECT_EQ(mainPackage->getDependencies().size(), 2); 

    mainPackage->removeDependency("dep1");
    mainPackage->setInstalledVersion("1.5.0");

    EXPECT_EQ(cloned->getDependencies().size(), 3); 
    EXPECT_TRUE(cloned->dependsOn("dep1")); 
    EXPECT_EQ(cloned->getInstalledVersion(), "1.0.0"); 

    delete cloned;
}

TEST_F(PackageTest, CloneDifferentPackageTypes) {
    Package* clonedMain = mainPackage->clone();
    Package* clonedLib = libraryPackage->clone();
    Package* clonedMeta = metaPackage->clone();

    EXPECT_EQ(clonedMain->getType(), PackageType::MAIN);
    EXPECT_EQ(clonedLib->getType(), PackageType::LIBRARY);
    EXPECT_EQ(clonedMeta->getType(), PackageType::META);

    EXPECT_EQ(clonedMain->getName(), "test-main");
    EXPECT_EQ(clonedLib->getName(), "test-lib");
    EXPECT_EQ(clonedMeta->getName(), "test-meta");

    delete clonedMain;
    delete clonedLib;
    delete clonedMeta;
}

TEST_F(PackageTest, TypeConversionEdgeCases) {
    EXPECT_EQ(Package::stringToType("UNKNOWN"), PackageType::MAIN);
    EXPECT_EQ(Package::stringToType(""), PackageType::MAIN);
    EXPECT_EQ(Package::stringToType("random"), PackageType::MAIN);
    EXPECT_EQ(Package::stringToType("invalid"), PackageType::MAIN);

    EXPECT_EQ(Package::stringToType("main"), PackageType::MAIN);
    EXPECT_EQ(Package::stringToType("Main"), PackageType::MAIN);
    EXPECT_EQ(Package::stringToType("MAIN"), PackageType::MAIN);

    EXPECT_EQ(Package::stringToType("library"), PackageType::LIBRARY);
    EXPECT_EQ(Package::stringToType("LIBRARY"), PackageType::LIBRARY);

    EXPECT_EQ(Package::stringToType("meta"), PackageType::META);
    EXPECT_EQ(Package::stringToType("META"), PackageType::META);
}

TEST_F(PackageTest, TypeToStringEdgeCases) {
    EXPECT_EQ(Package::typeToString(PackageType::MAIN), "MAIN");
    EXPECT_EQ(Package::typeToString(PackageType::LIBRARY), "LIBRARY");
    EXPECT_EQ(Package::typeToString(PackageType::META), "META");
}

TEST_F(PackageTest, CanBeInstalledForDifferentTypes) {

    EXPECT_TRUE(mainPackage->canBeInstalled());
    EXPECT_FALSE(libraryPackage->canBeInstalled()); 
    EXPECT_TRUE(metaPackage->canBeInstalled()); 

    mainPackage->setInstalledVersion("1.0.0");
    EXPECT_TRUE(mainPackage->canBeInstalled()); 

    libraryPackage->setInstalledVersion("2.0.0"); 
    EXPECT_FALSE(libraryPackage->canBeInstalled()); 
}

TEST_F(PackageTest, PackageWithSpecialCharacters) {

    Package* special = new MainPackage("test-package_123", "1.2.3-beta", "Publisher & Co.");

    EXPECT_EQ(special->getName(), "test-package_123");
    EXPECT_EQ(special->getLatestVersion(), "1.2.3-beta");
    EXPECT_EQ(special->getPublisher(), "Publisher & Co.");

    special->addDependency("lib-c++");
    special->addDependency("python3.8");
    special->addDependency("dot.net");

    EXPECT_TRUE(special->dependsOn("lib-c++"));
    EXPECT_TRUE(special->dependsOn("python3.8"));
    EXPECT_TRUE(special->dependsOn("dot.net"));

    delete special;
}

TEST_F(PackageTest, PackagePolymorphism) {

    Package* pkg1 = new MainPackage("main", "1.0", "Publisher1");
    Package* pkg2 = new LibraryPackage("lib", "2.0", "Publisher2");
    Package* pkg3 = new MetaPackage("meta", "latest", "Publisher3");

    std::vector<Package*> packages = { pkg1, pkg2, pkg3 };

    for (auto pkg : packages) {

        EXPECT_FALSE(pkg->getName().empty());
        EXPECT_FALSE(pkg->getLatestVersion().empty());
        EXPECT_FALSE(pkg->getTypeName().empty());

        std::string str = pkg->toString();
        EXPECT_FALSE(str.empty());
        EXPECT_NE(str.find(pkg->getName()), std::string::npos);

        Package* cloned = pkg->clone();
        EXPECT_EQ(cloned->getType(), pkg->getType());
        EXPECT_EQ(cloned->getName(), pkg->getName());
        delete cloned;
    }

    delete pkg1;
    delete pkg2;
    delete pkg3;
}

TEST_F(PackageTest, PackageEquality) {

    Package* pkg1 = new MainPackage("same-name", "1.0", "Pub1");
    Package* pkg2 = new MainPackage("same-name", "2.0", "Pub2"); 

    EXPECT_EQ(pkg1->getName(), pkg2->getName());

    EXPECT_NE(pkg1, pkg2);

    Package* pkg3 = new MainPackage("different-name", "1.0", "Pub1");
    EXPECT_NE(pkg1->getName(), pkg3->getName());

    delete pkg1;
    delete pkg2;
    delete pkg3;
}

TEST_F(PackageTest, DependencyChain) {

    mainPackage->addDependency("lib1");

    Package* lib1 = new LibraryPackage("lib1", "1.0");
    lib1->addDependency("lib2");

    EXPECT_TRUE(mainPackage->dependsOn("lib1"));
    EXPECT_FALSE(mainPackage->dependsOn("lib2")); 

    EXPECT_TRUE(lib1->dependsOn("lib2"));

    delete lib1;
}

TEST_F(PackageTest, RemoveMiddleDependency) {

    mainPackage->addDependency("first");
    mainPackage->addDependency("second");
    mainPackage->addDependency("third");

    EXPECT_EQ(mainPackage->getDependencies().size(), 3);

    mainPackage->removeDependency("second");

    EXPECT_EQ(mainPackage->getDependencies().size(), 2);
    EXPECT_TRUE(mainPackage->dependsOn("first"));
    EXPECT_FALSE(mainPackage->dependsOn("second"));
    EXPECT_TRUE(mainPackage->dependsOn("third"));

    const auto& deps = mainPackage->getDependencies();
    EXPECT_EQ(deps[0], "first");
    EXPECT_EQ(deps[1], "third");
}

TEST_F(PackageTest, RemoveFirstDependency) {

    mainPackage->addDependency("first");
    mainPackage->addDependency("second");
    mainPackage->addDependency("third");

    mainPackage->removeDependency("first");

    EXPECT_EQ(mainPackage->getDependencies().size(), 2);
    EXPECT_FALSE(mainPackage->dependsOn("first"));
    EXPECT_TRUE(mainPackage->dependsOn("second"));
    EXPECT_TRUE(mainPackage->dependsOn("third"));

    const auto& deps = mainPackage->getDependencies();
    EXPECT_EQ(deps[0], "second");
    EXPECT_EQ(deps[1], "third");
}

TEST_F(PackageTest, RemoveLastDependency) {

    mainPackage->addDependency("first");
    mainPackage->addDependency("second");
    mainPackage->addDependency("third");

    mainPackage->removeDependency("third");

    EXPECT_EQ(mainPackage->getDependencies().size(), 2);
    EXPECT_TRUE(mainPackage->dependsOn("first"));
    EXPECT_TRUE(mainPackage->dependsOn("second"));
    EXPECT_FALSE(mainPackage->dependsOn("third"));

    const auto& deps = mainPackage->getDependencies();
    EXPECT_EQ(deps[0], "first");
    EXPECT_EQ(deps[1], "second");
}

TEST_F(PackageTest, EmptyPackageName) {

    Package* emptyName = new MainPackage("", "1.0", "Publisher");

    EXPECT_EQ(emptyName->getName(), "");
    EXPECT_EQ(emptyName->getLatestVersion(), "1.0");
    EXPECT_EQ(emptyName->getPublisher(), "Publisher");

    delete emptyName;
}

TEST_F(PackageTest, VersionUpdateScenarios) {

    mainPackage->setInstalledVersion("1.0");
    EXPECT_TRUE(mainPackage->isInstalled());
    EXPECT_EQ(mainPackage->getInstalledVersion(), "1.0");

    mainPackage->setInstalledVersion("1.0");
    EXPECT_TRUE(mainPackage->isInstalled());
    EXPECT_EQ(mainPackage->getInstalledVersion(), "1.0");

    mainPackage->setInstalledVersion("2.0");
    EXPECT_TRUE(mainPackage->isInstalled());
    EXPECT_EQ(mainPackage->getInstalledVersion(), "2.0");

    mainPackage->setInstalledVersion("1.5");
    EXPECT_TRUE(mainPackage->isInstalled());
    EXPECT_EQ(mainPackage->getInstalledVersion(), "1.5");

    mainPackage->setInstalledVersion("");
    EXPECT_FALSE(mainPackage->isInstalled());
    EXPECT_EQ(mainPackage->getInstalledVersion(), "");
}

TEST_F(PackageTest, ConcurrentDependencyOperations) {

    const int NUM_OPERATIONS = 100;

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        std::string depName = "dep-" + std::to_string(i);

        mainPackage->addDependency(depName);
        EXPECT_TRUE(mainPackage->dependsOn(depName));

        if (i % 2 == 0) {
            mainPackage->removeDependency(depName);
            EXPECT_FALSE(mainPackage->dependsOn(depName));
        }
    }

    EXPECT_EQ(mainPackage->getDependencies().size(), NUM_OPERATIONS / 2);
}

TEST_F(PackageTest, PackageComparison) {

    Package* pkg1 = new MainPackage("pkg1", "1.0", "Pub1");
    Package* pkg2 = new MainPackage("pkg1", "2.0", "Pub2"); 
    Package* pkg3 = new MainPackage("pkg2", "1.0", "Pub1"); 

    EXPECT_EQ(pkg1->getName(), pkg2->getName());

    EXPECT_NE(pkg1->getLatestVersion(), pkg2->getLatestVersion());

    EXPECT_NE(pkg1->getName(), pkg3->getName());

    delete pkg1;
    delete pkg2;
    delete pkg3;
}

TEST_F(PackageTest, PackageFactoryPatternTest) {

    std::vector<std::pair<PackageType, std::string>> testCases = {
        {PackageType::MAIN, "MAIN"},
        {PackageType::LIBRARY, "LIBRARY"},
        {PackageType::META, "META"}
    };

    for (const auto& [type, typeName] : testCases) {
        Package* pkg = nullptr;

        switch (type) {
        case PackageType::MAIN:
            pkg = new MainPackage("test", "1.0", "Publisher");
            break;
        case PackageType::LIBRARY:
            pkg = new LibraryPackage("test", "1.0", "Publisher");
            break;
        case PackageType::META:
            pkg = new MetaPackage("test", "latest", "Publisher");
            break;
        }

        ASSERT_NE(pkg, nullptr);
        EXPECT_EQ(pkg->getType(), type);
        EXPECT_EQ(pkg->getTypeName(), typeName);

        delete pkg;
    }
}


TEST_F(PackageTest, MainPackageSpecificBehavior) {
    MainPackage* mainPkg = new MainPackage("specific-main", "3.0", "Specific Publisher");

    EXPECT_EQ(mainPkg->getType(), PackageType::MAIN);
    EXPECT_TRUE(mainPkg->canBeInstalled());


    mainPkg->addDependency("some-lib");
    EXPECT_TRUE(mainPkg->dependsOn("some-lib"));

    delete mainPkg;
}

TEST_F(PackageTest, LibraryPackageSpecificBehavior) {
    LibraryPackage* libPkg = new LibraryPackage("specific-lib", "4.0", "Lib Publisher");

    EXPECT_EQ(libPkg->getType(), PackageType::LIBRARY);
    EXPECT_FALSE(libPkg->canBeInstalled()); 


    libPkg->addDependency("another-lib");
    EXPECT_TRUE(libPkg->dependsOn("another-lib"));

    delete libPkg;
}

TEST_F(PackageTest, MetaPackageSpecificBehavior) {
    MetaPackage* metaPkg = new MetaPackage("specific-meta", "latest", "Meta Publisher");

    EXPECT_EQ(metaPkg->getType(), PackageType::META);
    EXPECT_TRUE(metaPkg->canBeInstalled());


    metaPkg->addDependency("main-package");
    EXPECT_TRUE(metaPkg->dependsOn("main-package"));

    delete metaPkg;
}

TEST_F(PackageTest, MixedPackageOperations) {

    std::vector<Package*> packages = {
        new MainPackage("mixed-main", "1.0"),
        new LibraryPackage("mixed-lib", "2.0"),
        new MetaPackage("mixed-meta", "latest")
    };

    for (auto pkg : packages) {

        pkg->setInstalledVersion(pkg->getLatestVersion());
        EXPECT_TRUE(pkg->isInstalled() || pkg->getLatestVersion().empty());


        pkg->addDependency("common-dep");
        EXPECT_TRUE(pkg->dependsOn("common-dep"));


        Package* cloned = pkg->clone();
        EXPECT_EQ(cloned->getType(), pkg->getType());
        delete cloned;
    }


    for (auto pkg : packages) {
        delete pkg;
    }
}




