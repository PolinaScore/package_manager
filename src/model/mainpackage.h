/**
 * @file mainpackage.h
 * @brief Заголовочный файл для класса MainPackage
 *
 * @details
 * Класс MainPackage представляет основной системный пакет в системе управления пакетами.
 * Наследуется от абстрактного класса Package и реализует специфичное поведение
 * для критических системных пакетов, которые имеют ограничения на удаление.
 *
 * @see Package
 * @see LibraryPackage
 * @see VirtualPackage
 */

#ifndef MAINPACKAGE_H
#define MAINPACKAGE_H

#include "package.h"
#include "capabilityinterfaces.h"

 /**
  * @class MainPackage
  * @brief Класс, представляющий основной системный пакет
  *
  * @ingroup PackageTypes
  * @ingroup SystemPackages
  *
  * MainPackage представляет критические системные пакеты, которые являются
  * фундаментальными для работы системы. Эти пакеты обычно:
  * - Устанавливаются по умолчанию с системой
  * - Не могут быть удалены без специальных прав или условий
  * - Содержат ключевые системные компоненты (ядро, оболочка, системные утилиты)
  * - Часто имеют зависимости от других основных пакетов
  *
  * @note Основные пакеты всегда могут быть установлены (canBeInstalled() возвращает true)
  * @note Могут иметь ограничения на удаление (проверяется через canBeRemoved())
  * @note Поддерживает семантику копирования через метод clone()
  *
  */
class MainPackage : public Package, public IInstallable, public IRemovable, public IConcretePackage {
public:
    /**
     * @brief Конструктор MainPackage
     * @param[in] name Название основного пакета
     * @param[in] latestVersion Последняя доступная версия
     * @param[in] publisher Издатель/поддерживающая организация (опционально)
     *
     * @details
     * Создает объект основного системного пакета. Основные пакеты обычно представляют
     * критические компоненты системы, такие как ядро, системные библиотеки, оболочки и т.д.
     *
     * @throw std::invalid_argument если name пустой
     * @throw std::invalid_argument если latestVersion имеет неверный формат
     * @throw std::invalid_argument если имя не соответствует соглашению об именовании
     *
     * @note Для проверки формата версии используется validateVersion()
     * @note Имена системных пакетов обычно содержат префиксы: "system-", "core-", "base-"
     *
     * @complexity O(1)
     */
    MainPackage(const std::string& name,
        const std::string& latestVersion,
        const std::string& publisher = "");

    /**
     * @brief Получает тип пакета
     * @return PackageType::Main
     *
     * @details
     * Возвращает тип пакета, указывающий что это основной системный пакет.
     * Используется для проверки типа во время выполнения и для логирования.
     *
     * @note Переопределяет виртуальный метод Package::getType()
     * @note Гарантированно возвращает PackageType::Main
     *
     * @complexity O(1)
     */
    PackageType getType() const override;

    /**
     * @brief Получает строковое представление типа пакета
     * @return Строку "Main"
     *
     * @details
     * Возвращает человекочитаемое название типа пакета.
     * Используется для отображения в пользовательском интерфейсе и логирования.
     *
     * @note Переопределяет виртуальный метод Package::getTypeName()
     * @note Всегда возвращает "Main" для объектов MainPackage
     *
     * @complexity O(1)
     */
    std::string getTypeName() const override;

    /**
     * @brief Проверяет, может ли пакет быть установлен
     * @return Всегда true для основных пакетов
     *
     * @details
     * Основные системные пакеты всегда могут быть установлены, так как они
     * представляют фундаментальные компоненты системы. Однако установка может
     * потребовать специальных прав или условий.
     *
     * @note Переопределяет виртуальный метод Package::canBeInstalled()
     * @note Даже если метод возвращает true, установка может требовать прав root/администратора
     *
     * @complexity O(1)
     */
    bool canBeInstalled() const override;

    /**
     * @brief Создает глубокую копию объекта
     * @return Указатель на новый объект MainPackage
     *
     * @details
     * Создает полную копию текущего объекта, включая все внутренние данные и флаги.
     * Используется для реализации полиморфного копирования в иерархии Package.
     *
     * @note Переопределяет виртуальный метод Package::clone()
     * @note Реализует идиому виртуального конструктора
     * @note Копия включает все атрибуты оригинального пакета
     *
     * @complexity O(n) где n - размер данных пакета
     * @throw std::bad_alloc если не удалось выделить память
     *
     * 
     */
    Package* clone() const override;

    /**
     * @brief Проверяет, может ли пакет быть удален из системы
     * @return true если пакет может быть безопасно удален, false в противном случае
     *
     * @details
     * Проверяет, является ли пакет критическим для работы системы и можно ли его
     * безопасно удалить. Метод анализирует:
     * - Наличие зависимостей от других пакетов
     * - Использование пакета системными процессами
     * - Критичность пакета для базовой функциональности
     * - Наличие альтернативных пакетов
     *
     *
     * @post Не изменяет состояние пакета
     *
     * @complexity O(m + n) где:
     *   - m: количество проверяемых зависимостей
     *   - n: количество проверяемых процессов
     *
     * @throw std::runtime_error если не удалось выполнить проверку из-за недостатка прав
     *
     *
     * @see isCriticalSystemComponent()
     * @see hasActiveDependents()
     * @see hasReplacementAvailable()
     */
    void install() override;

    void uninstall() override;
    bool canBeRemoved() const;
};

#endif // MAINPACKAGE_H