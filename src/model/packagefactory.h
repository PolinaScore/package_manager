/**
 * @file packagefactory.h
 * @brief Заголовочный файл для фабрики создания пакетов
 *
 * @details
 * Файл содержит объявления функций фабрики для создания объектов пакетов
 * различных типов. Фабрика обеспечивает создание объектов на основе типа пакета.
 */

#ifndef PACKAGEFACTORY_H
#define PACKAGEFACTORY_H

#include "packagetype.h"
#include <string>

class Package;  ///< Предварительное объявление класса Package

/**
 * @brief Создает пакет указанного типа
 * @param type Тип создаваемого пакета
 * @param name Имя пакета
 * @param version Версия пакета
 * @param publisher Издатель пакета
 * @return Указатель на созданный пакет
 */
Package* createPackage(PackageType type,
    const std::string& name,
    const std::string& version,
    const std::string& publisher = "");

/**
 * @brief Создает пакет на основе строкового представления типа
 * @param typeStr Строковое представление типа пакета
 * @param name Имя пакета
 * @param version Версия пакета
 * @param publisher Издатель пакета
 * @return Указатель на созданный пакет
 */
Package* createPackage(const std::string& typeStr,
    const std::string& name,
    const std::string& version,
    const std::string& publisher = "");

#endif