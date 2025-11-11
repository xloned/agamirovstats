# MLE/MLS Оценка параметров распределений

Проект для оценки параметров нормального распределения и распределения Вейбулла методами максимального правдоподобия (MLE) и метода наименьших квадратов (MLS) с поддержкой цензурированных данных.

## Версии приложения

- **CLI** - Консольное приложение для Linux/macOS/Windows
- **GUI (Qt)** - Графическое приложение для macOS (ветка `feature/macos-gui-support`)
- **GUI (Windows Forms)** - Графическое приложение для Windows (ветка `feature/windows-forms`)

## Возможности

- **MLE/MLS оценка параметров** для нормального распределения и распределения Вейбулла
- **Поддержка цензурированных данных** (правая цензура)
- **Доверительные интервалы** для параметров и персентилей:
  - При известной σ (нормальное распределение)
  - При неизвестной σ (распределение Стьюдента)
  - При неизвестном μ (χ² распределение для дисперсии)
- **Вычисление персентилей** (квантилей) с 95% ДИ
- **Визуализация результатов** (7 графиков)
- **Кроссплатформенность** (Linux, macOS, Windows)

## Требования

### Все платформы

- **C++ компилятор** с поддержкой C++17 (g++, clang++, MSVC)
- **Boost Math Library** (версия 1.70+)
- **Python 3.7+** для визуализации
- **Python библиотеки**: numpy, matplotlib, scipy

### Установка зависимостей

#### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install g++ libboost-math-dev python3 python3-pip python3-venv
```

#### macOS

```bash
# Установка Homebrew (если не установлен)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Установка зависимостей
brew install boost python3
```

#### Windows (MinGW/MSYS2)

1. Установите [MSYS2](https://www.msys2.org/)
2. В MSYS2 терминале:

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-boost python3 python3-pip
```

## Установка

### 1. Клонирование репозитория

```bash
git clone https://github.com/xloned/agamirovstats.git
cd agamirovstats
```

### 2. Установка Python зависимостей

#### Linux/macOS

```bash
python3 -m venv python/venv
source python/venv/bin/activate
pip install numpy matplotlib scipy
deactivate
```

#### Windows

```cmd
python -m venv python\venv
python\venv\Scripts\activate
pip install numpy matplotlib scipy
deactivate
```

### 3. Сборка проекта

#### Linux/macOS

```bash
make
```

#### Windows (MinGW)

```cmd
# Используйте специальный Makefile для Windows
mingw32-make -f Makefile.win
```

## Запуск

### Linux/macOS

```bash
make run
```

или

```bash
./mle_estimator
```

### Windows

```cmd
mle_estimator.exe
```

### GUI приложения

#### macOS (Qt)

```bash
# Переключиться на ветку macOS GUI
git checkout feature/macos-gui-support

# Собрать и запустить
make gui
```

См. [gui/README.md](gui/README.md) для подробной информации.

#### Windows (Windows Forms)

```cmd
# Переключиться на ветку Windows Forms
git checkout feature/windows-forms

# Открыть в Visual Studio
winforms\StatisticalAnalysisWinForms.sln
```

См. [winforms/README.md](winforms/README.md) для подробной информации.

## Структура проекта

```
agamirovstats/
├── include/                    # Заголовочные файлы
│   ├── boost_distributions.h   # Обертки для Boost распределений
│   ├── confidence_intervals.h  # Доверительные интервалы
│   ├── mle_methods.h           # MLE/MLS методы
│   ├── matrix_operations.h     # Операции с матрицами
│   └── nelder_mead.h           # Метод Нелдера-Мида
├── src/                        # Исходные файлы
│   ├── boost_distributions.cpp
│   ├── confidence_intervals.cpp
│   ├── mle_methods.cpp
│   ├── matrix_operations.cpp
│   └── nelder_mead.cpp
├── python/                     # Python скрипты для визуализации
│   ├── plot_normal.py
│   ├── plot_weibull.py
│   └── plot_t_distribution.py
├── input/                      # Входные данные
│   ├── data_normal.txt
│   ├── data_weibull.txt
│   ├── data_censored_normal.txt
│   └── data_censored_weibull.txt
├── output/                     # Результаты (создается автоматически)
│   ├── *.txt                   # Числовые результаты
│   └── *.png                   # Графики
├── test/                       # Тесты
├── main.cpp                    # Главный файл программы
├── Makefile                    # Makefile для Unix
├── Makefile.win                # Makefile для Windows
└── README.md                   # Этот файл
```

