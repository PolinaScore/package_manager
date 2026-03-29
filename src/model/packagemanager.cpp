#include "packagemanager.h"
#include "mainpackage.h"
#include "librarypackage.h"
#include "metapackage.h"
#include <algorithm>
#include <sstream>
#include <mutex>
#include <thread>


const Package* PackageManager::getTargetPackage(const std::string& packageName) const {
    const Package* pkg = repository.getPackage(packageName);
    if (!pkg) return nullptr;


    if (pkg->getType() == PackageType::META) {
        return repository.getLinkedPackage(packageName);
    }

    return pkg;
}

Package* PackageManager::getTargetPackage(const std::string& packageName) {
    Package* pkg = repository.getPackage(packageName);
    if (!pkg) return nullptr;

    if (pkg->getType() == PackageType::META) {
        return repository.getLinkedPackage(packageName);
    }

    return pkg;
}

bool PackageManager::installDependencies(const std::string& packageName,
    std::set<std::string>& installing,
    std::vector<std::string>& installOrder,
    std::vector<std::string>& errors) {
    if (installing.count(packageName)) {
        errors.push_back("Circular dependency detected at: " + packageName);
        return false;
    }

    installing.insert(packageName);

    const Package* targetPkg = getTargetPackage(packageName);
    if (!targetPkg) {
        errors.push_back("Package not found: " + packageName);
        installing.erase(packageName);
        return false;
    }


    for (const auto& dep : targetPkg->getDependencies()) {
        const Package* depPkg = repository.getPackage(dep);
        if (!depPkg) {
            errors.push_back("Dependency not found: " + dep);
            installing.erase(packageName);
            return false;
        }


        if (!depPkg->isInstalled()) {
            if (!installDependencies(dep, installing, installOrder, errors)) {
                installing.erase(packageName);
                return false;
            }
        }
    }


    if (!targetPkg->isInstalled()) {
        Package* mutablePkg = getTargetPackage(packageName);
        if (mutablePkg) {
            mutablePkg->setInstalledVersion(mutablePkg->getLatestVersion());
            installOrder.push_back(targetPkg->getName());
        }
    }

    installing.erase(packageName);
    return true;
}

bool PackageManager::initialize(const std::vector<std::string>& packageNames) {
    bool success = true;
    for (const auto& name : packageNames) {
        auto result = installPackage(name);
        if (!result.success) {
            success = false;
        }
    }
    return success;
}

PackageManager::InstallResult PackageManager::installPackage(const std::string& packageName) {
    InstallResult result;

    const Package* pkg = repository.getPackage(packageName);
    if (!pkg) {
        result.errors.push_back("Package not found: " + packageName);
        result.success = false;
        return result;
    }


    if (pkg->getType() == PackageType::LIBRARY) {
        result.errors.push_back("Cannot install LIBRARY package directly: " + packageName);
        result.success = false;
        return result;
    }


    const Package* targetPkg = getTargetPackage(packageName);
    if (targetPkg && targetPkg->isInstalled()) {
        result.installedPackages.push_back(targetPkg->getName() + " (already installed)");
        result.success = true;
        return result;
    }


    std::set<std::string> installing;
    std::vector<std::string> installOrder;
    std::vector<std::string> errors;

    if (!installDependencies(packageName, installing, installOrder, errors)) {
        result.errors = errors;
        result.success = false;
        return result;
    }

    result.installedPackages = installOrder;
    result.success = true;
    return result;
}



