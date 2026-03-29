/**
 * @file librarypackage.h
 * @brief Заголовочный файл для класса LibraryPackage
 *
 * @details
 * Класс LibraryPackage представляет пакет библиотеки в системе управления пакетами.
 * Наследуется от абстрактного класса Package и предоставляет специфичную для библиотек
 * функциональность, включая возможность разделения на подпакеты.
 *
 * @see Package
 */

#ifndef LIBRARYPACKAGE_H
#define LIBRARYPACKAGE_H

#include "package.h"
#include "capabilityinterfaces.h"
#include <vector>
#include <string>

 /**
  * @class LibraryPackage
  * @brief Класс, представляющий пакет библиотеки
  *
  * @ingroup PackageTypes
  * @ingroup Libraries
  *
  * LibraryPackage расширяет базовый класс Package, добавляя специфичные для библиотек
  * возможности. Библиотеки могут быть разделены на части (например, runtime, dev, debug),
  * что позволяет устанавливать только необходимые компоненты.
  *
  * @note Библиотеки всегда могут быть установлены (canBeInstalled() возвращает true)
  * @note Поддерживает семантику копирования через метод clone()
  *
  * 
  */
class LibraryPackage : public Package, public IConcretePackage {
public:
    /**
     * @brief Конструктор LibraryPackage
     * @param[in] name Название библиотеки
     * @param[in] latestVersion Последняя доступная версия
     * @param[in] publisher Издатель/поддерживающая организация (опционально)
     *
     * @pre name не должен быть пустым
     * @pre latestVersion должна быть корректной строкой версии
     *
     * @post Создан объект LibraryPackage с заданными параметрами
     * @post Тип пакета установлен в PackageType::Library
     *
     * @throw std::invalid_argument если name пустой
     * @throw std::invalid_argument если latestVersion имеет неверный формат
     *
     * @note Для проверки формата версии используется validateVersion()
     */
    LibraryPackage(const std::string& name,
        const std::string& latestVersion,
        const std::string& publisher = "");

    /**
     * @brief Получает тип пакета
     * @return PackageType::Library
     *
     * @note Переопределяет виртуальный метод Package::getType()
     *
     * @complexity O(1)
     */
    PackageType getType() const override;

    /**
     * @brief Получает строковое представление типа пакета
     * @return Строку "Library"
     *
     * @note Переопределяет виртуальный метод Package::getTypeName()
     *
     * @complexity O(1)
     */
    std::string getTypeName() const override;

    /**
     * @brief Проверяет, может ли пакет быть установлен
     * @return Всегда true для библиотек
     *
     * @details
     * В отличие от некоторых других типов пакетов (например, виртуальных пакетов),
     * библиотеки всегда могут быть установлены в систему.
     *
     * @note Переопределяет виртуальный метод Package::canBeInstalled()
     *
     * @complexity O(1)
     */
    bool canBeInstalled() const override;

    /**
     * @brief Создает глубокую копию объекта
     * @return Указатель на новый объект LibraryPackage
     *
     * @details
     * Создает полную копию текущего объекта, включая все внутренние данные.
     * Вызывающий код отвечает за освобождение памяти.
     *
     * @note Переопределяет виртуальный метод Package::clone()
     * @note Использует идиому виртуального конструктора
     *
     * @complexity O(n) где n - размер данных пакета
     * @throw std::bad_alloc если не удалось выделить память
     *
     * @warning Возвращаемый указатель должен быть удален вызывающим кодом
     */
    Package* clone() const override;

    /**
     * @brief Разделяет библиотеку на несколько подпакетов
     * @param[in] newPartNames Вектор названий для новых подпакетов
     * @param[in] newPublishers Вектор издателей для новых подпакетов (опционально)
     * @return Вектор указателей на созданные подпакеты LibraryPackage
     *
     * @details
     * Позволяет разделить одну библиотеку на несколько компонентов (например,
     * разделение на runtime, development и debug версии). Каждый подпакет
     * наследует версию и часть атрибутов от родительской библиотеки.
     *
     * @pre newPartNames не должен быть пустым
     * @pre Все элементы newPartNames должны быть уникальными
     * @pre Если указан newPublishers, его размер должен совпадать с newPartNames
     *      или быть пустым (тогда используется publisher родительской библиотеки)
     *
     * @post Созданы новые объекты LibraryPackage для каждого имени
     * @post Родительский объект остается неизменным
     *
     * @throw std::invalid_argument если newPartNames пустой
     * @throw std::invalid_argument если имена не уникальны
     * @throw std::invalid_argument если размеры векторов не совпадают (когда newPublishers не пуст)
     * @throw std::bad_alloc если не удалось выделить память
     *
     * @complexity O(m) где m = newPartNames.size()
     *
     *
     */
    std::vector<LibraryPackage*> split(
        const std::vector<std::string>& newPartNames,
        const std::vector<std::string>& newPublishers = {}) const;

    /**
     * @brief Проверяет, используется ли библиотека в системе
     * @return true если библиотека используется, false в противном случае
     *
     * @details
     * Проверяет, ссылаются ли на библиотеку какие-либо исполняемые файлы
     * или другие библиотеки в системе. Это полезно для определения,
     * может ли библиотека быть безопасно обновлена или удалена.
     *
     * @note Реализация зависит от операционной системы
     * @note В Linux может проверять ldd для исполняемых файлов
     * @note В Windows может проверять зависимости через API
     *
     * @complexity O(n) где n - количество проверяемых файлов
     *
     *
     */
    bool isUsed() const;
};

#endif // LIBRARYPACKAGE_H