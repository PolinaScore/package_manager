/**
 * @file packagemanager.h
 * @brief Заголовочный файл для класса PackageManager
 *
 * @details
 * Класс PackageManager представляет менеджер пакетов, который управляет
 * установкой, удалением и обновлением пакетов в системе. Использует
 * репозиторий для хранения информации о доступных пакетах.
 */

#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include "repository.h"
#include "packagefactory.h"
#include <vector>
#include <string>
#include <set>
#include <functional>

 /**
  * @class PackageManager
  * @brief Менеджер пакетов для управления операциями с пакетами
  *
  * @ingroup PackageSystem
  *
  * Класс PackageManager предоставляет функциональность для управления
  * жизненным циклом пакетов, включая установку, удаление, обновление
  * и проверку зависимостей.
  */
class PackageManager {
private:
    Repository repository; ///< Репозиторий пакетов

    /**
     * @brief Устанавливает зависимости пакета
     * @param packageName Имя пакета
     * @param installing Множество устанавливаемых пакетов
     * @param installOrder Порядок установки
     * @param errors Список ошибок
     * @return Результат установки зависимостей
     */
    bool installDependencies(const std::string& packageName,
        std::set<std::string>& installing,
        std::vector<std::string>& installOrder,
        std::vector<std::string>& errors);

    /**
     * @brief Проверяет использование библиотеки
     * @param libraryName Имя библиотеки
     * @return Результат проверки
     */
    bool isLibraryUsed(const std::string& libraryName) const;

    /**
     * @brief Получает целевой пакет
     * @param packageName Имя пакета
     * @return Константный указатель на пакет
     */
    const Package* getTargetPackage(const std::string& packageName) const;

    /**
     * @brief Получает целевой пакет
     * @param packageName Имя пакета
     * @return Указатель на пакет
     */
    Package* getTargetPackage(const std::string& packageName);

public:
    /**
     * @brief Конструктор по умолчанию
     */
    PackageManager() = default;

    /**
     * @brief Деструктор
     */
    ~PackageManager() = default;

    /**
     * @brief Удаленный конструктор копирования
     */
    PackageManager(const PackageManager&) = delete;

    /**
     * @brief Удаленный оператор присваивания
     */
    PackageManager& operator=(const PackageManager&) = delete;

    /**
     * @brief Инициализирует менеджер пакетов
     * @param packageNames Список имен пакетов
     * @return Результат инициализации
     */
    bool initialize(const std::vector<std::string>& packageNames);

    /**
     * @struct InstallResult
     * @brief Результат установки пакета
     */
    struct InstallResult {
        bool success;                          ///< Успешность операции
        std::vector<std::string> installedPackages; ///< Установленные пакеты
        std::vector<std::string> errors;       ///< Ошибки при установке
    };

    /**
     * @brief Устанавливает пакет
     * @param packageName Имя пакета
     * @return Результат установки
     */
    InstallResult installPackage(const std::string& packageName);

    /**
     * @struct RemoveResult
     * @brief Результат удаления пакета
     */
    struct RemoveResult {
        bool success;                          ///< Успешность операции
        std::vector<std::string> removedPackages; ///< Удаленные пакеты
        std::vector<std::string> errors;       ///< Ошибки при удалении
    };

    /**
     * @brief Удаляет пакет
     * @param packageName Имя пакета
     * @param removeUnusedLibs Флаг удаления неиспользуемых библиотек
     * @return Результат удаления
     */
    RemoveResult removePackage(const std::string& packageName, bool removeUnusedLibs = true);

    /**
     * @struct CleanupResult
     * @brief Результат очистки неиспользуемых библиотек
     */
    struct CleanupResult {
        size_t removedCount = 0;               ///< Количество удаленных пакетов
        std::vector<std::string> removedPackages; ///< Удаленные пакеты
    };

    /**
     * @brief Удаляет пакет если он не используется
     * @param packageName Имя пакета
     * @param removed Множество удаленных пакетов
     * @param removalOrder Порядок удаления
     * @return Результат удаления
     */
    bool uninstallIfUnused(const std::string& packageName,
        std::set<std::string>& removed,
        std::vector<std::string>& removalOrder);

    /**
     * @brief Проверяет используется ли библиотека с именем libName
     * @param libName Название библиотеки
     * @return bool используется или нет
     */
    bool isUsedLibrary(const std::string& libName) const;

    /**
     * @brief Ищет неиспользуемые библиотеки
     * @param threadCount Количество потоков
     * @return вектор названий неиспользуемых библиотек
     */
    std::vector<std::vector<std::string>> findUnusedLibraries(size_t threadCount) const;

    /**
     * @brief Очищает неиспользуемые библиотеки
     * @param threadCount Количество потоков
     * @return Результат очистки
     */
    CleanupResult cleanupUnusedLibraries(size_t threadCount = 1);

