#include "repository.h"
#include "mainpackage.h"
#include "librarypackage.h"
#include "metapackage.h"
#include <algorithm>
#include <sstream>
#include <memory>


Repository::Repository() = default;

Repository::~Repository() {
    deleteAllPackages();
}

void Repository::deleteAllPackages() {
    for(auto it = packages.begin(); it!=packages.end(); ++it) {
        delete it->second;
    }
    packages.clear();
    reverseDeps.clear();
}


void Repository::addReverseDependency(const std::string& packageName, const std::string& dependsOn) {
    auto it = reverseDeps.find(dependsOn);
    if (it != reverseDeps.end()) {
        std::vector<std::string>& deps = (*it).second;
        auto depsIt = std::find(deps.begin(), deps.end(), packageName);

        if (depsIt == deps.end()) {
            deps.push_back(packageName);
        }
    }
    else {
        reverseDeps.insert({dependsOn, std::vector<std::string>{packageName}});
    }
}

void Repository::removeReverseDependency(const std::string& packageName, const std::string& dependsOn) {
    auto entryIt = reverseDeps.find(dependsOn);
    if (entryIt == reverseDeps.end()) {
        return;
    }
    auto& dependencyList = (*entryIt).second;
    dependencyList.erase( std::remove(dependencyList.begin(), dependencyList.end(), packageName), dependencyList.end());
    if (dependencyList.empty()) {
        reverseDeps.erase(dependsOn);
    }
}

bool Repository::hasCycleDFS(const std::string& packageName,
                            std::set<std::string>& visited,
                            std::set<std::string>& recursionStack) const {
    if (recursionStack.count(packageName)) {
        return true;
    }
    
    if (visited.count(packageName)) {
        return false;
    }
    
    visited.insert(packageName);
    recursionStack.insert(packageName);
    
    const Package* pkg = getPackage(packageName);
    if (pkg) {
        for (const auto& dep : pkg->getDependencies()) {
            if (hasCycleDFS(dep, visited, recursionStack)) {
                return true;
            }
        }
    }
    
    recursionStack.erase(packageName);
    return false;
}

void Repository::getAllDependencies(const std::string& packageName,
                                   std::set<std::string>& result,
                                   std::set<std::string>& visited) const {
    if (visited.count(packageName)) {
        return;
    }
    
    visited.insert(packageName);
    
    const Package* pkg = getPackage(packageName);
    if (!pkg) return;
    
    for (const auto& dep : pkg->getDependencies()) {
        result.insert(dep);
        getAllDependencies(dep, result, visited);
    }
}



bool Repository::addPackage(Package* package) {
    if (!package) return false;
    
    const std::string& name = package->getName();
    
    
    if (hasPackage(name)) {
        delete package;  
        return false;
    }
    

    if (auto* redirectable = dynamic_cast<IRedirectable*>(package)) {
        const std::string& linkedName = redirectable->getLinkedPackageName();
        if (!linkedName.empty() && !hasPackage(linkedName)) {
            delete package;
            return false;
        }
    }

    auto insertResult = packages.insert(name, package);

    return insertResult.second;
}

Package* Repository::getPackage(const std::string& packageName) {
    auto pkgPtr = packages.find(packageName);
    return pkgPtr != packages.end() ? (*pkgPtr).second : nullptr;
}


const Package* Repository::getPackage(const std::string& packageName) const {
    auto pkgPtr = packages.find(packageName);
    return pkgPtr != packages.end() ? (*pkgPtr).second : nullptr;
}


bool Repository::hasPackage(const std::string& packageName) const {
    return packages.contains(packageName);
}

bool Repository::updatePackage(const std::string& packageName, const std::string& newVersion) {
    Package* pkg = getPackage(packageName);
    if (!pkg) {
        return false;
    }
    
    pkg->setLatestVersion(newVersion);
    return true;
}

bool Repository::removePackage(const std::string& packageName) {

    if (!hasPackage(packageName)) {
        return false;
    }
    auto reverseIt = reverseDeps.find(packageName);
    if (reverseIt != reverseDeps.end()) {
        const auto& depsList = (*reverseIt).second;
        if (!depsList.empty()) {
            return false; 
        }
    }
    Package* pkg = getPackage(packageName);
    if (pkg) {
        for (const auto& dep : pkg->getDependencies()) {
            removeReverseDependency(packageName, dep);
        }
    }
    auto packageIt = packages.find(packageName);
    if (packageIt != packages.end()) {
        Package* toDelete = (*packageIt).second;  
        packages.erase(packageName);
        delete toDelete;
    }
    reverseDeps.erase(packageName);

    return true;
}



