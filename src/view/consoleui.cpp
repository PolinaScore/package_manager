#include "consoleui.h"
#include "../model/mainpackage.h"
#include "../model/librarypackage.h"
#include "../model/metapackage.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <random>
#include <cassert>
#include <chrono>

#define COLOR_RESET "\033[0m"

#define C1 "\033[38;5;197m"
#define C2 "\033[38;5;198m"
#define C3 "\033[38;5;199m"
#define C4 "\033[38;5;200m"
#define C5 "\033[38;5;201m"
#define C6 "\033[38;5;202m"
#define C7 "\033[38;5;203m"
#define C8 "\033[38;5;204m"
#define C9 "\033[38;5;205m"
#define RED "\033[31m"
#define GREEN "\033[32m"


ConsoleUI::ConsoleUI(PackageManager* model) : model(model), running(false) {
    setupCommands();
}

void ConsoleUI::run() {
    running = true;

    printHeader("*** Package Manager ***");
    std::cout << "Type 'help' for list of commands, 'exit' to quit.\n\n";

    while (running) {
        std::cout << C1 << ">>> " << COLOR_RESET;
        std::string input;
        std::getline(std::cin, input);

        if (!processCommand(input)) {
            std::cout << RED << "Unknown command. Type 'help' for help.\n" << COLOR_RESET;
        }
    }
}

bool ConsoleUI::processCommand(const std::string& commandLine) {
    if (commandLine.empty()) return true;

    auto args = splitCommand(commandLine);
    if (args.empty()) return true;

    std::string command = args[0];
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    auto it = commands.find(command);
    if (it != commands.end()) {
        args.erase(args.begin());
        try {
            it->second(args);
        }
        catch (const std::exception& e) {
            printError("Error executing command: " + std::string(e.what()));
        }
        return true;
    }

    return false;
}


void ConsoleUI::printHeader(const std::string& title) const {
    std::cout << "\n" << C1;
    std::cout << "========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n";
    std::cout << COLOR_RESET << "\n";
}

void ConsoleUI::printSeparator() const {
    std::cout << C6 << "----------------------------------------\n" << COLOR_RESET;
}

void ConsoleUI::printIndent(int level) const {
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }
}

void ConsoleUI::printPackageList(const std::vector<std::string>& packages, const std::string& title) const {
    if (!title.empty()) {
        std::cout << C7 << title << " (" << packages.size() << "):\n" << COLOR_RESET;
    }

    if (packages.empty()) {
        std::cout << "  (none)\n";
        return;
    }

    for (size_t i = 0; i < packages.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << packages[i] << "\n";
    }
}

void ConsoleUI::printPackageDetails(const std::string& packageName) const {
    if (!model->hasPackage(packageName)) {
        printError("Package not found: " + packageName);
        return;
    }

    printHeader("Package Info: " + packageName);
    std::cout << model->getPackageInfo(packageName) << "\n";
}

void ConsoleUI::printStatistics() const {
    auto stats = model->getStatistics();

    printHeader("System Statistics");

    std::cout << GREEN << "Total packages: " << COLOR_RESET << stats.totalPackages << "\n";
    std::cout << GREEN << "Installed packages: " << COLOR_RESET << stats.installedPackages << "\n";
    std::cout << GREEN << "Main packages: " << COLOR_RESET << stats.mainPackages << "\n";
    std::cout << GREEN << "Library packages: " << COLOR_RESET << stats.libraryPackages << "\n";
    std::cout << GREEN << "Meta packages: " << COLOR_RESET << stats.metaPackages << "\n";

    auto installed = model->getInstalledPackageNames();
    if (!installed.empty()) {
        std::cout << "\n";
        printPackageList(installed, "Installed packages");
    }
}

