#include <gtest/gtest.h>
#include "model/repository.h"
#include "model/mainpackage.h"
#include "model/librarypackage.h"
#include "model/metapackage.h"
#include "model/packagefactory.h"

class RepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        repo = new Repository();
    }

    void TearDown() override {
        delete repo;
    }

    Repository* repo;
};


TEST_F(RepositoryTest, EmptyRepository) {
    EXPECT_EQ(repo->getAllPackageNames().size(), 0);
    EXPECT_FALSE(repo->hasPackage("any"));
    EXPECT_EQ(repo->getPackage("any"), nullptr);
    EXPECT_FALSE(repo->hasCycles());
}

TEST_F(RepositoryTest, AddAndGetPackage) {
    auto* pkg = new MainPackage("test-pkg", "1.0", "TestPub");
    EXPECT_TRUE(repo->addPackage(pkg));

    EXPECT_TRUE(repo->hasPackage("test-pkg"));
    EXPECT_FALSE(repo->hasPackage("non-existent"));

    Package* retrieved = repo->getPackage("test-pkg");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "test-pkg");
    EXPECT_EQ(retrieved->getType(), PackageType::MAIN);
    EXPECT_EQ(retrieved->getLatestVersion(), "1.0");
    EXPECT_EQ(retrieved->getPublisher(), "TestPub");
}

TEST_F(RepositoryTest, AddPackageNullptr) {
    EXPECT_FALSE(repo->addPackage(nullptr));
}

TEST_F(RepositoryTest, AddDuplicatePackage) {
    auto* pkg1 = new MainPackage("duplicate", "1.0", "Pub");
    auto* pkg2 = new MainPackage("duplicate", "2.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
    EXPECT_FALSE(repo->addPackage(pkg2));
    EXPECT_EQ(repo->getAllPackageNames().size(), 1);

}

TEST_F(RepositoryTest, AddMetaPackageWithoutLinkedPackage) {
    auto* meta = new MetaPackage("meta-pkg", "latest", "MetaPub");

    EXPECT_TRUE(repo->addPackage(meta));
    EXPECT_TRUE(repo->hasPackage("meta-pkg"));
}

TEST_F(RepositoryTest, RemovePackage) {
    auto* pkg = new MainPackage("to-remove", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg));
    EXPECT_TRUE(repo->hasPackage("to-remove"));

    EXPECT_TRUE(repo->removePackage("to-remove"));
    EXPECT_FALSE(repo->hasPackage("to-remove"));
    EXPECT_EQ(repo->getAllPackageNames().size(), 0);
}

TEST_F(RepositoryTest, RemoveNonExistentPackage) {
    EXPECT_FALSE(repo->removePackage("non-existent"));
}

TEST_F(RepositoryTest, RemovePackageWithReverseDependencies) {
    auto* depender = new MainPackage("depender", "1.0", "Pub");
    auto* depended = new MainPackage("depended", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(depender));
    EXPECT_TRUE(repo->addPackage(depended));
    EXPECT_TRUE(repo->addDependency("depender", "depended"));


    EXPECT_FALSE(repo->removePackage("depended"));
    EXPECT_TRUE(repo->hasPackage("depended"));
}

TEST_F(RepositoryTest, RemovePackageCleansReverseDeps) {
    auto* pkg1 = new MainPackage("pkg1", "1.0", "Pub");
    auto* pkg2 = new MainPackage("pkg2", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
    EXPECT_TRUE(repo->addPackage(pkg2));
    EXPECT_TRUE(repo->addDependency("pkg1", "pkg2"));

    EXPECT_TRUE(repo->removeDependency("pkg1", "pkg2"));

    EXPECT_TRUE(repo->removePackage("pkg2"));
    EXPECT_FALSE(repo->hasPackage("pkg2"));
}


TEST_F(RepositoryTest, UpdatePackage) {
    auto* pkg = new MainPackage("updatable", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg));

    EXPECT_TRUE(repo->updatePackage("updatable", "2.0"));

    Package* updated = repo->getPackage("updatable");
    ASSERT_NE(updated, nullptr);
    EXPECT_EQ(updated->getLatestVersion(), "2.0");
}

