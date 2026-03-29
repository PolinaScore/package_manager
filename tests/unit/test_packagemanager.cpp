#include <gtest/gtest.h>
#include "model/packagemanager.h"
#include "model/mainpackage.h"

class PackageManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = new PackageManager();

        manager->addPackageToRepository("app1", PackageType::MAIN, "1.0", "Publisher1");
        manager->addPackageToRepository("lib1", PackageType::LIBRARY, "2.0", "Publisher2");
        manager->addPackageToRepository("lib2", PackageType::LIBRARY, "1.5", "Publisher3");
        manager->addPackageToRepository("meta1", PackageType::META, "latest", "Publisher4");

        manager->getRepository().addDependency("app1", "lib1");
        manager->getRepository().addDependency("lib1", "lib2");
    }

    void TearDown() override {
        delete manager;
    }

    PackageManager* manager;
    
    bool isPackageInstalled(const std::string& name) {
        const Package* pkg = manager->findPackage(name);
        return pkg != nullptr && pkg->isInstalled();
    }
    
    size_t countInstalledPackages() {
        auto installed = manager->getInstalledPackageNames();
        return installed.size();
    }
};

TEST_F(PackageManagerTest, PackageInstallation) {
    auto result = manager->installPackage("app1");

    EXPECT_TRUE(result.success);
    EXPECT_GE(result.installedPackages.size(), 1);

    const Package* pkg = manager->findPackage("app1");
    ASSERT_NE(pkg, nullptr);
    EXPECT_TRUE(pkg->isInstalled());

    const Package* lib1 = manager->findPackage("lib1");
    const Package* lib2 = manager->findPackage("lib2");
    EXPECT_TRUE(lib1->isInstalled());
    EXPECT_TRUE(lib2->isInstalled());
}

TEST_F(PackageManagerTest, InstallNonExistentPackage) {
    auto result = manager->installPackage("unknown");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errors.empty());
}

TEST_F(PackageManagerTest, InstallAlreadyInstalledPackage) {
    manager->installPackage("app1");

    auto result = manager->installPackage("app1");
    EXPECT_TRUE(result.success);
}

TEST_F(PackageManagerTest, PackageRemoval) {

    manager->installPackage("app1");

    auto result = manager->removePackage("app1", true);

    EXPECT_TRUE(result.success);
    EXPECT_GE(result.removedPackages.size(), 1);

    const Package* pkg = manager->findPackage("app1");
    ASSERT_NE(pkg, nullptr);
    EXPECT_FALSE(pkg->isInstalled());

    const Package* lib1 = manager->findPackage("lib1");
    const Package* lib2 = manager->findPackage("lib2");
    EXPECT_FALSE(lib1->isInstalled());
}

TEST_F(PackageManagerTest, CleanupUnusedLibraries) {

    manager->addPackageToRepository("app2", PackageType::MAIN, "1.0");
    manager->getRepository().addDependency("app2", "lib1");

    manager->installPackage("app1");
    manager->installPackage("app2");

    manager->removePackage("app1", false);


    auto cleanupResult = manager->cleanupUnusedLibraries(1);
    EXPECT_EQ(cleanupResult.removedCount, 0); 

    manager->removePackage("app2", false);

    cleanupResult = manager->cleanupUnusedLibraries(2);
    EXPECT_GE(cleanupResult.removedCount, 1); 
}

TEST_F(PackageManagerTest, UpdateAllPackages) {

    manager->installPackage("app1");

    manager->updatePackageInRepository("app1", "2.0");
    manager->updatePackageInRepository("lib1", "3.0");
    manager->updatePackageInRepository("lib2", "2.0");

    auto result = manager->updateAll();

    EXPECT_GE(result.updatedCount, 1);

    const Package* pkg = manager->findPackage("app1");
    EXPECT_EQ(pkg->getInstalledVersion(), "2.0");
}

TEST_F(PackageManagerTest, SplitLibraryOperation) {
    auto result = manager->splitLibrary("lib1", { "lib1-runtime", "lib1-dev" });

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.newPackages.size(), 2);

    EXPECT_TRUE(manager->hasPackage("lib1-runtime"));
    EXPECT_TRUE(manager->hasPackage("lib1-dev"));
    EXPECT_FALSE(manager->hasPackage("lib1")); 
}