void ConsoleUI::printHelp() const {
    printHeader("Available Commands");

    std::cout << C1 << "\t\t\033[5;35mGeneral:\033[0m\n\n" << COLOR_RESET;
    std::cout << "  help                    - Show this help\033[0m\n";
    std::cout << "  exit                    - Exit program\033[0m\n";
    std::cout << "  list [type]             - List packages (all/main/lib/meta)\033[0m\n";
    std::cout << "  info <package>          - Show package information\033[0m\n";
    std::cout << "  stats                   - Show statistics\033[0m\n";
    std::cout << "  check                   - Check system integrity\033[0m\n";

    std::cout << "\n" << C2 << "Package Manager Operations:\n" << COLOR_RESET;
    std::cout << "  init <pkg1> [pkg2...]   - Initialize with packages\033[0m\n";
    std::cout << "  install <package>       - Install package with dependencies\033[0m\n";
    std::cout << "  remove <package>        - Remove package (with unused libs)\033[0m\n";
    std::cout << "  update-all              - Update all installed packages\033[0m\n";
    std::cout << "  cleanup                 - Remove unused libraries\033[0m\n";

    std::cout << "\n" << C3 << "Package Operations:\n" << COLOR_RESET;
    std::cout << "  add <name> <type> <ver> [pub] - Add package to repository\033[0m\n";
    std::cout << "                  For META: version should be 'latest'\033[0m\n";
    std::cout << "  delete <package>        - Delete package from repository\033[0m\n";
    std::cout << "  update <pkg> <version>  - Update package version\033[0m\n";
    std::cout << "  dep <from> <to>         - Add dependency between packages\033[0m\n";
    std::cout << "  link <meta> <target>    - Link META package to target package\033[0m\n";

    std::cout << "\n" << C4 << "Special Operations:\n" << COLOR_RESET;
    std::cout << "  split <lib> <part1> [part2...] - Split library\033[0m\n";
    std::cout << "  merge <pkg1> <pkg2> <new> <type> - Merge packages\033[0m\n";

    std::cout << "\n" << C5 << "Examples:\n" << COLOR_RESET;
    std::cout << "  list main              # List all main packages\033[0m\n";
    std::cout << "  install myapp          # Install myapp with dependencies\033[0m\n";
    std::cout << "  add mylib LIBRARY 1.0 # Add library to repository\033[0m\n";
    std::cout << "  split oldlib newlib1 newlib2 # Split library\033[0m\n";
    std::cout << "  link myapp-meta myapp  # Link META package\033[0m\n";


    std::cout << "  performance - compare single thread with multy thread\033[0m\n";
}

void ConsoleUI::printError(const std::string& message) const {
    std::cout << RED << "[ERROR] " << message << COLOR_RESET << "\n";
}

void ConsoleUI::printSuccess(const std::string& message) const {
    std::cout << GREEN << "[OK] " << message << COLOR_RESET << "\n";
}

void ConsoleUI::printWarning(const std::string& message) const {
    std::cout << C8 << "[WARNING] " << message << COLOR_RESET << "\n";
}



void ConsoleUI::setupCommands() {
    commands = {
        {"help", [this](auto args) { handleHelp(args); }},
        {"exit", [this](auto args) { handleExit(args); }},
        {"list", [this](auto args) { handleList(args); }},
        {"info", [this](auto args) { handleInfo(args); }},
        {"install", [this](auto args) { handleInstall(args); }},
        {"remove", [this](auto args) { handleRemove(args); }},
        {"add", [this](auto args) { handleAdd(args); }},
        {"delete", [this](auto args) { handleDelete(args); }},
        {"update", [this](auto args) { handleUpdate(args); }},
        {"update-all", [this](auto args) { handleUpdateAll(args); }},
        {"cleanup", [this](auto args) { handleCleanup(args); }},
        {"split", [this](auto args) { handleSplit(args); }},
        {"merge", [this](auto args) { handleMerge(args); }},
        {"init", [this](auto args) { handleInit(args); }},
        {"stats", [this](auto args) { handleStats(args); }},
        {"check", [this](auto args) { handleCheck(args); }},
        {"dep", [this](auto args) { handleDep(args); }},
        {"link", [this](auto args) { handleLink(args); }},
        {"performance", [this](auto args) { handlePerformance(args); }}
    };
}

void ConsoleUI::handleHelp(const std::vector<std::string>& /*args*/) {
    printHelp();
}

