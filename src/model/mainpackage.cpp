#include "mainpackage.h"

MainPackage::MainPackage(const std::string& name, 
                         const std::string& latestVersion,
                         const std::string& publisher)
    : Package(name, latestVersion, publisher) {
}

PackageType MainPackage::getType() const {
    return PackageType::MAIN;
}

std::string MainPackage::getTypeName() const {
    return "MAIN";
}

bool MainPackage::canBeInstalled() const {
    return true;  
}

Package* MainPackage::clone() const {
    return new MainPackage(*this);
}

bool MainPackage::canBeRemoved() const {
    return true; 
}


void MainPackage::install() {
    setInstalledVersion(getLatestVersion());
}


void MainPackage::uninstall() {
    setInstalledVersion(""); 
}
