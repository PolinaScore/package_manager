#include "librarypackage.h"
#include <stdexcept>

LibraryPackage::LibraryPackage(const std::string& name,
                               const std::string& latestVersion,
                               const std::string& publisher)
    : Package(name, latestVersion, publisher) {
}

PackageType LibraryPackage::getType() const {
    return PackageType::LIBRARY;
}

std::string LibraryPackage::getTypeName() const {
    return "LIBRARY";
}

bool LibraryPackage::canBeInstalled() const {
    return false;  
}

Package* LibraryPackage::clone() const {
    return new LibraryPackage(*this);
}

std::vector<LibraryPackage*> LibraryPackage::split(
    const std::vector<std::string>& newPartNames,
    const std::vector<std::string>& newPublishers) const {
    
    std::vector<LibraryPackage*> result;
    
    for (size_t i = 0; i < newPartNames.size(); ++i) {
        const std::string& name = newPartNames[i];
        std::string publisher = (i < newPublishers.size()) ? 
                                newPublishers[i] : getPublisher();
        
        LibraryPackage* newLib = new LibraryPackage(name, getLatestVersion(), publisher);
        
        
        for (const auto& dep : getDependencies()) {
            newLib->addDependency(dep);
        }
        
        result.push_back(newLib);
    }
    
    return result;
}

bool LibraryPackage::isUsed() const {
    return false;
}
