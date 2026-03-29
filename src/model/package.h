/**
 * @file package.h
 * @brief Заголовочный файл для абстрактного базового класса Package
 *
 * @details
 * Класс Package является абстрактным базовым классом для всех типов пакетов
 * в системе управления пакетами. Определяет общий интерфейс и базовую функциональность
 * для работы с пакетами различных типов.
 */

#ifndef PACKAGE_H
#define PACKAGE_H

#include "packagetype.h"
#include <string>
#include <vector>
#include <functional>

 /**
  * @class Package
  * @brief Абстрактный базовый класс для всех типов пакетов
  *
  * @ingroup PackageSystem
  *
  * Класс Package предоставляет общий интерфейс и базовую реализацию
  * для всех специфических типов пакетов. Определяет основные свойства
  * пакета и операции, которые должны поддерживать все производные классы.
  */
class Package {
protected:
    std::string name;               ///< Имя пакета
    std::string latestVersion;      ///< Последняя доступная версия
    std::string installedVersion;   ///< Установленная версия (пустая если не установлен)
    std::string publisher;          ///< Издатель/поддерживающая организация
    std::vector<std::string> dependencies; ///< Список зависимостей

public:
    /**
     * @brief Конструктор Package
     * @param name Название пакета
     * @param latestVersion Последняя доступная версия
     * @param publisher Издатель/поддерживающая организация
     */
    Package(const std::string& name,
        const std::string& latestVersion,
        const std::string& publisher = "");

    /**
     * @brief Виртуальный деструктор
     */
    virtual ~Package() = default;

    /**
     * @brief Получает тип пакета
     * @return Тип пакета
     */
    virtual PackageType getType() const = 0;

    /**
     * @brief Получает строковое представление типа пакета
     * @return Название типа пакета
     */
    virtual std::string getTypeName() const = 0;

    /**
     * @brief Проверяет, может ли пакет быть установлен
     * @return Результат проверки
     */
    virtual bool canBeInstalled() const = 0;

    /**
     * @brief Создает глубокую копию объекта
     * @return Указатель на копию объекта
     */
    virtual Package* clone() const = 0;

    /**
     * @brief Получает имя пакета
     * @return Имя пакета
     */
    std::string getName() const { return name; }

    /**
     * @brief Получает последнюю доступную версию
     * @return Последняя версия
     */
    std::string getLatestVersion() const { return latestVersion; }

    /**
     * @brief Получает установленную версию
     * @return Установленная версия
     */
    std::string getInstalledVersion() const { return installedVersion; }

    /**
     * @brief Получает издателя пакета
     * @return Издатель
     */
    std::string getPublisher() const { return publisher; }

    /**
     * @brief Получает список зависимостей
     * @return Константная ссылка на список зависимостей
     */
    const std::vector<std::string>& getDependencies() const { return dependencies; }

    /**
     * @brief Проверяет, установлен ли пакет
     * @return Результат проверки
     */
    bool isInstalled() const { return !installedVersion.empty(); }

    /**
     * @brief Устанавливает установленную версию
     * @param version Версия для установки
     */
    void setInstalledVersion(const std::string& version) { installedVersion = version; }

    /**
     * @brief Устанавливает последнюю версию
     * @param version Новая последняя версия
     */
    void setLatestVersion(const std::string& version) { latestVersion = version; }

    /**
     * @brief Добавляет зависимость
     * @param packageName Имя пакета-зависимости
     */
    void addDependency(const std::string& packageName);

    /**
     * @brief Удаляет зависимость
     * @param packageName Имя удаляемой зависимости
     */
    void removeDependency(const std::string& packageName);

    /**
     * @brief Проверяет наличие зависимости
     * @param packageName Имя проверяемого пакета
     * @return Наличие зависимости
     */
    bool dependsOn(const std::string& packageName) const;

    /**
     * @brief Очищает список зависимостей
     */
    void clearDependencies() { dependencies.clear(); }

    /**
     * @brief Получает строковое представление пакета
     * @return Строковое представление
     */
    virtual std::string toString() const;

    /**
     * @brief Преобразует тип пакета в строку
     * @param type Тип пакета
     * @return Строковое представление типа
     */
    static std::string typeToString(PackageType type) {
        return PackageTypeUtils::toString(type);
    }

    /**
     * @brief Преобразует строку в тип пакета
     * @param typeStr Строковое представление типа
     * @return Тип пакета
     */
    static PackageType stringToType(const std::string& typeStr) {
        return PackageTypeUtils::fromString(typeStr);
    }
};

#endif