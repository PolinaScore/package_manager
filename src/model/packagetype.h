/**
 * @file packagetype.h
 * @brief Заголовочный файл для перечисления типов пакетов
 *
 * @details
 * Файл содержит перечисление типов пакетов и вспомогательные функции
 * для преобразования между строковым представлением и перечислением.
 */

#ifndef PACKAGETYPE_H
#define PACKAGETYPE_H

#include <string>

 /**
  * @enum PackageType
  * @brief Перечисление типов пакетов
  *
  * @ingroup PackageSystem
  */
enum class PackageType {
    MAIN,     ///< Основной системный пакет
    LIBRARY,  ///< Пакет библиотеки
    META      ///< Мета-пакет
};

/**
 * @namespace PackageTypeUtils
 * @brief Пространство имен для вспомогательных функций работы с типами пакетов
 */
namespace PackageTypeUtils {
    /**
     * @brief Преобразует тип пакета в строку
     * @param type Тип пакета
     * @return Строковое представление типа
     */
    inline std::string toString(PackageType type) {
        switch (type) {
        case PackageType::MAIN: return "MAIN";
        case PackageType::LIBRARY: return "LIBRARY";
        case PackageType::META: return "META";
        default: return "UNKNOWN";
        }
    }

    /**
     * @brief Преобразует строку в тип пакета
     * @param str Строковое представление типа
     * @return Тип пакета
     */
    inline PackageType fromString(const std::string& str) {
        if (str == "MAIN" || str == "main") return PackageType::MAIN;
        if (str == "LIBRARY" || str == "library" || str == "LIB") return PackageType::LIBRARY;
        if (str == "META" || str == "meta") return PackageType::META;
        return PackageType::MAIN;
    }
}

#endif