void ConsoleUI::handleExit(const std::vector<std::string>& /*args*/) {
    running = false;
    std::cout << "Goodbye!\n";
}

void ConsoleUI::handleList(const std::vector<std::string>& args) {
    if (args.empty()) {
        auto all = model->getAllPackageNames();
        printPackageList(all, "All Packages");
        return;
    }

    std::string typeStr = args[0];
    std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), ::tolower);

    PackageType type;
    if (typeStr == "main") type = PackageType::MAIN;
    else if (typeStr == "lib" || typeStr == "library") type = PackageType::LIBRARY;
    else if (typeStr == "meta") type = PackageType::META;
    else {
        printError("Unknown type: " + typeStr + ". Use: main, lib, meta");
        return;
    }

    auto packages = model->getPackagesByType(type);
    printPackageList(packages, typeStr + " packages");
}

void ConsoleUI::handleInfo(const std::vector<std::string>& args) {
    if (args.empty()) {
        printError("Usage: info <package_name>");
        return;
    }

    printPackageDetails(args[0]);
}

void ConsoleUI::handleInstall(const std::vector<std::string>& args) {
    if (args.empty()) {
        printError("Usage: install <package_name>");
        return;
    }

    printHeader("Installing: " + args[0]);


    const Package* pkg = model->findPackage(args[0]);
    if (pkg && pkg->isInstalled()) {
        printWarning("Package already installed: " + args[0]);
        return;
    }

    auto result = model->installPackage(args[0]);

    if (result.success) {
        printSuccess("Package installed successfully");
        if (!result.installedPackages.empty()) {
            std::cout << "Installed packages:\n";
            for (const auto& installedPkg : result.installedPackages) {
                std::cout << "  - " << installedPkg << "\n";
            }
        }
    }
    else {
        printError("Failed to install package");
        for (const auto& err : result.errors) {
            printError("  " + err);
        }
    }
}

void ConsoleUI::handleRemove(const std::vector<std::string>& args) {
    if (args.empty()) {
        printError("Usage: remove <package_name>");
        return;
    }

    printHeader("Removing: " + args[0]);
    auto result = model->removePackage(args[0], true);

    if (result.success) {
        printSuccess("Package removed successfully");
        if (!result.removedPackages.empty()) {
            std::cout << "Removed packages:\n";
            for (const auto& pkg : result.removedPackages) {
                std::cout << "  - " << pkg << "\n";
            }
        }
    }
    else {
        printError("Failed to remove package");
        for (const auto& err : result.errors) {
            printError("  " + err);
        }
    }
}

void ConsoleUI::handleAdd(const std::vector<std::string>& args) {

    if (args.size() < 3) {
        printError("\033[31mUsage: add <name> <type> <version> [publisher]\033[0m");
        printError("\033[31mTypes: MAIN, LIBRARY, META\033[0m");
        printError("\033[31mFor META type, version should be 'latest'\033[0m");
        return;
    }

    std::string name = args[0];
    std::string typeStr = args[1];
    std::string version = args[2];
    std::string publisher = args.size() > 3 ? args[3] : "";

    if (model->hasPackage(name)) { std::cout << "\033[31mCan't add package. There's already such a package in the package manager\033[0m" << std::endl; return; }


    std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), ::toupper);
    if (typeStr == "META" && version != "latest") {
        printWarning("Note: For META packages, version should be 'latest' (using 'latest')");
        version = "latest";
    }


    if (model->addPackageToRepository(name, Package::stringToType(typeStr), version, publisher)) {
        printSuccess("Package added to repository: " + name);


        if (typeStr == "META") {
            printWarning("Note: META packages require linking with 'link' command");
        }
    }
    else {
        printError("Failed to add package (maybe duplicate name or cycle)");
    }
}

void ConsoleUI::handleDelete(const std::vector<std::string>& args) {
    if (args.empty()) {
        printError("Usage: delete <package_name>");
        return;
    }

    if (model->deletePackageFromRepository(args[0])) {
        printSuccess("Package deleted from repository: " + args[0]);
    }
    else {
        printError("Failed to delete package (maybe dependencies exist)");
    }
}

