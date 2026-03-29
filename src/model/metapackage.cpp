#include "metapackage.h"
#include <sstream>

MetaPackage::MetaPackage(const std::string& name,
                         const std::string& latestVersion,
                         const std::string& publisher)
    : Package(name, latestVersion, publisher), linkedPackageName("") {
    
}

PackageType MetaPackage::getType() const {
    return PackageType::META;
}

std::string MetaPackage::getTypeName() const {
    return "META";
}

bool MetaPackage::canBeInstalled() const {
    return true;  
}

Package* MetaPackage::clone() const {
    MetaPackage* clone = new MetaPackage(*this);
    clone->linkedPackageName = linkedPackageName;
    return clone;
}

std::string MetaPackage::toString() const {
   
    std::string result = Package::toString();
    
    if (!linkedPackageName.empty()) {
        
        result += "\n  Linked to: " + linkedPackageName;
    } else {
        result += "\n  Linked to: (not set)";
    }
    
    return result;
}