    /**
     * @struct UpdateResult
     * @brief Результат обновления пакетов
     */
    struct UpdateResult {
        size_t updatedCount = 0;               ///< Количество обновленных пакетов
        std::vector<std::string> updatedPackages; ///< Обновленные пакеты
        std::vector<std::string> errors;       ///< Ошибки при обновлении
    };

    /**
     * @brief Обновляет все пакеты
     * @return Результат обновления
     */
    UpdateResult updateAll();

    /**
     * @brief Добавляет пакет в репозиторий
     * @param package Указатель на пакет
     * @return Результат добавления
     */
    bool addPackageToRepository(Package* package);

    /**
     * @brief Добавляет пакет в репозиторий
     * @param name Имя пакета
     * @param type Тип пакета
     * @param version Версия пакета
     * @param publisher Издатель пакета
     * @return Результат добавления
     */
    bool addPackageToRepository(const std::string& name, PackageType type,
        const std::string& version, const std::string& publisher = "");

    /**
     * @brief Получает информацию о пакете
     * @param packageName Имя пакета
     * @return Информация о пакете
     */
    std::string getPackageInfo(const std::string& packageName) const;

    /**
     * @brief Удаляет пакет из репозитория
     * @param packageName Имя пакета
     * @return Результат удаления
     */
    bool deletePackageFromRepository(const std::string& packageName);

    /**
     * @brief Обновляет пакет в репозитории
     * @param packageName Имя пакета
     * @param newVersion Новая версия
     * @return Результат обновления
     */
    bool updatePackageInRepository(const std::string& packageName,
        const std::string& newVersion);

    /**
     * @struct SplitResult
     * @brief Результат разделения библиотеки
     */
    struct SplitResult {
        bool success;                          ///< Успешность операции
        std::vector<std::string> newPackages;  ///< Новые пакеты
        std::string error;                     ///< Ошибка при разделении
    };

    /**
     * @brief Разделяет библиотеку на части
     * @param libraryName Имя библиотеки
     * @param newPartNames Имена новых частей
     * @param newPublishers Издатели новых частей
     * @return Результат разделения
     */
    SplitResult splitLibrary(const std::string& libraryName,
        const std::vector<std::string>& newPartNames,
        const std::vector<std::string>& newPublishers = {});

    /**
     * @struct MergeResult
     * @brief Результат объединения пакетов
     */
    struct MergeResult {
        bool success;                          ///< Успешность операции
        std::string newPackageName;            ///< Имя нового пакета
        std::string error;                     ///< Ошибка при объединении
    };

    /**
     * @brief Объединяет пакеты
     * @param packageNames Имена объединяемых пакетов
     * @param newPackageName Имя нового пакета
     * @param newType Тип нового пакета
     * @param newPublisher Издатель нового пакета
     * @return Результат объединения
     */
    MergeResult mergePackages(const std::vector<std::string>& packageNames,
        const std::string& newPackageName,
        PackageType newType,
        const std::string& newPublisher = "");

    /**
     * @brief Получает имена всех пакетов
     * @return Список имен пакетов
     */
    std::vector<std::string> getAllPackageNames() const;

    /**
     * @brief Получает имена установленных пакетов
     * @return Список имен установленных пакетов
     */
    std::vector<std::string> getInstalledPackageNames() const;

    /**
     * @brief Получает пакеты по типу
     * @param type Тип пакетов
     * @return Список имен пакетов
     */
    std::vector<std::string> getPackagesByType(PackageType type) const;

    /**
     * @brief Находит пакет по имени
     * @param name Имя пакета
     * @return Константный указатель на пакет
     */
    const Package* findPackage(const std::string& name) const;

    /**
     * @brief Находит пакет по имени
     * @param name Имя пакета
     * @return Указатель на пакет
     */
    Package* findPackage(const std::string& name);

    /**
     * @brief Проверяет наличие пакета
     * @param name Имя пакета
     * @return Наличие пакета
     */
    bool hasPackage(const std::string& name) const;

    /**
     * @struct Statistics
     * @brief Статистика пакетов
     */
    struct Statistics {
        size_t totalPackages = 0;      ///< Всего пакетов
        size_t installedPackages = 0;  ///< Установленных пакетов
        size_t mainPackages = 0;       ///< Основных пакетов
        size_t libraryPackages = 0;    ///< Библиотек
        size_t metaPackages = 0;       ///< Мета-пакетов
    };

    /**
     * @brief Получает статистику пакетов
     * @return Статистика пакетов
     */
    Statistics getStatistics() const;

    /**
     * @brief Проверяет целостность системы
     * @param errors Список ошибок
     * @return Результат проверки
     */
    bool checkSystemIntegrity(std::vector<std::string>& errors) const;

    /**
     * @brief Получает репозиторий
     * @return Константная ссылка на репозиторий
     */
    const Repository& getRepository() const { return repository; }

    /**
     * @brief Получает репозиторий
     * @return Ссылка на репозиторий
     */
    Repository& getRepository() { return repository; }
};

#endif