void ConsoleUI::handleUpdate(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        printError("Usage: update <package_name> <new_version>");
        return;
    }

    if (model->updatePackageInRepository(args[0], args[1])) {
        printSuccess("Package updated: " + args[0] + " -> " + args[1]);
    }
    else {
        printError("Failed to update package");
    }
}

void ConsoleUI::handleUpdateAll(const std::vector<std::string>& /*args*/) {
    printHeader("Updating all packages");
    auto result = model->updateAll();

    if (result.updatedCount > 0) {
        printSuccess("Updated " + std::to_string(result.updatedCount) + " packages:");
        for (const auto& update : result.updatedPackages) {
            std::cout << "  - " << update << "\n";
        }
    }
    else {
        printSuccess("All packages are up to date");
    }
}

void ConsoleUI::handleCleanup(const std::vector<std::string>& /*args*/) {
    printHeader("Cleaning up unused libraries");
    auto result = model->cleanupUnusedLibraries(2);

    if (result.removedCount > 0) {
        printSuccess("Removed " + std::to_string(result.removedCount) + " unused libraries:");
        for (const auto& lib : result.removedPackages) {
            std::cout << "  - " << lib << "\n";
        }
    }
    else {
        printSuccess("No unused libraries found");
    }
}




void ConsoleUI::performanceTest(size_t numLibs) {

    PackageManager tempManager;

    std::vector<std::string> libraryNames;
    for (size_t i{ 0 }; i < numLibs; ++i) {
        std::string libName = "test_lib_" + std::to_string(i);
        if (tempManager.addPackageToRepository(libName, PackageType::LIBRARY, "1.0", "TestPublisher")) {
            libraryNames.push_back(libName);
        }
    }


    size_t numMainPackages = numLibs / 2;
    std::vector<std::string> mainPackageNames;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> libDist(0, numLibs - 1);
    std::uniform_int_distribution<int> depsDist(1, 5);

    for (size_t i = 0; i < numMainPackages; ++i) {
        std::string mainName = "test_app_" + std::to_string(i);

        if (tempManager.addPackageToRepository(mainName, PackageType::MAIN, "1.0", "TestPublisher")) {
            mainPackageNames.push_back(mainName);

            int numDeps = depsDist(rng);
            std::set<size_t> usedLibs;

            for (int d = 0; d < numDeps; d++) {
                size_t libIdx = libDist(rng);
                if (usedLibs.insert(libIdx).second) {
                    tempManager.getRepository().addDependency(mainName, libraryNames[libIdx]);
                }
            }

            if (i % 2 == 0) {
                tempManager.installPackage(mainName);
            }
        }
    }


    PackageManager singleThreadManager;

    for (const auto& name : tempManager.getAllPackageNames()) {
        const Package* sourcePkg = tempManager.findPackage(name);
        if (sourcePkg) {
            singleThreadManager.addPackageToRepository(name, sourcePkg->getType(), sourcePkg->getLatestVersion(), sourcePkg->getPublisher());

            Package* destPkg = singleThreadManager.findPackage(name);
            if (destPkg && sourcePkg->isInstalled()) {
                destPkg->setInstalledVersion(sourcePkg->getInstalledVersion());
            }
        }
    }

    for (const auto& name : tempManager.getAllPackageNames()) {
        auto deps = tempManager.getRepository().getDependencies(name);
        for (const auto& dep : deps) {
            singleThreadManager.getRepository().addDependency(name, dep);
        }
    }


    // Тестирование однопоточной очистки
    auto start1 = std::chrono::high_resolution_clock::now();
    auto result1 = singleThreadManager.cleanupUnusedLibraries(1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);

    // Тестирование многопоточной очистки
    auto start2 = std::chrono::high_resolution_clock::now();
    auto result2 = tempManager.cleanupUnusedLibraries(2);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto time2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);

    std::cout << std::left;
    std::cout << std::setw(10) << numLibs;
    std::cout << std::setw(20) << time1.count() / 1000.0;
    std::cout << std::setw(20) << time2.count() / 1000.0;
    std::cout << '\n';

}


