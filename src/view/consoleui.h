/**
 * @file consoleui.h
 * @brief Заголовочный файл для класса ConsoleUI
 *
 * @details
 * Класс ConsoleUI предоставляет консольный пользовательский интерфейс
 * для взаимодействия с системой управления пакетами.
 */

#ifndef CONSOLEUI_H
#define CONSOLEUI_H

#include "../model/packagemanager.h"
#include "../model/packagefactory.h"
#include "../model/repository.h"
#include <string>
#include <vector>
#include <functional>
#include <map>

 /**
  * @class ConsoleUI
  * @brief Консольный пользовательский интерфейс для системы управления пакетами
  *
  * @ingroup UserInterface
  *
  * Класс ConsoleUI реализует паттерн MVC в качестве контроллера и представления,
  * предоставляя пользовательский интерфейс для работы с PackageManager.
  */
class ConsoleUI {
private:
    PackageManager* model;  ///< Указатель на модель
    bool running;           ///< Флаг работы интерфейса

    /**
     * @brief Выводит заголовок
     * @param title Текст заголовка
     */
    void printHeader(const std::string& title) const;

    /**
     * @brief Выводит разделитель
     */
    void printSeparator() const;

    /**
     * @brief Выводит отступ
     * @param level Уровень отступа
     */
    void printIndent(int level = 1) const;

    /**
     * @brief Выводит список пакетов
     * @param packages Список имен пакетов
     * @param title Заголовок списка
     */
    void printPackageList(const std::vector<std::string>& packages, const std::string& title = "") const;

    /**
     * @brief Выводит детальную информацию о пакете
     * @param packageName Имя пакета
     */
    void printPackageDetails(const std::string& packageName) const;

    /**
     * @brief Выводит статистику пакетов
     */
    void printStatistics() const;

    /**
     * @brief Выводит справочную информацию
     */
    void printHelp() const;

    using CommandHandler = std::function<void(const std::vector<std::string>&)>; ///< Тип обработчика команд
    std::map<std::string, CommandHandler> commands; ///< Карта команд

    /**
     * @brief Настраивает обработчики команд
     */
    void setupCommands();

    /**
     * @brief Обрабатывает команду help
     * @param args Аргументы команды
     */
    void handleHelp(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду exit
     * @param args Аргументы команды
     */
    void handleExit(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду list
     * @param args Аргументы команды
     */
    void handleList(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду info
     * @param args Аргументы команды
     */
    void handleInfo(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду install
     * @param args Аргументы команды
     */
    void handleInstall(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду remove
     * @param args Аргументы команды
     */
    void handleRemove(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду add
     * @param args Аргументы команды
     */
    void handleAdd(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду delete
     * @param args Аргументы команды
     */
    void handleDelete(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду update
     * @param args Аргументы команды
     */
    void handleUpdate(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду update-all
     * @param args Аргументы команды
     */
    void handleUpdateAll(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду cleanup
     * @param args Аргументы команды
     */
    void handleCleanup(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду split
     * @param args Аргументы команды
     */
    void handleSplit(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду merge
     * @param args Аргументы команды
     */
    void handleMerge(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду init
     * @param args Аргументы команды
     */
    void handleInit(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду stats
     * @param args Аргументы команды
     */
    void handleStats(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду check
     * @param args Аргументы команды
     */
    void handleCheck(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду dep
     * @param args Аргументы команды
     */
    void handleDep(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду link
     * @param args Аргументы команды
     */
    void handleLink(const std::vector<std::string>& args);

    /**
     * @brief Обрабатывает команду performance
     * @param args Аргументы команды
     */
    void handlePerformance(const std::vector<std::string>& args);

    /**
     * @brief Выполняет тест производительности
     * @param numLibs Количество библиотек
     */
    void performanceTest(size_t numLibs);

    /**
     * @brief Разделяет строку команды на токены
     * @param input Входная строка
     * @return Вектор токенов
     */
    std::vector<std::string> splitCommand(const std::string& input) const;

    /**
     * @brief Выводит сообщение об ошибке
     * @param message Текст сообщения
     */
    void printError(const std::string& message) const;

    /**
     * @brief Выводит сообщение об успехе
     * @param message Текст сообщения
     */
    void printSuccess(const std::string& message) const;

    /**
     * @brief Выводит предупреждение
     * @param message Текст сообщения
     */
    void printWarning(const std::string& message) const;

public:
    /**
     * @brief Конструктор ConsoleUI
     * @param model Указатель на модель
     */
    ConsoleUI(PackageManager* model);

    /**
     * @brief Запускает интерфейс
     */
    void run();

    /**
     * @brief Обрабатывает одну команду
     * @param commandLine Строка команды
     * @return Результат обработки
     */
    bool processCommand(const std::string& commandLine);
};

#endif