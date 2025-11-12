# Makefile для проекта оценки параметров распределений

# Компилятор и флаги
CXX = g++
BOOST_PREFIX = $(shell brew --prefix boost)
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I./include -I$(BOOST_PREFIX)/include
LDFLAGS = -L$(BOOST_PREFIX)/lib -lboost_math_tr1

# Директории
SRC_DIR = src
INCLUDE_DIR = include
OUTPUT_DIR = output

# Исходные файлы
SOURCES = main.cpp \
          $(SRC_DIR)/boost_distributions.cpp \
          $(SRC_DIR)/matrix_operations.cpp \
          $(SRC_DIR)/nelder_mead.cpp \
          $(SRC_DIR)/mle_methods.cpp \
          $(SRC_DIR)/confidence_intervals.cpp \
          $(SRC_DIR)/order.cpp \
          $(SRC_DIR)/statistical_tests.cpp

# Объектные файлы
OBJECTS = $(SOURCES:.cpp=.o)

# Исполняемый файл
TARGET = mle_estimator

# Цель по умолчанию
all: directories $(TARGET)

# Создание необходимых директорий
directories:
	@mkdir -p $(OUTPUT_DIR)

# Линковка
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Сборка завершена: $(TARGET)"

# Компиляция объектных файлов
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(OUTPUT_DIR)/*.txt
	@echo "Очистка выполнена"

# Очистка только объектных файлов
clean-obj:
	rm -f $(OBJECTS)
	@echo "Объектные файлы удалены"

# Запуск программы
run: $(TARGET)
	./$(TARGET)

# Полная очистка и пересборка
rebuild: clean all

# Запуск с визуализацией (визуализация теперь встроена в программу)
visualize: run
	@echo "\nПрограмма автоматически создала все графики!"
	@echo "Список созданных файлов:"
	@ls -lh output/*.png

# Проверка зависимостей
check-deps:
	@echo "Проверка установленных зависимостей..."
	@which $(CXX) > /dev/null || (echo "Ошибка: g++ не найден" && exit 1)
	@echo "✓ g++ найден: $$($(CXX) --version | head -n1)"
	@echo "✓ Проверка Boost..."
	@echo "#include <boost/math/distributions.hpp>" | $(CXX) -x c++ -E - > /dev/null 2>&1 || \
		(echo "✗ Boost Math не найден. Установите: brew install boost" && exit 1)
	@echo "✓ Boost найден"
	@which python3 > /dev/null || (echo "⚠ Python3 не найден (требуется для визуализации)" && exit 0)
	@echo "✓ Python3 найден: $$(python3 --version)"
	@echo "\nВсе зависимости установлены!"

# Помощь
help:
	@echo "Доступные команды:"
	@echo "  make              - Сборка проекта"
	@echo "  make run          - Сборка и запуск (с автоматической визуализацией)"
	@echo "  make clean        - Удаление скомпилированных файлов"
	@echo "  make clean-obj    - Удаление только объектных файлов"
	@echo "  make rebuild      - Полная пересборка"
	@echo "  make visualize    - То же что и 'make run' (визуализация встроена)"
	@echo "  make check-deps   - Проверка зависимостей"
	@echo "  make help         - Показать эту справку"
	@echo "  make gui          - Очистить, собрать и запустить GUI (рекомендуется)"
	@echo "  make gui-help     - Показать все GUI команды"
	@echo ""
	@echo "Примечание: Программа автоматически создает все графики при каждом запуске!"

# Зависимости заголовочных файлов
main.o: $(INCLUDE_DIR)/mle_methods.h $(INCLUDE_DIR)/nelder_mead.h \
        $(INCLUDE_DIR)/boost_distributions.h $(INCLUDE_DIR)/matrix_operations.h \
        $(INCLUDE_DIR)/confidence_intervals.h

$(SRC_DIR)/boost_distributions.o: $(INCLUDE_DIR)/boost_distributions.h
$(SRC_DIR)/matrix_operations.o: $(INCLUDE_DIR)/matrix_operations.h
$(SRC_DIR)/nelder_mead.o: $(INCLUDE_DIR)/nelder_mead.h
$(SRC_DIR)/mle_methods.o: $(INCLUDE_DIR)/mle_methods.h $(INCLUDE_DIR)/boost_distributions.h \
                           $(INCLUDE_DIR)/nelder_mead.h $(INCLUDE_DIR)/matrix_operations.h
$(SRC_DIR)/mle_normal.o: $(INCLUDE_DIR)/mle_methods.h $(INCLUDE_DIR)/boost_distributions.h \
                          $(INCLUDE_DIR)/nelder_mead.h $(INCLUDE_DIR)/matrix_operations.h
$(SRC_DIR)/mle_weibull.o: $(INCLUDE_DIR)/mle_methods.h $(INCLUDE_DIR)/boost_distributions.h \
                           $(INCLUDE_DIR)/nelder_mead.h $(INCLUDE_DIR)/matrix_operations.h
$(SRC_DIR)/mls_normal.o: $(INCLUDE_DIR)/mle_methods.h $(INCLUDE_DIR)/boost_distributions.h \
                          $(INCLUDE_DIR)/nelder_mead.h $(INCLUDE_DIR)/matrix_operations.h
$(SRC_DIR)/mls_weibull.o: $(INCLUDE_DIR)/mle_methods.h $(INCLUDE_DIR)/boost_distributions.h \
                           $(INCLUDE_DIR)/nelder_mead.h $(INCLUDE_DIR)/matrix_operations.h
$(SRC_DIR)/confidence_intervals.o: $(INCLUDE_DIR)/confidence_intervals.h $(INCLUDE_DIR)/boost_distributions.h
$(SRC_DIR)/statistical_tests.o: $(INCLUDE_DIR)/statistical_tests.h $(INCLUDE_DIR)/boost_distributions.h

.PHONY: all clean clean-obj run rebuild visualize check-deps help directories

# ==================== GUI TARGETS ====================

# Сборка GUI
gui-build:
	@echo "Сборка GUI приложения..."
	@mkdir -p gui/build
	@cd gui/build && cmake .. > /dev/null 2>&1 && cmake --build . --config Release
	@echo "✓ GUI собран успешно!"

# Запуск GUI (с автоочисткой перед сборкой) - запуск через терминал для корректной работы Python
gui: gui-clean gui-build
	@echo "Запуск GUI приложения..."
	@cd gui/build && ./StatisticalAnalysisGUI.app/Contents/MacOS/StatisticalAnalysisGUI

# Запуск GUI напрямую (без пересборки) - запуск через терминал
gui-run:
	@echo "Запуск GUI приложения..."
	@cd gui/build && ./StatisticalAnalysisGUI.app/Contents/MacOS/StatisticalAnalysisGUI

# Запуск GUI в фоновом режиме (через Finder) - ВНИМАНИЕ: Python скрипты могут не работать при запуске через open
gui-open:
	@echo "Запуск GUI приложения в фоновом режиме (Python скрипты могут не работать)..."
	@cd gui/build && open StatisticalAnalysisGUI.app

# Очистка GUI
gui-clean:
	@echo "Очистка GUI..."
	@rm -rf gui/build
	@echo "✓ GUI очищен"

# Пересборка GUI
gui-rebuild: gui-clean gui-build

# Помощь для GUI
gui-help:
	@echo "GUI команды:"
	@echo "  make gui          - Очистить, собрать и запустить GUI (рекомендуется)"
	@echo "  make gui-build    - Только сборка GUI (без очистки)"
	@echo "  make gui-run      - Запустить GUI в терминале (с логами, без пересборки)"
	@echo "  make gui-open     - Запустить GUI в фоновом режиме (без пересборки)"
	@echo "  make gui-clean    - Очистка GUI"
	@echo "  make gui-rebuild  - То же что и 'make gui' (очистка + сборка)"

.PHONY: gui gui-build gui-run gui-open gui-clean gui-rebuild gui-help
