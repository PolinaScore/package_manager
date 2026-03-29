#include "package.h"
#include <algorithm>
#include <sstream>

Package::Package(const std::string& name, 
                 const std::string& latestVersion, 
                 const std::string& publisher)
    : name(name), latestVersion(latestVersion), 
      installedVersion(""), publisher(publisher) {
}

void Package::addDependency(const std::string& packageName) {
    if (std::find(dependencies.begin(), dependencies.end(), packageName) == dependencies.end()) {
        dependencies.push_back(packageName);
    }
}

void Package::removeDependency(const std::string& packageName) {
    dependencies.erase(
        std::remove(dependencies.begin(), dependencies.end(), packageName),
        dependencies.end()
    );
}

bool Package::dependsOn(const std::string& packageName) const {
    return std::find(dependencies.begin(), dependencies.end(), packageName) != dependencies.end();
}

std::string Package::toString() const {
    std::stringstream ss;
    ss << "Package: " << name << "\n"
       << "  Type: " << getTypeName() << "\n"
       << "  Latest version: " << latestVersion << "\n"
       << "  Installed: " << (isInstalled() ? "Yes (" + installedVersion + ")" : "No") << "\n"
       << "  Publisher: " << publisher << "\n"
       << "  Dependencies (" << dependencies.size() << "): ";
    
    for (size_t i = 0; i < dependencies.size(); ++i) {
        ss << dependencies[i];
        if (i != dependencies.size() - 1) ss << ", ";
    }
    
    return ss.str();
}