## Результаты

После запуска программа создает следующие файлы в директории `output/`:

### Числовые результаты

- `mle_normal_complete.txt` - MLE для нормального распределения
- `mle_weibull_complete.txt` - MLE для Вейбулла
- `mls_normal_censored.txt` - MLS для нормального (цензурированные)
- `mls_weibull_censored.txt` - MLS для Вейбулла (цензурированные)
- `confidence_intervals.txt` - Доверительные интервалы
- `percentiles_normal.txt` - Персентили для нормального
- `percentiles_weibull.txt` - Персентили для Вейбулла

### Графики

- `plot_mle_normal.png` - График MLE для нормального
- `plot_mls_normal.png` - График MLS для нормального
- `plot_mle_weibull.png` - График MLE для Вейбулла
- `plot_mls_weibull.png` - График MLS для Вейбулла
- `plot_t_varying_df.png` - t-распределение (неизвестная σ)
- `plot_normal_varying_sigma.png` - Нормальное (известная σ)
- `plot_chi_squared.png` - χ² распределение (неизвестное μ)

## Формат входных данных

### Полные данные

Текстовый файл с одним значением на строку:

```
87.3
92.1
98.5
...
```

### Цензурированные данные

Текстовый файл с двумя колонками (значение и индикатор цензуры):

```
102.5 0
95.3 0
110.0 1
...
```

Индикатор: `0` = полное наблюдение, `1` = цензурированное (правая цензура)

## Используемые методы

### MLE (Maximum Likelihood Estimation)

- **Нормальное**: μ̂ = x̄, σ̂² = (1/n)Σ(xᵢ - x̄)²
- **Вейбулл**: λ = (1/n × Σxᵢᵏ)^(1/k), k решается численно

### Доверительные интервалы

- **Известная σ**: μ ± z_{α/2} × σ/√n
- **Неизвестная σ**: μ ± t_{α/2}(n-1) × s/√n
- **Для дисперсии**: [(n-1)s²/χ²_{α/2}, (n-1)s²/χ²_{1-α/2}]

### Персентили (Квантили)

- **Нормальное**: x_p = μ + z_p × σ
- **Вейбулл**: x_p = λ × (-ln(1-p))^(1/k)

## Очистка

### Linux/macOS

```bash
make clean        # Удалить все скомпилированные файлы
make clean-obj    # Удалить только объектные файлы
```

### Windows

```cmd
mingw32-make -f Makefile.win clean
```

## Устранение неполадок

### Ошибка: "boost/math not found"

Убедитесь что Boost установлен и путь к нему указан правильно:

```bash
# Linux/macOS
export BOOST_ROOT=/path/to/boost

# Windows (в MSYS2)
export BOOST_ROOT=/mingw64
```

### Ошибка: "Python venv not found"

Создайте виртуальное окружение заново (см. раздел Установка).

### Графики не создаются

Проверьте что Python библиотеки установлены:

```bash
python/venv/bin/python -c "import numpy, matplotlib, scipy"
```

## Литература

Реализация основана на формулах из:
- Агамиров Л.В. "Статистические методы анализа результатов научных исследований"
- Формулы (2.79), (2.80), (2.83) для доверительных интервалов

## Авторы

- xloned (https://github.com/xloned)
- Assisted by Claude (Anthropic)

## Лицензия

MIT License
