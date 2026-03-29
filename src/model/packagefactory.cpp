#include "packagefactory.h"
#include "mainpackage.h"
#include "librarypackage.h"
#include "metapackage.h"

Package* createPackage(PackageType type,
                       const std::string& name,
                       const std::string& version,
                       const std::string& publisher) {
    switch (type) {
        case PackageType::MAIN:
            return new MainPackage(name, version, publisher);
        case PackageType::LIBRARY:
            return new LibraryPackage(name, version, publisher);
        case PackageType::META:
            return new MetaPackage(name, "latest", publisher); 
        default:
            return nullptr;
    }
}

Package* createPackage(const std::string& typeStr,
                       const std::string& name,
                       const std::string& version,
                       const std::string& publisher) {
    PackageType type = PackageTypeUtils::fromString(typeStr);
    return createPackage(type, name, version, publisher);
}
