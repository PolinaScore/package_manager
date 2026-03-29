#include "model/packagemanager.h"
#include "model/metapackage.h"
#include <iostream>
#include <sstream>
#include <cassert>

void runPackageManagerTests() {
    std::cout << "=== Starting Package Manager Tests ===\n\n";
    
    PackageManager pm;
    int testsPassed = 0;
    int totalTests = 0;
    
    {
        totalTests++;
        std::cout << "Test 1: Adding packages of different types... ";
        
        bool mainAdded = pm.addPackageToRepository("app1", PackageType::MAIN, "1.0", "AppCorp");
        bool libAdded = pm.addPackageToRepository("graphics", PackageType::LIBRARY, "2.1", "BaseLibs");
        bool metaAdded = pm.addPackageToRepository("meta1", PackageType::META, "latest", "MetaCorp");
        
        if (mainAdded && libAdded && metaAdded) {
            std::cout << "PASSED\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 2: Installing MAIN package... ";
        
        auto result = pm.installPackage("app1");
        
        if (result.success && !result.installedPackages.empty()) {
            std::cout << "PASSED (installed: " << result.installedPackages[0] << ")\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 3: Cannot install LIBRARY directly... ";
        
        auto result = pm.installPackage("graphics");
        
        if (!result.success) {
            std::cout << "PASSED (correctly rejected)\n";
            testsPassed++;
        } else {
            std::cout << "FAILED (should reject library installation)\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 4: Adding dependencies... ";
        
        pm.addPackageToRepository("app2", PackageType::MAIN, "1.5", "AppCorp");
        pm.addPackageToRepository("network", PackageType::LIBRARY, "3.0", "BaseLibs");
        
        bool dep1 = pm.getRepository().addDependency("app2", "network");
        bool dep2 = pm.getRepository().addDependency("app2", "graphics");
        
        if (dep1 && dep2) {
            std::cout << "PASSED\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }

    {
        totalTests++;
        std::cout << "Test 5: Installing with dependencies... ";
        
        auto result = pm.installPackage("app2");
        
        if (result.success && result.installedPackages.size() >= 3) {
            std::cout << "PASSED (installed " << result.installedPackages.size() << " packages)\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 6: META package linking... ";
        
        auto* metaPkg = dynamic_cast<MetaPackage*>(pm.findPackage("meta1"));
        if (metaPkg) {
            metaPkg->setLinkedPackageName("app1");
            
            auto result = pm.installPackage("meta1");
            
            if (result.success) {
                std::cout << "PASSED\n";
                testsPassed++;
            } else {
                std::cout << "FAILED (META installation)\n";
            }
        } else {
            std::cout << "FAILED (META package not found)\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 7: Splitting library... ";
        
        pm.addPackageToRepository("utils", PackageType::LIBRARY, "1.0", "BaseLibs");
        
        auto result = pm.splitLibrary("utils", {"utils-core", "utils-ext"});
        
        if (result.success && result.newPackages.size() == 2) {
            std::cout << "PASSED (created: ";
            for (const auto& name : result.newPackages) {
                std::cout << name << " ";
            }
            std::cout << ")\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 8: Merging packages... ";
        
        auto result = pm.mergePackages({"app1", "app2"}, "superapp", PackageType::MAIN);
        
        if (result.success) {
            std::cout << "PASSED (created: " << result.newPackageName << ")\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }
    
    {
        totalTests++;
        std::cout << "Test 9: System statistics... ";
        
        auto stats = pm.getStatistics();
        
        if (stats.totalPackages > 0) {
            std::cout << "PASSED (total: " << stats.totalPackages 
                     << ", installed: " << stats.installedPackages 
                     << ", main: " << stats.mainPackages << ")\n";
            testsPassed++;
        } else {
            std::cout << "FAILED\n";
        }
    }

    {
        totalTests++;
        std::cout << "Test 10: System integrity check... ";
        
        std::vector<std::string> errors;
        bool integrity = pm.checkSystemIntegrity(errors);
        
        if (integrity && errors.empty()) {
            std::cout << "PASSED (system is healthy)\n";
            testsPassed++;
        } else {
            std::cout << "FAILED (errors: " << errors.size() << ")\n";
        }
    }
    
    std::cout << "\n=== Test Results ===\n";
    std::cout << "Tests passed: " << testsPassed << "/" << totalTests << "\n";
    std::cout << "Success rate: " << (testsPassed * 100 / totalTests) << "%\n";
    
    if (testsPassed == totalTests) {
        std::cout << "\n✅ ALL TESTS PASSED! PackageManager works correctly.\n";
    } else {
        std::cout << "\n❌ Some tests failed. Needs debugging.\n";
    }
}

void runConsoleUITests() {
    std::cout << "\n=== Console UI Simulation Tests ===\n\n";
    
    PackageManager pm;
    std::stringstream input;
    std::stringstream output;
    
    std::cout << "Simulating: add myapp MAIN 1.0 MyCorp\n";
    bool added = pm.addPackageToRepository("myapp", PackageType::MAIN, "1.0", "MyCorp");
    std::cout << "Result: " << (added ? "SUCCESS" : "FAILED") << "\n";
    
    std::cout << "\nSimulating: install myapp\n";
    auto installResult = pm.installPackage("myapp");
    std::cout << "Result: " << (installResult.success ? "SUCCESS" : "FAILED") << "\n";
    
    std::cout << "\nSimulating: list\n";
    auto allPackages = pm.getAllPackageNames();
    std::cout << "Packages: ";
    for (const auto& pkg : allPackages) {
        std::cout << pkg << " ";
    }
    std::cout << "\n";
    
    std::cout << "\n✅ Console UI simulation completed.\n";
}



int main() {
    std::cout << "Package Manager Test Suite\n";
    std::cout << "==========================\n\n";
    
    try {
        runPackageManagerTests();
    } catch (const std::exception& e) {
        std::cerr << "\n❌ TEST SUITE CRASHED: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