bool Repository::addDependency(const std::string& fromPackage, const std::string& toPackage) {


    Package* fromPkg = getPackage(fromPackage);
    Package* toPkg = getPackage(toPackage);
    
    if (!fromPkg || !toPkg) {
        return false;
    }
    
    if (fromPkg->dependsOn(toPackage)) {
        return true;
    }
    
    fromPkg->addDependency(toPackage);
    addReverseDependency(fromPackage, toPackage);
    
    if (hasCycleFromPackage(fromPackage)) {
        fromPkg->removeDependency(toPackage);
        removeReverseDependency(fromPackage, toPackage);
        return false;
    }
    
    return true;
}

bool Repository::removeDependency(const std::string& fromPackage, const std::string& toPackage) {
    Package* fromPkg = getPackage(fromPackage);
    if (!fromPkg) {
        return false;
    }
    
    if (fromPkg->dependsOn(toPackage)) {
        fromPkg->removeDependency(toPackage);
        removeReverseDependency(fromPackage, toPackage);
        return true;
    }
    
    return false;
}


std::vector<std::string> Repository::getDependencies(const std::string& packageName) const {
    const Package* pkg = getPackage(packageName);
    if (pkg) {
        return pkg->getDependencies();
    }
    return {};
}


std::vector<std::string> Repository::getReverseDependencies(const std::string& packageName) const {
    auto deps = reverseDeps.find(packageName);
    if (deps != reverseDeps.end()) {
        return (*deps).second;
    }
    return {};
}


std::vector<std::string> Repository::getAllTransitiveDependencies(const std::string& packageName) const {
    std::set<std::string> result;
    std::set<std::string> visited;
    
    getAllDependencies(packageName, result, visited);
    
    return std::vector<std::string>(result.begin(), result.end());
}


bool Repository::hasCycles() const {
    std::set<std::string> visited;
    
    for (const auto& entry : packages) {
        const std::string& packageName = entry.first;
        
        if (!visited.count(packageName)) {
            std::set<std::string> recursionStack;
            if (hasCycleDFS(packageName, visited, recursionStack)) {
                return true;
            }
        }
    }
    
    return false;
}

bool Repository::hasCycleFromPackage(const std::string& packageName) const {
    std::set<std::string> visited;
    std::set<std::string> recursionStack;
    
    return hasCycleDFS(packageName, visited, recursionStack);
}



std::string Repository::getPackageInfo(const std::string& packageName) const {
    const Package* pkg = getPackage(packageName);
    if (!pkg) {
        return "Package not found: " + packageName;
    }
    
    std::stringstream ss;
    ss << "=== Package Info ===\n"
       << "Name: " << pkg->getName() << "\n"
       << "Type: " << pkg->getTypeName() << "\n"
       << "Installed: " << (pkg->isInstalled() ? "Yes" : "No") << "\n"
       << "Current version: " << pkg->getInstalledVersion() << "\n"
       << "Latest version: " << pkg->getLatestVersion() << "\n"
       << "Publisher: " << pkg->getPublisher() << "\n"
       << "Dependencies: " << pkg->getDependencies().size() << "\n";
    
    TypeCount typeCount = countPackagesByType();
    ss << "Main packages in repo: " << typeCount.main << "\n"
       << "Library packages in repo: " << typeCount.library << "\n"
       << "Meta packages in repo: " << typeCount.meta << "\n";
    
    if (auto* redirectable = dynamic_cast<const IRedirectable*>(pkg)) {
        const std::string& linkedName = redirectable->getLinkedPackageName();
        if (!linkedName.empty()) {
            ss << "Linked to: " << linkedName << "\n";
            const Package* linked = getPackage(linkedName);
            if (linked) {
                ss << "Linked package latest version: " << linked->getLatestVersion() << "\n";
            }
        }
    }
    
    return ss.str();
}

Repository::TypeCount Repository::countPackagesByType() const {
    TypeCount count;
    
    for (const auto& entry : packages) {
        const Package* pkg = entry.second;
        switch (pkg->getType()) {
            case PackageType::MAIN: count.main++; break;
            case PackageType::LIBRARY: count.library++; break;
            case PackageType::META: count.meta++; break;
        }
    }
    
    return count;
}

