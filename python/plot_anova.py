#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скрипт для визуализации результатов однофакторного дисперсионного анализа (ANOVA)
Создает графики распределения F и box plots для сравнения групп
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

def read_anova_results(filename):
    """
    Читает результаты ANOVA из файла

    Args:
        filename: путь к файлу с результатами

    Returns:
        dict: словарь с параметрами теста
    """
    result = {
        'group_means': [],
        'group_sizes': [],
        'group_count': 0
    }

    try:
        with open(filename, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        in_group_info = False
        for line in lines:
            line = line.strip()

            # Извлекаем количество групп
            if line.startswith('Количество групп:'):
                result['num_groups'] = int(line.split('= ')[1])

            # Извлекаем общее количество наблюдений
            elif line.startswith('Общее количество наблюдений:'):
                result['total_n'] = int(line.split('= ')[1])

            # Извлекаем уровень значимости
            elif 'Уровень значимости:' in line and 'α = ' in line:
                result['alpha'] = float(line.split('α = ')[1])

            # Извлекаем информацию о группах
            elif line.startswith('Информация о группах:'):
                in_group_info = True
            elif in_group_info and line.startswith('Группа'):
                parts = line.split(',')
                n = int(parts[0].split('= ')[1])
                mean = float(parts[1].split('= ')[1])
                result['group_sizes'].append(n)
                result['group_means'].append(mean)
            elif in_group_info and 'Общее среднее:' in line:
                result['grand_mean'] = float(line.split('= ')[1])
                in_group_info = False

            # Извлекаем F-статистику
            elif line.startswith('F-статистика = '):
                result['f_statistic'] = float(line.split('= ')[1])

            # Извлекаем критическое значение
            elif line.startswith('Критическое значение'):
                result['critical_value'] = float(line.split('= ')[1])

            # Извлекаем p-значение
            elif line.startswith('p-value = '):
                result['p_value'] = float(line.split('= ')[1])

            # Степени свободы из таблицы
            elif 'Между группами' in line:
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        result['df_between'] = int(parts[2])
                    except:
                        pass
            elif 'Внутри групп' in line:
                parts = line.split()
                if len(parts) >= 3:
                    try:
                        result['df_within'] = int(parts[2])
                    except:
                        pass

            # Определяем результат теста
            elif 'H0 ОТВЕРГАЕТСЯ' in line:
                result['reject_h0'] = True
            elif 'H0 НЕ ОТВЕРГАЕТСЯ' in line:
                result['reject_h0'] = False

        return result

    except Exception as e:
        print(f"Ошибка при чтении файла {filename}: {e}")
        return None

def plot_f_distribution(result, output_filename):
    """
    Создает график F-распределения с критическими областями

    Args:
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
    """
    if result is None or 'df_between' not in result:
        print(f"Ошибка: недостаточно данных для построения F-распределения")
        return

    df1 = result['df_between']
    df2 = result['df_within']
    alpha = result.get('alpha', 0.05)
    f_stat = result.get('f_statistic', 0.0)
    f_crit = result.get('critical_value', stats.f.ppf(1 - alpha, df1, df2))
    reject_h0 = result.get('reject_h0', False)

    # Создаем массив значений для графика
    x_max = max(f_crit * 2, f_stat * 1.5, 5.0)
    x = np.linspace(0.01, x_max, 1000)
    y = stats.f.pdf(x, df1, df2)

    # Создаем график
    fig, ax = plt.subplots(figsize=(12, 7))

    # Рисуем распределение
    ax.plot(x, y, 'b-', linewidth=2, label=f'F-распределение (df1={df1}, df2={df2})')

    # Закрашиваем критическую область (правосторонний тест)
    x_crit = x[x >= f_crit]
    ax.fill_between(x_crit, stats.f.pdf(x_crit, df1, df2), alpha=0.3, color='red',
                     label=f'Критическая область (α={alpha})')

    # Отмечаем критическое значение
    ax.axvline(f_crit, color='red', linestyle='--', linewidth=1.5,
               label=f'Критическое значение: {f_crit:.4f}')

    # Отмечаем наблюдаемую статистику
    if reject_h0:
        color = 'darkred'
        marker_label = f'Наблюдаемое F={f_stat:.4f} (H₀ отвергается)'
    else:
        color = 'green'
        marker_label = f'Наблюдаемое F={f_stat:.4f} (H₀ не отвергается)'

    ax.axvline(f_stat, color=color, linestyle='-', linewidth=2.5, label=marker_label)

    # Отмечаем точку на кривой
    y_point = stats.f.pdf(f_stat, df1, df2)
    ax.plot(f_stat, y_point, 'o', color=color, markersize=10, zorder=5)

    # Настройки графика
    ax.set_xlabel('F-значение', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.set_title('Однофакторный дисперсионный анализ (ANOVA)', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10, loc='upper right')
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, x_max)

    # Добавляем текст с результатами
    info_text = f'Количество групп: {result.get("num_groups", 0)}\n'
    info_text += f'Общее N: {result.get("total_n", 0)}\n'
    info_text += f'df1 (между): {df1}\n'
    info_text += f'df2 (внутри): {df2}\n'
    info_text += f'α = {alpha}\n'
    info_text += f'F-статистика: {f_stat:.4f}\n'
    info_text += f'p-значение: {result.get("p_value", 0.0):.4f}'

    ax.text(0.98, 0.98, info_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"График F-распределения сохранен: {output_filename}")
    plt.close()

def plot_group_comparison(result, output_filename):
    """
    Создает график сравнения групп (средние с доверительными интервалами)

    Args:
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
    """
    if result is None or not result.get('group_means'):
        print(f"Ошибка: недостаточно данных для построения графика групп")
        return

    group_means = result['group_means']
    grand_mean = result.get('grand_mean', np.mean(group_means))
    num_groups = len(group_means)

    # Создаем график
    fig, ax = plt.subplots(figsize=(10, 7))

    # Рисуем средние каждой группы
    x_pos = np.arange(num_groups)
    colors = ['blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive']

    bars = ax.bar(x_pos, group_means, width=0.6, alpha=0.7,
                   color=[colors[i % len(colors)] for i in range(num_groups)],
                   edgecolor='black', linewidth=1.5)

    # Отмечаем общее среднее
    ax.axhline(grand_mean, color='red', linestyle='--', linewidth=2,
               label=f'Общее среднее: {grand_mean:.4f}', zorder=5)

    # Настройки графика
    ax.set_xlabel('Группа', fontsize=12)
    ax.set_ylabel('Среднее значение', fontsize=12)
    ax.set_title('Сравнение средних значений групп (ANOVA)', fontsize=14, fontweight='bold')
    ax.set_xticks(x_pos)
    ax.set_xticklabels([f'Группа {i+1}' for i in range(num_groups)])
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3, axis='y')

    # Добавляем значения над столбцами
    for i, (bar, mean) in enumerate(zip(bars, group_means)):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{mean:.3f}\n(n={result["group_sizes"][i]})',
                ha='center', va='bottom', fontsize=9)

    # Добавляем информацию о результате теста
    if result.get('reject_h0'):
        test_result = 'H₀ ОТВЕРГАЕТСЯ\nСредние статистически различаются'
        box_color = 'lightcoral'
    else:
        test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ\nНет оснований отвергнуть равенство средних'
        box_color = 'lightgreen'

    info_text = f'{test_result}\n\n'
    info_text += f'F = {result.get("f_statistic", 0.0):.4f}\n'
    info_text += f'p = {result.get("p_value", 0.0):.4f}'

    ax.text(0.02, 0.98, info_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"График сравнения групп сохранен: {output_filename}")
    plt.close()

