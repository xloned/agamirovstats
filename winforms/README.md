# Statistical Analysis - Windows Forms GUI

Графическое приложение на Windows Forms (C++/CLI) для статистического анализа данных методами MLE и MLS.

## Возможности

- **MLE анализ** - Метод максимального правдоподобия для нормального распределения и распределения Вейбулла
- **MLS анализ** - Метод наименьших квадратов для нормального распределения (метод Дэйвида)
- **Работа с данными** - Загрузка файлов, просмотр статистики, поддержка цензурированных данных
- **Визуализация** - Просмотр и экспорт графиков результатов анализа
- **Интуитивный интерфейс** - Вкладки для разных типов анализа

## Требования

### Системные требования

- **Windows 10/11** (64-bit)
- **Visual Studio 2022** (или Visual Studio 2019 с C++/CLI support)
- **.NET Framework 4.7.2** или выше
- **Boost Library** версия 1.70+
- **Python 3.7+** с библиотеками: numpy, matplotlib, scipy

### Установка зависимостей

#### 1. Visual Studio

Установите Visual Studio 2022 Community Edition с компонентами:
- Разработка классических приложений на C++
- .NET desktop development
- C++/CLI support for v143 build tools

#### 2. Boost Library

**Вариант 1: С помощью vcpkg (рекомендуется)**
```cmd
vcpkg install boost:x64-windows
```

**Вариант 2: Вручную**
1. Скачайте Boost с https://www.boost.org/
2. Распакуйте в `C:\Boost` (или другую папку)
3. Установите переменную окружения `BOOST_ROOT=C:\Boost`

#### 3. Python и библиотеки

```cmd
python -m venv python\venv
python\venv\Scripts\activate
pip install numpy matplotlib scipy
deactivate
```

## Сборка

### С помощью Visual Studio

1. Откройте `StatisticalAnalysisWinForms.sln` в Visual Studio
2. Выберите конфигурацию **Release** и платформу **x64**
3. Меню: **Build → Build Solution** (или F7)
4. Исполняемый файл будет создан в `bin/Release/StatisticalAnalysisWinForms.exe`

### С помощью MSBuild (командная строка)

```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    StatisticalAnalysisWinForms.sln /p:Configuration=Release /p:Platform=x64
```

## Запуск

### Из Visual Studio

Нажмите **F5** (Debug) или **Ctrl+F5** (Release without debugging)

### Из командной строки

```cmd
cd bin\Release
StatisticalAnalysisWinForms.exe
```

## Структура проекта

```
winforms/
├── include/                     # Заголовочные файлы
│   └── MainForm.h              # Главная форма приложения
├── src/                        # Исходные файлы
│   ├── main.cpp                # Точка входа
│   └── MainForm.cpp            # Реализация главной формы
├── resources/                  # Ресурсы (иконки, изображения)
├── bin/                        # Скомпилированные исполняемые файлы
├── obj/                        # Промежуточные объектные файлы
├── StatisticalAnalysisWinForms.sln     # Visual Studio Solution
├── StatisticalAnalysisWinForms.vcxproj # Visual Studio Project
└── README.md                   # Этот файл
```

## Использование

### 1. Загрузка данных

- Перейдите на вкладку **"Данные"**
- Выберите файл из списка или нажмите **"Загрузить файл"**
- Просмотрите статистику загруженных данных

### 2. Выполнение MLE анализа

- Перейдите на вкладку **"MLE"**
- Выберите тип распределения (Нормальное или Вейбулла)
- Нажмите **"Выполнить анализ"**
- Результаты отобразятся в текстовом поле

### 3. Выполнение MLS анализа

- Перейдите на вкладку **"MLS"**
- Нажмите **"Выполнить анализ"** (доступно только для нормального распределения)
- Просмотрите результаты

### 4. Просмотр графиков

- Перейдите на вкладку **"Графики"**
- Выберите тип графика из выпадающего списка
- График отобразится автоматически
- Используйте кнопку **"Экспорт графика"** для сохранения

### 5. Сохранение результатов

- Меню: **Файл → Сохранить результаты...**
- Выберите место сохранения
- Сохранятся результаты с активной вкладки (MLE или MLS)

## Формат входных данных

### Полные данные (без цензурирования)

```
98.5
102.3
95.7
108.1
...
```

### Цензурированные данные

```
102.5 0
95.3 0
110.0 1
...
```

Где:
- Первая колонка - значение наблюдения
- Вторая колонка - флаг цензурирования (0 = полное, 1 = цензурировано)

## Устранение неполадок

### Ошибка: "Cannot find Boost headers"

Убедитесь что:
1. Boost установлен правильно
2. Переменная окружения `BOOST_ROOT` указывает на корневую папку Boost
3. В Visual Studio настроены правильные пути Include Directories

### Ошибка: "Python script failed"

Проверьте что:
1. Python виртуальное окружение создано в `python/venv`
2. Все необходимые библиотеки установлены: `pip list`
3. Путь к Python скриптам корректен

### Ошибка при сборке: "CLR support"

Убедитесь что в Visual Studio Installer установлен компонент "C++/CLI support"

## Отличия от Qt версии (macOS)

- **Платформа**: Windows вместо macOS
- **Framework**: Windows Forms вместо Qt
- **Язык UI**: C++/CLI вместо C++ + Qt MOC
- **Расчётные модули**: Те же самые (нативный C++)

## Авторы

- xloned (https://github.com/xloned)

## Лицензия

MIT License