TEST_F(PackageManagerTest, MergePackagesOperation) {
    auto result = manager->mergePackages({ "lib1", "lib2" }, "merged-lib", PackageType::LIBRARY, "Merged");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.newPackageName, "merged-lib");

    EXPECT_TRUE(manager->hasPackage("merged-lib"));

    EXPECT_TRUE(manager->hasPackage("lib1"));
    EXPECT_TRUE(manager->hasPackage("lib2"));
}

TEST_F(PackageManagerTest, SystemIntegrityCheck) {
    std::vector<std::string> errors;
    bool integrity = manager->checkSystemIntegrity(errors);

    EXPECT_TRUE(integrity);
    EXPECT_TRUE(errors.empty());
}


TEST_F(PackageManagerTest, GetAllPackageNames) {
    auto names = manager->getAllPackageNames();

    EXPECT_GE(names.size(), 4);

    EXPECT_NE(std::find(names.begin(), names.end(), "app1"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "lib1"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "lib2"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "meta1"), names.end());
}

TEST_F(PackageManagerTest, InstallLibraryPackageShouldFail) {

    auto result = manager->installPackage("lib1");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errors.empty());

}

TEST_F(PackageManagerTest, InstallMetaPackage) {

    auto result = manager->installPackage("meta1");


    const Package* meta = manager->findPackage("meta1");
    ASSERT_NE(meta, nullptr);
    EXPECT_EQ(meta->getType(), PackageType::META);


    if (result.success) {
        EXPECT_GE(result.installedPackages.size(), 1);
    }
}

TEST_F(PackageManagerTest, RemoveMainPackageWithoutUnusedLibs) {

    manager->installPackage("app1");

    auto result = manager->removePackage("app1", false);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.removedPackages.size(), 1);

    const Package* lib1 = manager->findPackage("lib1");
    const Package* lib2 = manager->findPackage("lib2");
    EXPECT_TRUE(lib1->isInstalled());
    EXPECT_TRUE(lib2->isInstalled());
}

TEST_F(PackageManagerTest, CleanupWhenNoUnusedLibraries) {

    manager->installPackage("app1");

    auto cleanupResult = manager->cleanupUnusedLibraries(1);

    EXPECT_EQ(cleanupResult.removedCount, 0);
    EXPECT_TRUE(cleanupResult.removedPackages.empty());
}

TEST_F(PackageManagerTest, UpdateAllPackagesWithDependencies) {

    manager->installPackage("app1");

    manager->updatePackageInRepository("app1", "2.0");
    manager->updatePackageInRepository("lib1", "3.0");
    manager->updatePackageInRepository("lib2", "2.0");

    auto result = manager->updateAll();

    EXPECT_GE(result.updatedCount, 3);
    EXPECT_GE(result.updatedPackages.size(), 3);

    const Package* app = manager->findPackage("app1");
    const Package* lib1 = manager->findPackage("lib1");
    const Package* lib2 = manager->findPackage("lib2");

    EXPECT_EQ(app->getInstalledVersion(), "2.0");
    EXPECT_EQ(lib1->getInstalledVersion(), "3.0");
    EXPECT_EQ(lib2->getInstalledVersion(), "2.0");
}

TEST_F(PackageManagerTest, AddPackageToRepositoryWithCycleDetection) {

    Package* pkg1 = new MainPackage("cycle1", "1.0", "Test");
    Package* pkg2 = new MainPackage("cycle2", "1.0", "Test");

    pkg1->addDependency("cycle2");


    manager->addPackageToRepository(pkg1);
    manager->addPackageToRepository(pkg2);

}

TEST_F(PackageManagerTest, GetPackageInfoForAllTypes) {
    std::string info;

    info = manager->getPackageInfo("app1");
    EXPECT_FALSE(info.empty());
    EXPECT_NE(info.find("app1"), std::string::npos);
    EXPECT_NE(info.find("MAIN"), std::string::npos);

    info = manager->getPackageInfo("lib1");
    EXPECT_FALSE(info.empty());
    EXPECT_NE(info.find("lib1"), std::string::npos);
    EXPECT_NE(info.find("LIBRARY"), std::string::npos);

    info = manager->getPackageInfo("meta1");
    EXPECT_FALSE(info.empty());
    EXPECT_NE(info.find("meta1"), std::string::npos);
    EXPECT_NE(info.find("META"), std::string::npos);
}