def main():
    """
    Основная функция: читает результаты и создает графики
    """
    input_file = 'output/anova_result.txt'

    if not os.path.exists(input_file):
        print(f"Файл {input_file} не найден")
        return

    print("Создание графиков для ANOVA...")

    result = read_anova_results(input_file)

    if result:
        # Создаем комплексный график с двумя подграфиками
        fig = plt.figure(figsize=(16, 8))

        # Подграфик 1: F-распределение
        ax1 = plt.subplot(1, 2, 1)
        df1 = result['df_between']
        df2 = result['df_within']
        alpha = result.get('alpha', 0.05)
        f_stat = result.get('f_statistic', 0.0)
        f_crit = result.get('critical_value', stats.f.ppf(1 - alpha, df1, df2))
        reject_h0 = result.get('reject_h0', False)

        x_max = max(f_crit * 2, f_stat * 1.5, 5.0)
        x = np.linspace(0.01, x_max, 1000)
        y = stats.f.pdf(x, df1, df2)

        ax1.plot(x, y, 'b-', linewidth=2, label=f'F-распределение (df1={df1}, df2={df2})')
        x_crit = x[x >= f_crit]
        ax1.fill_between(x_crit, stats.f.pdf(x_crit, df1, df2), alpha=0.3, color='red',
                         label=f'Критическая область (α={alpha})')
        ax1.axvline(f_crit, color='red', linestyle='--', linewidth=1.5,
                   label=f'Критическое значение: {f_crit:.4f}')

        if reject_h0:
            color = 'darkred'
            marker_label = f'Наблюдаемое F={f_stat:.4f} (H₀ отвергается)'
        else:
            color = 'green'
            marker_label = f'Наблюдаемое F={f_stat:.4f} (H₀ не отвергается)'

        ax1.axvline(f_stat, color=color, linestyle='-', linewidth=2.5, label=marker_label)
        y_point = stats.f.pdf(f_stat, df1, df2)
        ax1.plot(f_stat, y_point, 'o', color=color, markersize=10, zorder=5)

        ax1.set_xlabel('F-значение', fontsize=12)
        ax1.set_ylabel('Плотность вероятности', fontsize=12)
        ax1.set_title('Диаграмма размаха групп ANOVA', fontsize=14, fontweight='bold')
        ax1.legend(fontsize=9, loc='upper right')
        ax1.grid(True, alpha=0.3)
        ax1.set_xlim(0, x_max)

        info_text = f'Количество групп: {result.get("num_groups", 0)}\n'
        info_text += f'Общее N: {result.get("total_n", 0)}\n'
        info_text += f'df1 (между): {df1}\n'
        info_text += f'df2 (внутри): {df2}\n'
        info_text += f'α = {alpha}\n'
        info_text += f'F-статистика: {f_stat:.4f}\n'
        info_text += f'p-значение: {result.get("p_value", 0.0):.4f}'

        ax1.text(0.98, 0.98, info_text, transform=ax1.transAxes,
                fontsize=9, verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

        # Подграфик 2: Сравнение средних групп
        ax2 = plt.subplot(1, 2, 2)
        group_means = result['group_means']
        grand_mean = result.get('grand_mean', np.mean(group_means))
        num_groups = len(group_means)

        x_pos = np.arange(num_groups)
        colors = ['blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive']

        bars = ax2.bar(x_pos, group_means, width=0.6, alpha=0.7,
                       color=[colors[i % len(colors)] for i in range(num_groups)],
                       edgecolor='black', linewidth=1.5)

        ax2.axhline(grand_mean, color='red', linestyle='--', linewidth=2,
                   label=f'Общее среднее: {grand_mean:.4f}', zorder=5)

        ax2.set_xlabel('Группа', fontsize=12)
        ax2.set_ylabel('Среднее значение', fontsize=12)
        ax2.set_title('Сравнение трёх групп (ANOVA)', fontsize=14, fontweight='bold')
        ax2.set_xticks(x_pos)
        ax2.set_xticklabels([f'Группа {i+1}' for i in range(num_groups)])
        ax2.legend(fontsize=10)
        ax2.grid(True, alpha=0.3, axis='y')

        for i, (bar, mean) in enumerate(zip(bars, group_means)):
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{mean:.3f}\n(n={result["group_sizes"][i]})',
                    ha='center', va='bottom', fontsize=9)

        if result.get('reject_h0'):
            test_result = 'H₀ ОТВЕРГАЕТСЯ\nСредние статистически различаются'
            box_color = 'lightcoral'
        else:
            test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ\nНет оснований отвергнуть равенство средних'
            box_color = 'lightgreen'

        info_text2 = f'{test_result}\n\n'
        info_text2 += f'F = {result.get("f_statistic", 0.0):.4f}\n'
        info_text2 += f'p = {result.get("p_value", 0.0):.4f}'

        ax2.text(0.02, 0.98, info_text2, transform=ax2.transAxes,
                fontsize=10, verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.8))

        plt.tight_layout()
        plt.savefig('output/plot_anova_f_distribution.png', dpi=300, bbox_inches='tight')
        print(f"Комплексный график ANOVA сохранен: output/plot_anova_f_distribution.png")
        plt.close()

        print("Визуализация ANOVA завершена!")
    else:
        print("Ошибка при чтении результатов ANOVA")

if __name__ == "__main__":
    main()