TEST_F(RepositoryTest, UpdateNonExistentPackage) {
    EXPECT_FALSE(repo->updatePackage("non-existent", "2.0"));
}


TEST_F(RepositoryTest, AddDependency) {
    auto* from = new MainPackage("from-pkg", "1.0", "Pub");
    auto* to = new MainPackage("to-pkg", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(from));
    EXPECT_TRUE(repo->addPackage(to));

    EXPECT_TRUE(repo->addDependency("from-pkg", "to-pkg"));

    auto deps = repo->getDependencies("from-pkg");
    EXPECT_EQ(deps.size(), 1);
    EXPECT_EQ(deps[0], "to-pkg");

    auto reverseDeps = repo->getReverseDependencies("to-pkg");
    EXPECT_EQ(reverseDeps.size(), 1);
    EXPECT_EQ(reverseDeps[0], "from-pkg");
}

TEST_F(RepositoryTest, AddDependencyNonExistentPackages) {
    EXPECT_FALSE(repo->addDependency("non-existent", "to-pkg"));

    auto* from = new MainPackage("from-pkg", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(from));
    EXPECT_FALSE(repo->addDependency("from-pkg", "non-existent"));
}

TEST_F(RepositoryTest, AddDuplicateDependency) {
    auto* from = new MainPackage("from-pkg", "1.0", "Pub");
    auto* to = new MainPackage("to-pkg", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(from));
    EXPECT_TRUE(repo->addPackage(to));

    EXPECT_TRUE(repo->addDependency("from-pkg", "to-pkg"));
    EXPECT_TRUE(repo->addDependency("from-pkg", "to-pkg"));

    auto deps = repo->getDependencies("from-pkg");
    EXPECT_EQ(deps.size(), 1); 
}

TEST_F(RepositoryTest, RemoveDependency) {
    auto* from = new MainPackage("from-pkg", "1.0", "Pub");
    auto* to = new MainPackage("to-pkg", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(from));
    EXPECT_TRUE(repo->addPackage(to));
    EXPECT_TRUE(repo->addDependency("from-pkg", "to-pkg"));

    EXPECT_TRUE(repo->removeDependency("from-pkg", "to-pkg"));
    EXPECT_TRUE(repo->getDependencies("from-pkg").empty());
    EXPECT_TRUE(repo->getReverseDependencies("to-pkg").empty());
}

TEST_F(RepositoryTest, RemoveNonExistentDependency) {
    auto* from = new MainPackage("from-pkg", "1.0", "Pub");
    auto* to = new MainPackage("to-pkg", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(from));
    EXPECT_TRUE(repo->addPackage(to));

    EXPECT_FALSE(repo->removeDependency("from-pkg", "to-pkg"));

    EXPECT_FALSE(repo->removeDependency("non-existent", "to-pkg"));
}


TEST_F(RepositoryTest, ReverseDependenciesMultiple) {
    auto* dep1 = new MainPackage("dep1", "1.0", "Pub");
    auto* dep2 = new MainPackage("dep2", "1.0", "Pub");
    auto* target = new MainPackage("target", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(dep1));
    EXPECT_TRUE(repo->addPackage(dep2));
    EXPECT_TRUE(repo->addPackage(target));

    EXPECT_TRUE(repo->addDependency("dep1", "target"));
    EXPECT_TRUE(repo->addDependency("dep2", "target"));

    auto reverseDeps = repo->getReverseDependencies("target");
    EXPECT_EQ(reverseDeps.size(), 2);
    EXPECT_NE(std::find(reverseDeps.begin(), reverseDeps.end(), "dep1"), reverseDeps.end());
    EXPECT_NE(std::find(reverseDeps.begin(), reverseDeps.end(), "dep2"), reverseDeps.end());
}