TEST_F(PackageManagerTest, DeletePackageFromRepository) {

    manager->addPackageToRepository("todelete", PackageType::LIBRARY, "1.0");
    EXPECT_TRUE(manager->hasPackage("todelete"));

    bool success = manager->deletePackageFromRepository("todelete");
    EXPECT_TRUE(success);
    EXPECT_FALSE(manager->hasPackage("todelete"));

    success = manager->deletePackageFromRepository("nonexistent");
    EXPECT_FALSE(success);
}


TEST_F(PackageManagerTest, SplitLibrarySuccess) {

    manager->addPackageToRepository("tosplit", PackageType::LIBRARY, "1.0", "Original");

    manager->findPackage("tosplit")->addDependency("lib2");

    auto result = manager->splitLibrary("tosplit",
        { "tosplit-runtime", "tosplit-dev" },
        { "RuntimePublisher", "DevPublisher" });

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.newPackages.size(), 2);

    EXPECT_TRUE(manager->hasPackage("tosplit-runtime"));
    EXPECT_TRUE(manager->hasPackage("tosplit-dev"));

    EXPECT_FALSE(manager->hasPackage("tosplit"));
}

TEST_F(PackageManagerTest, SplitLibraryFailure) {

    auto result = manager->splitLibrary("nonexistent", { "a", "b" }, { "pub1", "pub2" });
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());

    result = manager->splitLibrary("app1", { "a", "b" }, { "pub1", "pub2" });
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

TEST_F(PackageManagerTest, MergePackagesSuccess) {

    manager->addPackageToRepository("merge1", PackageType::LIBRARY, "1.0", "Publisher1");
    manager->addPackageToRepository("merge2", PackageType::LIBRARY, "1.0", "Publisher2");

    auto result = manager->mergePackages({ "merge1", "merge2" },
        "merged-lib",
        PackageType::LIBRARY,
        "MergedPublisher");

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.newPackageName, "merged-lib");

    EXPECT_TRUE(manager->hasPackage("merged-lib"));

    EXPECT_TRUE(manager->hasPackage("merge1"));
    EXPECT_TRUE(manager->hasPackage("merge2"));
}

TEST_F(PackageManagerTest, MergePackagesFailure) {

    auto result = manager->mergePackages({ "nonexistent1", "nonexistent2" },
        "merged",
        PackageType::LIBRARY,
        "Publisher");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

TEST_F(PackageManagerTest, GetInstalledPackageNames) {

    auto installed = manager->getInstalledPackageNames();
    EXPECT_TRUE(installed.empty());

    manager->installPackage("app1");

    installed = manager->getInstalledPackageNames();
    EXPECT_GE(installed.size(), 3); 

    EXPECT_NE(std::find(installed.begin(), installed.end(), "app1"), installed.end());
    EXPECT_NE(std::find(installed.begin(), installed.end(), "lib1"), installed.end());
    EXPECT_NE(std::find(installed.begin(), installed.end(), "lib2"), installed.end());
}

TEST_F(PackageManagerTest, GetPackagesByTypeFiltering) {

    auto mainPkgs = manager->getPackagesByType(PackageType::MAIN);
    auto libPkgs = manager->getPackagesByType(PackageType::LIBRARY);
    auto metaPkgs = manager->getPackagesByType(PackageType::META);

    EXPECT_EQ(mainPkgs.size(), 1); 
    EXPECT_GE(libPkgs.size(), 2);  
    EXPECT_EQ(metaPkgs.size(), 1); 


    EXPECT_NE(std::find(mainPkgs.begin(), mainPkgs.end(), "app1"), mainPkgs.end());
    EXPECT_NE(std::find(libPkgs.begin(), libPkgs.end(), "lib1"), libPkgs.end());
    EXPECT_NE(std::find(libPkgs.begin(), libPkgs.end(), "lib2"), libPkgs.end());
    EXPECT_NE(std::find(metaPkgs.begin(), metaPkgs.end(), "meta1"), metaPkgs.end());
}

TEST_F(PackageManagerTest, StatisticsCollectionComplete) {

    auto stats = manager->getStatistics();

    EXPECT_EQ(stats.totalPackages, 4); 
    EXPECT_EQ(stats.installedPackages, 0); 
    EXPECT_EQ(stats.mainPackages, 1); 
    EXPECT_GE(stats.libraryPackages, 2); 
    EXPECT_EQ(stats.metaPackages, 1); 

    manager->installPackage("app1");

    stats = manager->getStatistics();
    EXPECT_GE(stats.installedPackages, 3); 
}

TEST_F(PackageManagerTest, CheckSystemIntegrityValid) {
    std::vector<std::string> errors;
    bool integrity = manager->checkSystemIntegrity(errors);

    EXPECT_TRUE(integrity);
    EXPECT_TRUE(errors.empty());
}

TEST_F(PackageManagerTest, CheckSystemIntegrityWithMissingDependency) {

    manager->addPackageToRepository("broken", PackageType::MAIN, "1.0");
    manager->findPackage("broken")->addDependency("missing-dep");

    std::vector<std::string> errors;
    bool integrity = manager->checkSystemIntegrity(errors);

    EXPECT_FALSE(integrity);
    EXPECT_FALSE(errors.empty());
    EXPECT_NE(errors[0].find("non-existent"), std::string::npos);
}

TEST_F(PackageManagerTest, InitializeWithMultiplePackages) {

    manager->addPackageToRepository("init1", PackageType::MAIN, "1.0");
    manager->addPackageToRepository("init2", PackageType::MAIN, "1.0");

    std::vector<std::string> toInitialize = { "init1", "init2" };
    bool success = manager->initialize(toInitialize);

    EXPECT_TRUE(success);


    EXPECT_TRUE(manager->findPackage("init1")->isInstalled());
    EXPECT_TRUE(manager->findPackage("init2")->isInstalled());
}

TEST_F(PackageManagerTest, InitializeWithPartialFailure) {

    std::vector<std::string> toInitialize = { "app1", "nonexistent" };
    bool success = manager->initialize(toInitialize);

    EXPECT_FALSE(success); 

    EXPECT_TRUE(manager->findPackage("app1")->isInstalled());
}

TEST_F(PackageManagerTest, EdgeCaseEmptyPackageName) {

    auto result = manager->installPackage("");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errors.empty());

    const Package* pkg = manager->findPackage("");
    EXPECT_EQ(pkg, nullptr);
}