void ConsoleUI::handlePerformance(const std::vector<std::string>&) {
    std::cout << C2 << "***PERFORMANCE TEST***" << COLOR_RESET << "\n";
    std::cout << std::left;
    std::cout << std::setw(10) << "numLibs";
    std::cout << std::setw(20) << "1 thread, ms";
    std::cout << std::setw(20) << "2 threads, ms";
    std::cout << '\n';

    for (size_t numLibs = 100; numLibs < 5000; numLibs += 100) {
        performanceTest(numLibs);
    }
}


void ConsoleUI::handleSplit(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        printError("Usage: split <library_name> <part1> [part2 ...]");
        return;
    }

    std::string libName = args[0];
    std::vector<std::string> newParts(args.begin() + 1, args.end());

    printHeader("Splitting library: " + libName);
    auto result = model->splitLibrary(libName, newParts);

    if (result.success) {
        printSuccess("Library split successfully");
        std::cout << "New libraries:\n";
        for (const auto& part : result.newPackages) {
            std::cout << "  - " << part << "\n";
        }
    }
    else {
        printError(result.error);
    }
}

void ConsoleUI::handleMerge(const std::vector<std::string>& args) {
    if (args.size() < 4) {
        printError("Usage: merge <pkg1> <pkg2> <new_name> <type>");
        printError("Types: MAIN, LIBRARY");
        return;
    }

    std::vector<std::string> packages = { args[0], args[1] };
    std::string newName = args[2];
    PackageType newType = Package::stringToType(args[3]);

    printHeader("Merging packages into: " + newName);
    auto result = model->mergePackages(packages, newName, newType);

    if (result.success) {
        printSuccess("Packages merged successfully: " + result.newPackageName);
    }
    else {
        printError(result.error);
    }
}

void ConsoleUI::handleInit(const std::vector<std::string>& args) {
    if (args.empty()) {
        printError("Usage: init <pkg1> [pkg2 ...]");
        return;
    }

    printHeader("Initializing system with packages");
    if (model->initialize(args)) {
        printSuccess("System initialized successfully");
    }
    else {
        printError("Failed to initialize system");
    }
}

void ConsoleUI::handleStats(const std::vector<std::string>& /*args*/) {
    printStatistics();
}

void ConsoleUI::handleCheck(const std::vector<std::string>& /*args*/) {
    printHeader("System Integrity Check");

    std::vector<std::string> errors;
    if (model->checkSystemIntegrity(errors)) {
        printSuccess("System integrity: OK");
    }
    else {
        printError("System integrity: FAILED");
        for (const auto& err : errors) {
            printError("  " + err);
        }
    }
}

void ConsoleUI::handleDep(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        printError("Usage: dep <from_package> <to_package>");
        return;
    }

    if (model->getRepository().addDependency(args[0], args[1])) {
        printSuccess("Dependency added: " + args[0] + " -> " + args[1]);
    }
    else {
        printError("Failed to add dependency (maybe cycle or package not found)");
    }
}

void ConsoleUI::handleLink(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        printError("Usage: link <meta_package> <target_package>");
        return;
    }

    std::string metaName = args[0];
    std::string targetName = args[1];

    Package* metaPkg = model->findPackage(metaName);
    if (!metaPkg) {
        printError("Package not found: " + metaName);
        return;
    }

    if (metaPkg->getType() != PackageType::META) {
        printError("Package is not META type: " + metaName);
        return;
    }

    if (!model->hasPackage(targetName)) {
        printError("Target package not found: " + targetName);
        return;
    }

    IRedirectable* redirectable = dynamic_cast<IRedirectable*>(metaPkg);
    if (!redirectable) {
        printError("Internal error: cannot cast to MetaPackage");
        return;
    }
    redirectable->setLinkedPackageName(targetName);
    printSuccess("Linked " + metaName + " -> " + targetName);
}


std::vector<std::string> ConsoleUI::splitCommand(const std::string& input) const {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        if (!token.empty() && token.front() == '"' && token.back() == '"') {
            token = token.substr(1, token.size() - 2);
        }
        result.push_back(token);
    }

    return result;
}
