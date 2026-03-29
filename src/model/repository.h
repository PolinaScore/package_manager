    /**
 * @file repository.h
 * @brief Заголовочный файл для класса Repository
 *
 * @details
 * Класс Repository представляет репозиторий пакетов, который хранит
 * информацию о всех доступных пакетах и их зависимостях. Использует
 * хэш-таблицу для эффективного доступа к пакетам по имени.
 */

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "hashtable.h"
#include "package.h"
#include "capabilityinterfaces.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>

 /**
  * @class Repository
  * @brief Репозиторий для хранения и управления пакетами
  *
  * @ingroup PackageSystem
  *
  * Класс Repository предоставляет функциональность для хранения пакетов,
  * управления зависимостями между ними и выполнения операций поиска.
  * Поддерживает отслеживание прямых и обратных зависимостей.
  */
    class Repository {
    private:
        HashTable<std::string, Package*> packages; ///< Хэш-таблица пакетов

        std::map<std::string, std::vector<std::string>> reverseDeps; ///< Обратные зависимости

        /**
         * @brief Добавляет обратную зависимость
         * @param packageName Имя пакета
         * @param dependsOn Имя зависимости
         */
        void addReverseDependency(const std::string& packageName, const std::string& dependsOn);

        /**
         * @brief Удаляет обратную зависимость
         * @param packageName Имя пакета
         * @param dependsOn Имя зависимости
         */
        void removeReverseDependency(const std::string& packageName, const std::string& dependsOn);

        /**
         * @brief Проверяет наличие циклов в зависимостях
         * @param packageName Имя пакета
         * @param visited Множество посещенных пакетов
         * @param recursionStack Множество пакетов в стеке рекурсии
         * @return Наличие цикла
         */
        bool hasCycleDFS(const std::string& packageName,
            std::set<std::string>& visited,
            std::set<std::string>& recursionStack) const;

        /**
         * @brief Получает все зависимости пакета
         * @param packageName Имя пакета
         * @param result Множество зависимостей
         * @param visited Множество посещенных пакетов
         */
        void getAllDependencies(const std::string& packageName,
            std::set<std::string>& result,
            std::set<std::string>& visited) const;

    public:
        /**
         * @brief Конструктор репозитория
         */
        Repository();

        /**
         * @brief Деструктор репозитория
         */
        ~Repository();

        /**
         * @brief Удаленный конструктор копирования
         */
        Repository(const Repository&) = delete;

        /**
         * @brief Удаленный оператор присваивания
         */
        Repository& operator=(const Repository&) = delete;

        /**
         * @brief Добавляет пакет в репозиторий
         * @param package Указатель на пакет
         * @return Результат добавления
         */
        bool addPackage(Package* package);

        /**
         * @brief Получает пакет по имени
         * @param packageName Имя пакета
         * @return Указатель на пакет
         */
        Package* getPackage(const std::string& packageName);

        /**
         * @brief Получает пакет по имени
         * @param packageName Имя пакета
         * @return Константный указатель на пакет
         */
        const Package* getPackage(const std::string& packageName) const;

        /**
         * @brief Проверяет наличие пакета
         * @param packageName Имя пакета
         * @return Наличие пакета
         */
        bool hasPackage(const std::string& packageName) const;

        /**
         * @brief Обновляет версию пакета
         * @param packageName Имя пакета
         * @param newVersion Новая версия
         * @return Результат обновления
         */
        bool updatePackage(const std::string& packageName, const std::string& newVersion);

        /**
         * @brief Удаляет пакет из репозитория
         * @param packageName Имя пакета
         * @return Результат удаления
         */
        bool removePackage(const std::string& packageName);

        /**
         * @brief Добавляет зависимость между пакетами
         * @param fromPackage Имя пакета-источника
         * @param toPackage Имя пакета-зависимости
         * @return Результат добавления
         */
        bool addDependency(const std::string& fromPackage, const std::string& toPackage);

        /**
         * @brief Удаляет зависимость между пакетами
         * @param fromPackage Имя пакета-источника
         * @param toPackage Имя пакета-зависимости
         * @return Результат удаления
         */
        bool removeDependency(const std::string& fromPackage, const std::string& toPackage);

        /**
         * @brief Получает зависимости пакета
         * @param packageName Имя пакета
         * @return Список зависимостей
         */
        std::vector<std::string> getDependencies(const std::string& packageName) const;

        /**
         * @brief Получает обратные зависимости пакета
         * @param packageName Имя пакета
         * @return Список обратных зависимостей
         */
        std::vector<std::string> getReverseDependencies(const std::string& packageName) const;

        /**
         * @brief Получает все транзитивные зависимости пакета
         * @param packageName Имя пакета
         * @return Список транзитивных зависимостей
         */
        std::vector<std::string> getAllTransitiveDependencies(const std::string& packageName) const;

        /**
         * @brief Проверяет наличие циклов в зависимостях
         * @return Наличие циклов
         */
        bool hasCycles() const;

        /**
         * @brief Проверяет наличие циклов из указанного пакета
         * @param packageName Имя пакета
         * @return Наличие цикла
         */
        bool hasCycleFromPackage(const std::string& packageName) const;

        /**
         * @brief Получает информацию о пакете
         * @param packageName Имя пакета
         * @return Информация о пакете
         */
        std::string getPackageInfo(const std::string& packageName) const;

        /**
         * @struct TypeCount
         * @brief Количество пакетов по типам
         */
        struct TypeCount {
            size_t main = 0;     ///< Количество основных пакетов
            size_t library = 0;  ///< Количество библиотек
            size_t meta = 0;     ///< Количество мета-пакетов
        };

        /**
         * @brief Считает пакеты по типам
         * @return Количество пакетов по типам
         */
        TypeCount countPackagesByType() const;

        /**
         * @brief Получает пакеты по типу
         * @param type Тип пакетов
         * @return Список имен пакетов
         */
        std::vector<std::string> getPackagesByType(PackageType type) const;

        /**
         * @brief Получает установленные пакеты
         * @return Список установленных пакетов
         */
        std::vector<std::string> getInstalledPackages() const;

        /**
         * @brief Получает имена всех пакетов
         * @return Список имен пакетов
         */
        std::vector<std::string> getAllPackageNames() const;

        /**
         * @brief Получает связанный пакет для мета-пакета
         * @param metaPackageName Имя мета-пакета
         * @return Указатель на связанный пакет
         */
        Package* getLinkedPackage(const std::string& metaPackageName);

        /**
         * @brief Получает связанный пакет для мета-пакета
         * @param metaPackageName Имя мета-пакета
         * @return Константный указатель на связанный пакет
         */
        const Package* getLinkedPackage(const std::string& metaPackageName) const;

        /**
         * @brief Разделяет библиотеку на части
         * @param libraryName Имя библиотеки
         * @param newPartNames Имена новых частей
         * @param newPublishers Издатели новых частей
         * @return Результат разделения
         */
        bool splitLibrary(const std::string& libraryName,
            const std::vector<std::string>& newPartNames,
            const std::vector<std::string>& newPublishers = {});

        /**
         * @brief Объединяет пакеты
         * @param packageNames Имена объединяемых пакетов
         * @param newPackageName Имя нового пакета
         * @param newType Тип нового пакета
         * @param newPublisher Издатель нового пакета
         * @return Результат объединения
         */
        bool mergePackages(const std::vector<std::string>& packageNames,
            const std::string& newPackageName,
            PackageType newType,
            const std::string& newPublisher = "");

        /**
         * @brief Очищает репозиторий
         */
        void clear();

        /**
         * @brief Получает количество пакетов
         * @return Количество пакетов
         */
        size_t size() const { return packages.size(); }

        /**
         * @brief Проверяет пустоту репозитория
         * @return Результат проверки
         */
        bool empty() const { return packages.empty(); }

        /**
         * @brief Получает все пакеты
         * @return Константная ссылка на хэш-таблицу пакетов
         */
        const HashTable<std::string, Package*>& getAllPackages() const { return packages; }

    private:
        /**
         * @brief Удаляет все пакеты
         */
        void deleteAllPackages();
    };

#endif
