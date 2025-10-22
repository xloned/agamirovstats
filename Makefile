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
          $(SRC_DIR)/order.cpp

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
	@echo ""
	@echo "Примечание: Программа автоматически создает все 7 графиков при каждом запуске!"

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

.PHONY: all clean clean-obj run rebuild visualize check-deps help directories