PackageManager::RemoveResult PackageManager::removePackage(const std::string& packageName, bool removeUnusedLibs) {
    RemoveResult result;

    Package* pkg = repository.getPackage(packageName);
    if (!pkg) {
        result.errors.push_back("Package not found: " + packageName);
        return result;
    }

    if (pkg->getType() != PackageType::MAIN) {
        result.errors.push_back("Can only remove MAIN packages: " + packageName);
        return result;
    }
    if (!pkg->isInstalled()) {
        result.removedPackages.push_back(packageName);
        result.success = true;
        return result;
    }

    std::vector<std::string> allDeps = repository.getAllTransitiveDependencies(packageName);
    pkg->setInstalledVersion("");
    result.removedPackages.push_back(packageName);

    if (removeUnusedLibs) {

        std::set<std::string> librariesToCheck;

        for (const auto& depName : allDeps) {
            Package* dep = repository.getPackage(depName);
            if (dep && dep->getType() == PackageType::LIBRARY) {
                librariesToCheck.insert(depName);
            }
        }

        const auto& allPackages = repository.getAllPackages();

        for (auto it = allPackages.begin(); it != allPackages.end(); ++it) {
            const std::string& currentPkgName = (*it).first;
            Package* currentPkg = (*it).second;

            if (currentPkg && currentPkg->getType() == PackageType::LIBRARY && currentPkg->isInstalled()) {

                bool shouldCheck = (librariesToCheck.find(currentPkgName) != librariesToCheck.end());

                if (shouldCheck) {

                    std::vector<std::string> reverseDeps = repository.getReverseDependencies(currentPkgName);
                    bool isUsed = false;

                    for (const auto& dependerName : reverseDeps) {
                        Package* depender = repository.getPackage(dependerName);
                        if (depender && depender->isInstalled() && dependerName != packageName) {
                            isUsed = true;
                            break;
                        }
                    }

                    if (!isUsed) {
                        currentPkg->setInstalledVersion("");
                        result.removedPackages.push_back(currentPkgName);
                    }
                }
            }
        }
    }

    result.success = true;
    return result;
}

bool PackageManager::isUsedLibrary(const std::string& libName) const {
    auto reverseDeps = repository.getReverseDependencies(libName);

    for (const auto& dependerName : reverseDeps) {
        const Package* depender = repository.getPackage(dependerName);
        if (depender && depender->isInstalled()) {
            return true;
        }
    }
    return false;
}

