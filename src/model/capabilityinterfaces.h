#ifndef CAPABILITYINTERFACES_H
#define CAPABILITYINTERFACES_H


#include <string>


struct IInstallable {
    virtual ~IInstallable() = default;
    virtual void install() = 0;
    virtual bool canBeInstalled() const = 0;
};


struct IRemovable {
    virtual ~IRemovable() = default;
    virtual void uninstall() = 0;
};


struct IRedirectable {
    virtual ~IRedirectable() = default;
    virtual std::string getLinkedPackageName() const = 0;
    virtual void setLinkedPackageName(const std::string& name) = 0;
};


struct IConcretePackage {
    virtual ~IConcretePackage() = default;
};

#endif