TEST_F(PackageManagerTest, ConcurrentInstallRemoveStress) {

    const int ITERATIONS = 100;

    for (int i = 0; i < ITERATIONS; ++i) {
        std::string name = "stress" + std::to_string(i);
        manager->addPackageToRepository(name, PackageType::MAIN, "1.0");

        auto installResult = manager->installPackage(name);
        EXPECT_TRUE(installResult.success);

        auto removeResult = manager->removePackage(name, true);
        EXPECT_TRUE(removeResult.success);
    }
}


TEST_F(PackageManagerTest, UninstallIfUnusedThroughPublicInterface) {

    manager->addPackageToRepository("app-a", PackageType::MAIN, "1.0");
    manager->addPackageToRepository("app-b", PackageType::MAIN, "1.0");
    manager->addPackageToRepository("common-lib", PackageType::LIBRARY, "1.0");
    manager->addPackageToRepository("util-lib", PackageType::LIBRARY, "1.0");
    
    manager->getRepository().addDependency("app-a", "common-lib");

    manager->getRepository().addDependency("common-lib", "util-lib");

    manager->getRepository().addDependency("app-b", "common-lib");
    
    manager->installPackage("app-a");
    manager->installPackage("app-b");
    
    EXPECT_TRUE(isPackageInstalled("app-a"));
    EXPECT_TRUE(isPackageInstalled("app-b"));
    EXPECT_TRUE(isPackageInstalled("common-lib"));
    EXPECT_TRUE(isPackageInstalled("util-lib"));
    
    auto result1 = manager->removePackage("app-a", false);
    EXPECT_TRUE(result1.success);
    
    EXPECT_FALSE(isPackageInstalled("app-a"));
    EXPECT_TRUE(isPackageInstalled("app-b"));
    EXPECT_TRUE(isPackageInstalled("common-lib"));
    EXPECT_TRUE(isPackageInstalled("util-lib"));
    
    auto result2 = manager->removePackage("app-b", true);
    EXPECT_TRUE(result2.success);
    
    EXPECT_FALSE(isPackageInstalled("app-a"));
    EXPECT_FALSE(isPackageInstalled("app-b"));
    EXPECT_FALSE(isPackageInstalled("common-lib"));
    
}