std::vector<std::vector<std::string>> PackageManager::findUnusedLibraries(size_t threadCount) const {
    auto allLibs = repository.getPackagesByType(PackageType::LIBRARY);
    if (allLibs.empty())
        return std::vector<std::vector<std::string>>{{}};

    threadCount = std::min(threadCount, allLibs.size());

    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> threadResults(threadCount);
    size_t libsPerThread = (allLibs.size() + threadCount - 1) / threadCount;


    for (size_t i = 0; i < threadCount; ++i) {
        size_t startIdx = i * libsPerThread;
        size_t endIdx = std::min(startIdx + libsPerThread, allLibs.size());

        if (startIdx < allLibs.size()) {
            threads.emplace_back([this, &allLibs, startIdx, endIdx, i, &threadResults]() {
                std::vector<std::string> localRemoved;
                std::copy_if(allLibs.begin() + startIdx,
                    allLibs.begin() + endIdx,
                    std::back_inserter(localRemoved),
                    [this](const std::string& libName) {
                        const Package* lib = repository.getPackage(libName);
                        return (lib && lib->isInstalled() && !isUsedLibrary(libName));
                    });
                threadResults[i] = std::move(localRemoved);
                });
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return threadResults;
}

PackageManager::CleanupResult PackageManager::cleanupUnusedLibraries(size_t threadCount) {
    CleanupResult result;

    std::vector<std::vector<std::string>> unusedLibs = findUnusedLibraries(threadCount);

    for (const auto& names : unusedLibs) {
        for (const auto& libName : names) {
            Package* lib = repository.getPackage(libName);
            if (lib) {
                lib->setInstalledVersion("");
                result.removedPackages.push_back(libName);
                result.removedCount++;
            }
        }
    }

    return result;
}


PackageManager::UpdateResult PackageManager::updateAll() {
    UpdateResult result;

    auto installed = getInstalledPackageNames();

    for (const auto& name : installed) {
        Package* pkg = repository.getPackage(name);
        if (pkg && pkg->isInstalled()) {
            std::string oldVersion = pkg->getInstalledVersion();
            std::string newVersion = pkg->getLatestVersion();

            if (oldVersion != newVersion) {
                pkg->setInstalledVersion(newVersion);
                result.updatedPackages.push_back(name + ": " + oldVersion + " -> " + newVersion);
                result.updatedCount++;
            }
        }
    }

    return result;
}


bool PackageManager::addPackageToRepository(Package* package) {
    if (!package) return false;

    if (repository.hasCycles()) {
        delete package;
        return false;
    }

    if (!repository.addPackage(package)) {
        delete package;
        return false;
    }

    if (repository.hasCycles()) {
        repository.removePackage(package->getName());
        return false;
    }

    return true;
}

bool PackageManager::addPackageToRepository(const std::string& name, PackageType type,
    const std::string& version, const std::string& publisher) {
    Package* package = createPackage(type, name, version, publisher);
    if (!package) return false;

    return addPackageToRepository(package);
}

std::string PackageManager::getPackageInfo(const std::string& packageName) const {
    return repository.getPackageInfo(packageName);
}

bool PackageManager::deletePackageFromRepository(const std::string& packageName) {
    return repository.removePackage(packageName);
}

bool PackageManager::updatePackageInRepository(const std::string& packageName,
    const std::string& newVersion) {
    return repository.updatePackage(packageName, newVersion);
}


PackageManager::SplitResult PackageManager::splitLibrary(
    const std::string& libraryName,
    const std::vector<std::string>& newPartNames,
    const std::vector<std::string>& newPublishers) {

    SplitResult result;

    if (!repository.splitLibrary(libraryName, newPartNames, newPublishers)) {
        result.error = "Failed to split library: " + libraryName;
        result.success = false;
        return result;
    }

    result.newPackages = newPartNames;
    result.success = true;
    return result;
}

PackageManager::MergeResult PackageManager::mergePackages(
    const std::vector<std::string>& packageNames,
    const std::string& newPackageName,
    PackageType newType,
    const std::string& newPublisher) {

    MergeResult result;

    if (!repository.mergePackages(packageNames, newPackageName, newType, newPublisher)) {
        result.error = "Failed to merge packages";
        result.success = false;
        return result;
    }

    result.newPackageName = newPackageName;
    result.success = true;
    return result;
}



std::vector<std::string> PackageManager::getAllPackageNames() const {
    return repository.getAllPackageNames();
}

std::vector<std::string> PackageManager::getInstalledPackageNames() const {
    return repository.getInstalledPackages();
}

std::vector<std::string> PackageManager::getPackagesByType(PackageType type) const {
    return repository.getPackagesByType(type);
}

const Package* PackageManager::findPackage(const std::string& name) const {
    return repository.getPackage(name);
}

Package* PackageManager::findPackage(const std::string& name) {
    return repository.getPackage(name);
}

bool PackageManager::hasPackage(const std::string& name) const {
    return repository.hasPackage(name);
}

PackageManager::Statistics PackageManager::getStatistics() const {
    Statistics stats;

    auto allNames = getAllPackageNames();
    stats.totalPackages = allNames.size();

    auto installed = getInstalledPackageNames();
    stats.installedPackages = installed.size();

    auto typeCount = repository.countPackagesByType();
    stats.mainPackages = typeCount.main;
    stats.libraryPackages = typeCount.library;
    stats.metaPackages = typeCount.meta;

    return stats;
}

bool PackageManager::checkSystemIntegrity(std::vector<std::string>& errors) const {
    errors.clear();

    if (repository.hasCycles()) {
        errors.push_back("Dependency graph has cycles");
    }

    for (const auto& entry : repository.getAllPackages()) {
        const Package* pkg = entry.second;
        for (const auto& dep : pkg->getDependencies()) {
            if (!repository.hasPackage(dep)) {
                errors.push_back("Package " + pkg->getName() + " depends on non-existent: " + dep);
            }
        }
    }

    return errors.empty();
}