std::vector<std::string> Repository::getPackagesByType(PackageType type) const {
    std::vector<std::string> result;
    
    for (const auto& entry : packages) {
        const Package* pkg = entry.second;
        if (pkg->getType() == type) {
            result.push_back(entry.first);
        }
    }
    
    return result;
}

std::vector<std::string> Repository::getInstalledPackages() const {
    std::vector<std::string> result;
    
    for (const auto& entry : packages) {
        const Package* pkg = entry.second;
        if (pkg->isInstalled()) {
            result.push_back(entry.first);
        }
    }
    
    return result;
}

std::vector<std::string> Repository::getAllPackageNames() const {
    std::vector<std::string> result;
    
    for (const auto& entry : packages) {
        result.push_back(entry.first);
    }
    
    return result;
}

Package* Repository::getLinkedPackage(const std::string& metaPackageName) {
    Package* metaPkg = getPackage(metaPackageName);
    if (!metaPkg || metaPkg->getType() != PackageType::META) {
        return nullptr;
    }
    
    if (auto* redirectable = dynamic_cast<IRedirectable*>(metaPkg)) {
        return getPackage(redirectable->getLinkedPackageName());
    }
    return nullptr;
}

const Package* Repository::getLinkedPackage(const std::string& metaPackageName) const {
    const Package* metaPkg = getPackage(metaPackageName);
    if (!metaPkg) {
        return nullptr;
    }
    
    if (auto* redirectable = dynamic_cast<const IRedirectable*>(metaPkg)) {
        return getPackage(redirectable->getLinkedPackageName());
    }
    return nullptr;
}



bool Repository::splitLibrary(const std::string& libraryName,
                             const std::vector<std::string>& newPartNames,
                             const std::vector<std::string>& newPublishers) {
    LibraryPackage* lib = dynamic_cast<LibraryPackage*>(getPackage(libraryName));
    if (!lib || lib->getType() != PackageType::LIBRARY) {
        return false;
    }
    
    
    auto reverseDepsList = getReverseDependencies(libraryName);
    
    
    std::vector<LibraryPackage*> newLibs;
    for (size_t i = 0; i < newPartNames.size(); ++i) {
        const std::string& name = newPartNames[i];
        
        if (hasPackage(name)) {
            for (auto* l : newLibs) delete l;
            return false;
        }
        
        std::string publisher = (i < newPublishers.size()) ? newPublishers[i] : lib->getPublisher();
        LibraryPackage* newLib = new LibraryPackage(name, lib->getLatestVersion(), publisher);
        

        for (const auto& dep : lib->getDependencies()) {
            newLib->addDependency(dep);
        }
        
        
        if (!addPackage(newLib)) {
            delete newLib;
           
            for (auto* l : newLibs) delete l;
            return false;
        }
        
        newLibs.push_back(newLib);
        
        
        for (const auto& depender : reverseDepsList) {
            removeDependency(depender, libraryName);
            addDependency(depender, name);
        }
    }
    
    
    return removePackage(libraryName);
}

bool Repository::mergePackages(const std::vector<std::string>& packageNames, const std::string& newPackageName, PackageType newType, const std::string& newPublisher) {
    if (newType == PackageType::META) {
        return false;
    }
    
    if (packageNames.size() < 2) {
        return false;
    }

    if (hasPackage(newPackageName)) {
        return false;
    }
    
    std::set<std::string> allDeps;
    std::string latestVersion = "1.0";
    
    for (const auto& name : packageNames) {
        const Package* pkg = getPackage(name);
        if (!pkg) {
            return false;
        }
        
        if (pkg->getType() == PackageType::META) {
            return false;
        }
        
        for (const auto& dep : pkg->getDependencies()) {
            allDeps.insert(dep);
        }
        
        if (pkg->getLatestVersion() > latestVersion) {
            latestVersion = pkg->getLatestVersion();
        }
    }
    
    
    Package* mergedPackage = nullptr;
    switch (newType) {
        case PackageType::MAIN:
            mergedPackage = new MainPackage(newPackageName, latestVersion, newPublisher);
            break;
        case PackageType::LIBRARY:
            mergedPackage = new LibraryPackage(newPackageName, latestVersion, newPublisher);
            break;
        default:
            return false;
    }
    
    for (const auto& dep : allDeps) {
        mergedPackage->addDependency(dep);
    }
    
    return addPackage(mergedPackage);
}

void Repository::clear() {
    deleteAllPackages();
}