TEST_F(RepositoryTest, GetDependenciesNonExistentPackage) {
    auto deps = repo->getDependencies("non-existent");
    EXPECT_TRUE(deps.empty());
}

TEST_F(RepositoryTest, GetReverseDependenciesNonExistentPackage) {
    auto reverseDeps = repo->getReverseDependencies("non-existent");
    EXPECT_TRUE(reverseDeps.empty());
}


TEST_F(RepositoryTest, NoCyclesInEmptyRepository) {
    EXPECT_FALSE(repo->hasCycles());
}

TEST_F(RepositoryTest, NoCyclesLinearDependencies) {
    auto* pkg1 = new MainPackage("pkg1", "1.0", "Pub");
    auto* pkg2 = new MainPackage("pkg2", "1.0", "Pub");
    auto* pkg3 = new MainPackage("pkg3", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
    EXPECT_TRUE(repo->addPackage(pkg2));
    EXPECT_TRUE(repo->addPackage(pkg3));

    EXPECT_TRUE(repo->addDependency("pkg1", "pkg2"));
    EXPECT_TRUE(repo->addDependency("pkg2", "pkg3"));

    EXPECT_FALSE(repo->hasCycles());
    EXPECT_FALSE(repo->hasCycleFromPackage("pkg1"));
}

TEST_F(RepositoryTest, DetectDirectCycle) {
    auto* pkg1 = new MainPackage("pkg1", "1.0", "Pub");
    auto* pkg2 = new MainPackage("pkg2", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
    EXPECT_TRUE(repo->addPackage(pkg2));

    EXPECT_TRUE(repo->addDependency("pkg1", "pkg2"));

    EXPECT_FALSE(repo->addDependency("pkg2", "pkg1"));

    EXPECT_FALSE(repo->hasCycles());
}

TEST_F(RepositoryTest, DetectIndirectCycle) {
    auto* pkg1 = new MainPackage("pkg1", "1.0", "Pub");
    auto* pkg2 = new MainPackage("pkg2", "1.0", "Pub");
    auto* pkg3 = new MainPackage("pkg3", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
    EXPECT_TRUE(repo->addPackage(pkg2));
    EXPECT_TRUE(repo->addPackage(pkg3));

    EXPECT_TRUE(repo->addDependency("pkg1", "pkg2"));
    EXPECT_TRUE(repo->addDependency("pkg2", "pkg3"));

    EXPECT_FALSE(repo->addDependency("pkg3", "pkg1"));

    EXPECT_FALSE(repo->hasCycles());
}

TEST_F(RepositoryTest, HasCycleFromPackageNonExistent) {
    EXPECT_FALSE(repo->hasCycleFromPackage("non-existent"));
}


TEST_F(RepositoryTest, GetAllTransitiveDependencies) {
    auto* main = new MainPackage("main", "1.0", "Pub");
    auto* lib1 = new LibraryPackage("lib1", "1.0", "Pub");
    auto* lib2 = new LibraryPackage("lib2", "1.0", "Pub");
    auto* lib3 = new LibraryPackage("lib3", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(main));
    EXPECT_TRUE(repo->addPackage(lib1));
    EXPECT_TRUE(repo->addPackage(lib2));
    EXPECT_TRUE(repo->addPackage(lib3));

    EXPECT_TRUE(repo->addDependency("main", "lib1"));
    EXPECT_TRUE(repo->addDependency("lib1", "lib2"));
    EXPECT_TRUE(repo->addDependency("lib2", "lib3"));

    auto allDeps = repo->getAllTransitiveDependencies("main");
    EXPECT_EQ(allDeps.size(), 3);
    EXPECT_NE(std::find(allDeps.begin(), allDeps.end(), "lib1"), allDeps.end());
    EXPECT_NE(std::find(allDeps.begin(), allDeps.end(), "lib2"), allDeps.end());
    EXPECT_NE(std::find(allDeps.begin(), allDeps.end(), "lib3"), allDeps.end());
}

TEST_F(RepositoryTest, GetAllTransitiveDependenciesNonExistent) {
    auto deps = repo->getAllTransitiveDependencies("non-existent");
    EXPECT_TRUE(deps.empty());
}

TEST_F(RepositoryTest, GetAllTransitiveDependenciesNoDeps) {
    auto* pkg = new MainPackage("pkg", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg));

    auto deps = repo->getAllTransitiveDependencies("pkg");
    EXPECT_TRUE(deps.empty());
}


TEST_F(RepositoryTest, GetPackageInfoNonExistent) {
    std::string info = repo->getPackageInfo("non-existent");
    EXPECT_NE(info.find("not found"), std::string::npos);
    EXPECT_NE(info.find("non-existent"), std::string::npos);
}


TEST_F(RepositoryTest, CountPackagesByType) {
    EXPECT_TRUE(repo->addPackage(new MainPackage("main1", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new MainPackage("main2", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new LibraryPackage("lib1", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new LibraryPackage("lib2", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new LibraryPackage("lib3", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new MetaPackage("meta1", "latest", "Pub")));

    auto count = repo->countPackagesByType();
    EXPECT_EQ(count.main, 2);
    EXPECT_EQ(count.library, 3);
    EXPECT_EQ(count.meta, 1);
}

TEST_F(RepositoryTest, GetPackagesByType) {
    EXPECT_TRUE(repo->addPackage(new MainPackage("main1", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new MainPackage("main2", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new LibraryPackage("lib1", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new MetaPackage("meta1", "latest", "Pub")));

    auto mainPkgs = repo->getPackagesByType(PackageType::MAIN);
    EXPECT_EQ(mainPkgs.size(), 2);
    EXPECT_NE(std::find(mainPkgs.begin(), mainPkgs.end(), "main1"), mainPkgs.end());
    EXPECT_NE(std::find(mainPkgs.begin(), mainPkgs.end(), "main2"), mainPkgs.end());

    auto libPkgs = repo->getPackagesByType(PackageType::LIBRARY);
    EXPECT_EQ(libPkgs.size(), 1);
    EXPECT_EQ(libPkgs[0], "lib1");

    auto metaPkgs = repo->getPackagesByType(PackageType::META);
    EXPECT_EQ(metaPkgs.size(), 1);
    EXPECT_EQ(metaPkgs[0], "meta1");
}


TEST_F(RepositoryTest, GetAllPackageNames) {
    EXPECT_TRUE(repo->addPackage(new MainPackage("pkg1", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new LibraryPackage("pkg2", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new MetaPackage("pkg3", "latest", "Pub")));

    auto allNames = repo->getAllPackageNames();
    EXPECT_EQ(allNames.size(), 3);
    EXPECT_NE(std::find(allNames.begin(), allNames.end(), "pkg1"), allNames.end());
    EXPECT_NE(std::find(allNames.begin(), allNames.end(), "pkg2"), allNames.end());
    EXPECT_NE(std::find(allNames.begin(), allNames.end(), "pkg3"), allNames.end());
}


TEST_F(RepositoryTest, GetLinkedPackageNonMeta) {
    auto* mainPkg = new MainPackage("main-pkg", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(mainPkg));

    EXPECT_EQ(repo->getLinkedPackage("main-pkg"), nullptr);
}

TEST_F(RepositoryTest, GetLinkedPackageMetaWithoutLink) {
    auto* meta = new MetaPackage("meta-pkg", "latest", "Pub");
    EXPECT_TRUE(repo->addPackage(meta));

    EXPECT_EQ(repo->getLinkedPackage("meta-pkg"), nullptr);
}

TEST_F(RepositoryTest, GetLinkedPackageNonExistent) {
    EXPECT_EQ(repo->getLinkedPackage("non-existent"), nullptr);
}


TEST_F(RepositoryTest, SplitLibrary) {
    auto* lib = new LibraryPackage("original-lib", "3.0", "OriginalPub");
    auto* app = new MainPackage("app", "1.0", "AppPub");

    EXPECT_TRUE(repo->addPackage(lib));
    EXPECT_TRUE(repo->addPackage(app));
    EXPECT_TRUE(repo->addDependency("app", "original-lib"));

    std::vector<std::string> newParts = { "lib-runtime", "lib-dev", "lib-docs" };
    std::vector<std::string> publishers = { "RuntimePub", "DevPub", "DocsPub" };

    EXPECT_TRUE(repo->splitLibrary("original-lib", newParts, publishers));

    EXPECT_FALSE(repo->hasPackage("original-lib"));
    EXPECT_TRUE(repo->hasPackage("lib-runtime"));
    EXPECT_TRUE(repo->hasPackage("lib-dev"));
    EXPECT_TRUE(repo->hasPackage("lib-docs"));

    auto deps = repo->getDependencies("app");
    EXPECT_EQ(deps.size(), 3);
    EXPECT_NE(std::find(deps.begin(), deps.end(), "lib-runtime"), deps.end());
    EXPECT_NE(std::find(deps.begin(), deps.end(), "lib-dev"), deps.end());
    EXPECT_NE(std::find(deps.begin(), deps.end(), "lib-docs"), deps.end());
}

TEST_F(RepositoryTest, SplitLibraryNonExistent) {
    std::vector<std::string> parts = { "part1", "part2" };
    std::vector<std::string> pubs = { "Pub1", "Pub2" };

    EXPECT_FALSE(repo->splitLibrary("non-existent", parts, pubs));
}

TEST_F(RepositoryTest, SplitLibraryNotLibrary) {
    auto* mainPkg = new MainPackage("main-pkg", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(mainPkg));

    std::vector<std::string> parts = { "part1", "part2" };
    std::vector<std::string> pubs = { "Pub1", "Pub2" };

    EXPECT_FALSE(repo->splitLibrary("main-pkg", parts, pubs));
}


TEST_F(RepositoryTest, SplitLibraryFewerPublishersThanParts) {
    auto* lib = new LibraryPackage("lib", "1.0", "DefaultPub");
    EXPECT_TRUE(repo->addPackage(lib));

    std::vector<std::string> parts = { "part1", "part2", "part3" };
    std::vector<std::string> pubs = { "Pub1" };

    EXPECT_TRUE(repo->splitLibrary("lib", parts, pubs));

    Package* part2 = repo->getPackage("part2");
    ASSERT_NE(part2, nullptr);
    EXPECT_EQ(part2->getPublisher(), "DefaultPub");
}



TEST_F(RepositoryTest, MergePackagesCreateLibrary) {
    auto* lib1 = new LibraryPackage("lib1", "1.5", "Pub");
    auto* lib2 = new LibraryPackage("lib2", "2.0", "Pub");

    EXPECT_TRUE(repo->addPackage(lib1));
    EXPECT_TRUE(repo->addPackage(lib2));

    std::vector<std::string> packages = { "lib1", "lib2" };
    EXPECT_TRUE(repo->mergePackages(packages, "merged-lib", PackageType::LIBRARY, "MergedPub"));

    Package* merged = repo->getPackage("merged-lib");
    ASSERT_NE(merged, nullptr);
    EXPECT_EQ(merged->getType(), PackageType::LIBRARY);
}

TEST_F(RepositoryTest, MergePackagesNonExistent) {
    std::vector<std::string> packages = { "non-existent1", "non-existent2" };
    EXPECT_FALSE(repo->mergePackages(packages, "merged", PackageType::MAIN, "Pub"));
}

TEST_F(RepositoryTest, MergePackagesDuplicateName) {
    auto* pkg1 = new MainPackage("pkg1", "1.0", "Pub");
    auto* existing = new MainPackage("existing", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
    EXPECT_TRUE(repo->addPackage(existing));

    std::vector<std::string> packages = { "pkg1" };
    EXPECT_FALSE(repo->mergePackages(packages, "existing", PackageType::MAIN, "Pub"));
}

TEST_F(RepositoryTest, MergePackagesContainsMeta) {
    auto* mainPkg = new MainPackage("main", "1.0", "Pub");
    auto* metaPkg = new MetaPackage("meta", "latest", "Pub");

    EXPECT_TRUE(repo->addPackage(mainPkg));
    EXPECT_TRUE(repo->addPackage(metaPkg));

    std::vector<std::string> packages = { "main", "meta" };
    EXPECT_FALSE(repo->mergePackages(packages, "merged", PackageType::MAIN, "Pub"));
}

TEST_F(RepositoryTest, MergePackagesCreateMeta) {
    auto* pkg1 = new MainPackage("pkg1", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg1));

    std::vector<std::string> packages = { "pkg1" };
    EXPECT_FALSE(repo->mergePackages(packages, "meta-merged", PackageType::META, "Pub"));
}


TEST_F(RepositoryTest, MergeSinglePackage) {
    auto* pkg = new MainPackage("single", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg));

    std::vector<std::string> packages = { "single" };
    EXPECT_FALSE(repo->mergePackages(packages, "merged-single", PackageType::MAIN, "NewPub"));

    EXPECT_FALSE(repo->hasPackage("merged-single"));
    EXPECT_TRUE(repo->hasPackage("single"));
}


TEST_F(RepositoryTest, ClearRepository) {
    EXPECT_TRUE(repo->addPackage(new MainPackage("pkg1", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new LibraryPackage("pkg2", "1.0", "Pub")));
    EXPECT_TRUE(repo->addPackage(new MetaPackage("pkg3", "latest", "Pub")));

    EXPECT_EQ(repo->getAllPackageNames().size(), 3);

    repo->clear();

    EXPECT_EQ(repo->getAllPackageNames().size(), 0);
    EXPECT_FALSE(repo->hasPackage("pkg1"));
    EXPECT_FALSE(repo->hasPackage("pkg2"));
    EXPECT_FALSE(repo->hasPackage("pkg3"));
}


TEST_F(RepositoryTest, ConstMethods) {
    auto* pkg = new MainPackage("const-pkg", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg));

    const Repository* constRepo = repo;

    EXPECT_TRUE(constRepo->hasPackage("const-pkg"));
    EXPECT_FALSE(constRepo->hasPackage("non-existent"));

    const Package* constPkg = constRepo->getPackage("const-pkg");
    ASSERT_NE(constPkg, nullptr);
    EXPECT_EQ(constPkg->getName(), "const-pkg");

    auto deps = constRepo->getDependencies("const-pkg");
    EXPECT_TRUE(deps.empty());

    auto allNames = constRepo->getAllPackageNames();
    EXPECT_EQ(allNames.size(), 1);
    EXPECT_EQ(allNames[0], "const-pkg");
}


TEST_F(RepositoryTest, SelfDependencyPrevention) {
    auto* pkg = new MainPackage("self-dep", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(pkg));

    EXPECT_FALSE(repo->addDependency("self-dep", "self-dep"));
}

TEST_F(RepositoryTest, LargeNumberOfDependencies) {
    auto* mainPkg = new MainPackage("main", "1.0", "Pub");
    EXPECT_TRUE(repo->addPackage(mainPkg));

    for (int i = 0; i < 10; i++) {
        std::string libName = "lib" + std::to_string(i);
        auto* lib = new LibraryPackage(libName, "1.0", "Pub");
        EXPECT_TRUE(repo->addPackage(lib));
        EXPECT_TRUE(repo->addDependency("main", libName));
    }

    auto deps = repo->getDependencies("main");
    EXPECT_EQ(deps.size(), 10);

    auto allTransitive = repo->getAllTransitiveDependencies("main");
    EXPECT_EQ(allTransitive.size(), 10);
}

TEST_F(RepositoryTest, PackageNamesCaseSensitive) {
    auto* pkg1 = new MainPackage("Pkg", "1.0", "Pub");
    auto* pkg2 = new MainPackage("pkg", "1.0", "Pub");
    auto* pkg3 = new MainPackage("PKG", "1.0", "Pub");

    EXPECT_TRUE(repo->addPackage(pkg1));
}
