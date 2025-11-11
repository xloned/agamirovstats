#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скрипт для визуализации результатов t-критерия Стьюдента
Создает графики распределения с критическими областями и наблюдаемой статистикой
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import sys
import os

# Переходим в корневую директорию проекта (где находится output/)
script_dir = os.path.dirname(os.path.abspath(__file__))
project_dir = os.path.dirname(script_dir)
os.chdir(project_dir)

def read_student_test_results(filename):
    """
    Читает результаты t-критерия из файла

    Args:
        filename: путь к файлу с результатами

    Returns:
        dict: словарь с параметрами теста
    """
    result = {}

    try:
        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        for line in lines:
            line = line.strip()

            # Извлекаем степени свободы
            if 'Степени свободы:' in line and 'ν = ' in line:
                result['df'] = float(line.split('ν = ')[1].split()[0])

            # Извлекаем уровень значимости
            elif 'Уровень значимости:' in line and 'α = ' in line:
                result['alpha'] = float(line.split('α = ')[1].split()[0])

            # Извлекаем t-статистику
            elif line.startswith('t-статистика = '):
                result['t_statistic'] = float(line.split('= ')[1])

            # Извлекаем критическое значение
            elif 'Критическое значение' in line and '} = ' in line:
                result['critical_value'] = float(line.split('} = ')[1].split()[0])

            # Извлекаем p-значение
            elif line.startswith('P-значение = '):
                result['p_value'] = float(line.split('= ')[1])

            # Определяем результат теста
            elif 'H0 ОТВЕРГАЕТСЯ' in line:
                result['reject_h0'] = True
            elif 'H0 НЕ ОТВЕРГАЕТСЯ' in line:
                result['reject_h0'] = False

            # Извлекаем тип теста
            elif 'Метод:' in line:
                if 'равные дисперсии' in line:
                    result['test_type'] = 'equal_var'
                elif 'неравные дисперсии' in line:
                    result['test_type'] = 'unequal_var'

        return result

    except Exception as e:
        print(f"Ошибка при чтении файла {filename}: {e}")
        return None

def plot_student_test(result, output_filename, title):
    """
    Создает график распределения Стьюдента с критическими областями

    Args:
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
        title: заголовок графика
    """
    if result is None or 'df' not in result:
        print(f"Ошибка: недостаточно данных для построения графика {title}")
        return

    df = result['df']
    alpha = result.get('alpha', 0.05)
    t_stat = result.get('t_statistic', 0.0)
    t_crit = result.get('critical_value', stats.t.ppf(1 - alpha/2, df))
    reject_h0 = result.get('reject_h0', False)

    # Создаем массив значений для графика
    x = np.linspace(-4, 4, 1000)
    y = stats.t.pdf(x, df)

    # Создаем график
    fig, ax = plt.subplots(figsize=(12, 7))

    # Рисуем распределение
    ax.plot(x, y, 'b-', linewidth=2, label=f't-распределение (ν={df:.2f})')

    # Закрашиваем критические области (двусторонний тест)
    x_left = x[x <= -t_crit]
    x_right = x[x >= t_crit]
    ax.fill_between(x_left, stats.t.pdf(x_left, df), alpha=0.3, color='red',
                     label=f'Критическая область (α={alpha})')
    ax.fill_between(x_right, stats.t.pdf(x_right, df), alpha=0.3, color='red')

    # Отмечаем критические значения
    ax.axvline(-t_crit, color='red', linestyle='--', linewidth=1.5,
               label=f'Критические значения: ±{t_crit:.4f}')
    ax.axvline(t_crit, color='red', linestyle='--', linewidth=1.5)

    # Отмечаем наблюдаемую статистику
    if reject_h0:
        color = 'darkred'
        marker_label = f'Наблюдаемое t={t_stat:.4f} (H₀ отвергается)'
    else:
        color = 'green'
        marker_label = f'Наблюдаемое t={t_stat:.4f} (H₀ не отвергается)'

    ax.axvline(t_stat, color=color, linestyle='-', linewidth=2.5, label=marker_label)

    # Отмечаем точку на кривой
    y_point = stats.t.pdf(t_stat, df)
    ax.plot(t_stat, y_point, 'o', color=color, markersize=10, zorder=5)

    # Настройки графика
    ax.set_xlabel('t-значение', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.set_title(title, fontsize=14, fontweight='bold')
    ax.legend(fontsize=10, loc='upper right')
    ax.grid(True, alpha=0.3)

    # Добавляем текст с результатами
    info_text = f'Степени свободы: ν = {df:.2f}\n'
    info_text += f'Уровень значимости: α = {alpha}\n'
    info_text += f't-статистика: {t_stat:.4f}\n'
    info_text += f'p-значение: {result.get("p_value", 0.0):.4f}\n'
    info_text += f'Критическое значение: ±{t_crit:.4f}'

    ax.text(0.02, 0.98, info_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"График сохранен: {output_filename}")
    plt.close()

def main():
    """
    Основная функция: читает результаты и создает графики для всех вариантов t-теста
    """
    # Список файлов с результатами
    test_files = [
        ('output/student_test_equal_var.txt',
         'output/plot_student_equal_var.png',
         't-критерий Стьюдента (равные дисперсии)'),
        ('output/student_test_unequal_var.txt',
         'output/plot_student_unequal_var.png',
         't-критерий Стьюдента (неравные дисперсии, Уэлч)'),
        ('output/student_test_auto.txt',
         'output/plot_student_auto.png',
         't-критерий Стьюдента (автоматический выбор)')
    ]

    print("Создание графиков для t-критерия Стьюдента...")

    for input_file, output_file, title in test_files:
        if os.path.exists(input_file):
            result = read_student_test_results(input_file)
            if result:
                plot_student_test(result, output_file, title)
        else:
            print(f"Файл {input_file} не найден, пропускаем...")

    print("Визуализация t-критерия завершена!")

if __name__ == "__main__":
    main()
