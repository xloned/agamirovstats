#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скрипт для визуализации результатов критерия ранга суммы Уилкоксона
Создает графики распределения рангов, box plots и нормального приближения
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

def read_wilcoxon_results(filename):
    """
    Читает результаты критерия Уилкоксона из файла

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

            # Извлекаем размеры выборок
            if line.startswith('Размеры выборок:'):
                parts = line.split(',')
                result['n1'] = int(parts[0].split('= ')[1])
                result['n2'] = int(parts[1].split('= ')[1])

            # Извлекаем общее количество
            elif line.startswith('Общее количество наблюдений:'):
                result['total_n'] = int(line.split('= ')[1])

            # Извлекаем уровень значимости
            elif 'Уровень значимости:' in line and 'α = ' in line:
                result['alpha'] = float(line.split('α = ')[1].split()[0])

            # Извлекаем метод
            elif line.startswith('Метод:'):
                if 'нормальное приближение' in line:
                    result['use_normal_approx'] = True
                else:
                    result['use_normal_approx'] = False

            # Извлекаем статистики
            elif 'W (сумма рангов) = ' in line:
                result['w_statistic'] = float(line.split('= ')[1])
            elif 'U (Манна-Уитни) = ' in line:
                result['u_statistic'] = float(line.split('= ')[1])
            elif 'E[W] под H0 = ' in line:
                result['mean_w'] = float(line.split('= ')[1])
            elif 'SD[W] под H0 = ' in line:
                result['std_w'] = float(line.split('= ')[1])
            elif 'Z-статистика = ' in line:
                result['z_statistic'] = float(line.split('= ')[1])

            # Извлекаем критическое значение
            elif line.startswith('Критическое значение'):
                result['critical_value'] = float(line.split('= ')[1])

            # Извлекаем p-значение
            elif line.startswith('p-value'):
                result['p_value'] = float(line.split('= ')[1])

            # Извлекаем количество связей
            elif line.startswith('Обнаружено связанных групп:'):
                result['num_ties'] = int(line.split(':')[1].strip())

            # Определяем результат теста
            elif 'H0 ОТВЕРГАЕТСЯ' in line and 'различаются' in line:
                result['reject_h0'] = True
            elif 'H0 НЕ ОТВЕРГАЕТСЯ' in line:
                result['reject_h0'] = False

        return result

    except Exception as e:
        print(f"Ошибка при чтении файла {filename}: {e}")
        return None

def plot_normal_approximation(result, output_filename):
    """
    Создает график нормального приближения для W-статистики

    Args:
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
    """
    if result is None or not result.get('use_normal_approx', False):
        print(f"Нормальное приближение не используется, пропускаем график")
        return

    mean_w = result.get('mean_w', 0.0)
    std_w = result.get('std_w', 1.0)
    w_stat = result.get('w_statistic', mean_w)
    z_stat = result.get('z_statistic', 0.0)
    z_crit = result.get('critical_value', 1.96)
    reject_h0 = result.get('reject_h0', False)

    # Создаем массив значений для W
    x_w = np.linspace(mean_w - 4*std_w, mean_w + 4*std_w, 1000)
    y_w = stats.norm.pdf(x_w, mean_w, std_w)

    # Создаем график для W-распределения
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 12))

    # График 1: Распределение W
    ax1.plot(x_w, y_w, 'b-', linewidth=2,
             label=f'Нормальное приближение\nμ={mean_w:.2f}, σ={std_w:.2f}')

    # Закрашиваем критические области (двусторонний тест)
    w_crit_left = mean_w - z_crit * std_w
    w_crit_right = mean_w + z_crit * std_w

    x_left = x_w[x_w <= w_crit_left]
    x_right = x_w[x_w >= w_crit_right]
    ax1.fill_between(x_left, stats.norm.pdf(x_left, mean_w, std_w),
                      alpha=0.3, color='red', label='Критические области')
    ax1.fill_between(x_right, stats.norm.pdf(x_right, mean_w, std_w),
                      alpha=0.3, color='red')

    # Отмечаем критические значения
    ax1.axvline(w_crit_left, color='red', linestyle='--', linewidth=1.5)
    ax1.axvline(w_crit_right, color='red', linestyle='--', linewidth=1.5)

    # Отмечаем наблюдаемое значение W
    if reject_h0:
        color = 'darkred'
        marker_label = f'Наблюдаемое W={w_stat:.2f} (H₀ отвергается)'
    else:
        color = 'green'
        marker_label = f'Наблюдаемое W={w_stat:.2f} (H₀ не отвергается)'

    ax1.axvline(w_stat, color=color, linestyle='-', linewidth=2.5, label=marker_label)
    y_point = stats.norm.pdf(w_stat, mean_w, std_w)
    ax1.plot(w_stat, y_point, 'o', color=color, markersize=10, zorder=5)

    ax1.set_xlabel('W-статистика (сумма рангов)', fontsize=12)
    ax1.set_ylabel('Плотность вероятности', fontsize=12)
    ax1.set_title('Распределение W-статистики Уилкоксона', fontsize=14, fontweight='bold')
    ax1.legend(fontsize=10, loc='upper right')
    ax1.grid(True, alpha=0.3)

    # График 2: Стандартизованное Z-распределение
    x_z = np.linspace(-4, 4, 1000)
    y_z = stats.norm.pdf(x_z, 0, 1)

    ax2.plot(x_z, y_z, 'b-', linewidth=2, label='Стандартное нормальное N(0,1)')

    # Закрашиваем критические области
    x_left_z = x_z[x_z <= -z_crit]
    x_right_z = x_z[x_z >= z_crit]
    ax2.fill_between(x_left_z, stats.norm.pdf(x_left_z, 0, 1),
                      alpha=0.3, color='red',
                      label=f'Критические области (α={result.get("alpha", 0.05)})')
    ax2.fill_between(x_right_z, stats.norm.pdf(x_right_z, 0, 1),
                      alpha=0.3, color='red')

    # Отмечаем критические значения
    ax2.axvline(-z_crit, color='red', linestyle='--', linewidth=1.5,
                label=f'Критические значения: ±{z_crit:.4f}')
    ax2.axvline(z_crit, color='red', linestyle='--', linewidth=1.5)

    # Отмечаем наблюдаемое Z
    if reject_h0:
        color = 'darkred'
        marker_label = f'Наблюдаемое Z={z_stat:.4f} (H₀ отвергается)'
    else:
        color = 'green'
        marker_label = f'Наблюдаемое Z={z_stat:.4f} (H₀ не отвергается)'

    ax2.axvline(z_stat, color=color, linestyle='-', linewidth=2.5, label=marker_label)
    y_point_z = stats.norm.pdf(z_stat, 0, 1)
    ax2.plot(z_stat, y_point_z, 'o', color=color, markersize=10, zorder=5)

    ax2.set_xlabel('Z-значение', fontsize=12)
    ax2.set_ylabel('Плотность вероятности', fontsize=12)
    ax2.set_title('Стандартизованная Z-статистика', fontsize=14, fontweight='bold')
    ax2.legend(fontsize=10, loc='upper right')
    ax2.grid(True, alpha=0.3)

    # Добавляем информацию
    info_text = f'n₁ = {result.get("n1", 0)}, n₂ = {result.get("n2", 0)}\n'
    info_text += f'W = {w_stat:.2f}\n'
    info_text += f'Z = {z_stat:.4f}\n'
    info_text += f'p-value = {result.get("p_value", 0.0):.4f}'

    ax2.text(0.02, 0.98, info_text, transform=ax2.transAxes,
             fontsize=10, verticalalignment='top',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"График нормального приближения сохранен: {output_filename}")
    plt.close()

def read_two_samples(file1, file2):
    """
    Читает данные двух выборок из файлов

    Args:
        file1, file2: пути к файлам с данными

    Returns:
        tuple: (data1, data2) массивы данных
    """
    try:
        data1 = []
        if os.path.exists(file1):
            with open(file1, 'r') as f:
                for line in f:
                    line = line.strip()
                    if line and not line.startswith('#'):
                        parts = line.split()
                        if parts:
                            try:
                                data1.append(float(parts[0]))
                            except ValueError:
                                continue

        data2 = []
        if os.path.exists(file2):
            with open(file2, 'r') as f:
                for line in f:
                    line = line.strip()
                    if line and not line.startswith('#'):
                        parts = line.split()
                        if parts:
                            try:
                                data2.append(float(parts[0]))
                            except ValueError:
                                continue

        return np.array(data1) if data1 else None, np.array(data2) if data2 else None
    except Exception as e:
        print(f"Ошибка при чтении данных: {e}")
        return None, None

def plot_box_comparison(data1, data2, result, output_filename):
    """
    Создает box plot для сравнения двух выборок

    Args:
        data1, data2: массивы данных
        result: словарь с результатами теста
        output_filename: имя выходного файла
    """
    if data1 is None or data2 is None:
        print("Данные не найдены для box plot")
        return

    # Создаем график
    fig, ax = plt.subplots(figsize=(10, 8))

    # Box plots
    box_data = [data1, data2]
    bp = ax.boxplot(box_data, labels=['Выборка 1', 'Выборка 2'],
                     patch_artist=True, widths=0.6)

    # Раскрашиваем box plots
    colors = ['lightblue', 'lightcoral']
    for patch, color in zip(bp['boxes'], colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.7)

    # Настройки графика
    ax.set_ylabel('Значение', fontsize=12)
    ax.set_title('Сравнение двух выборок (критерий Уилкоксона)',
                 fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')

    # Добавляем информацию о результате
    if result:
        if result.get('reject_h0'):
            test_result = 'H₀ ОТВЕРГАЕТСЯ\nРаспределения различаются'
            box_color = 'lightcoral'
        else:
            test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ\nНет оснований отвергнуть равенство распределений'
            box_color = 'lightgreen'

        info_text = f'{test_result}\n\n'
        info_text += f'n₁ = {result.get("n1", len(data1))}\n'
        info_text += f'n₂ = {result.get("n2", len(data2))}\n'
        info_text += f'W = {result.get("w_statistic", 0.0):.2f}\n'
        info_text += f'U = {result.get("u_statistic", 0.0):.2f}\n'
        info_text += f'Z = {result.get("z_statistic", 0.0):.4f}\n'
        info_text += f'p = {result.get("p_value", 0.0):.4f}\n\n'
        info_text += f'Медиана 1: {np.median(data1):.3f}\n'
        info_text += f'Медиана 2: {np.median(data2):.3f}'

        ax.text(0.02, 0.98, info_text, transform=ax.transAxes,
                fontsize=10, verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.9))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"Box plot сохранен: {output_filename}")
    plt.close()

def main():
    """
    Основная функция: читает результаты и создает графики
    """
    result_file = 'output/wilcoxon_ranksum_result.txt'

    if not os.path.exists(result_file):
        print(f"Файл {result_file} не найден")
        return

    print("Создание графиков для критерия Уилкоксона...")

    result = read_wilcoxon_results(result_file)

    if result:
        # Создаем комплексный график с тремя подграфиками
        if not result.get('use_normal_approx', False):
            print("Нормальное приближение не используется, пропускаем визуализацию")
            return

        # Читаем реальные данные из файлов
        possible_files1 = ['input/sample1.txt', 'input/data1.txt']
        possible_files2 = ['input/sample2.txt', 'input/data2.txt']

        data1, data2 = None, None
        for f1, f2 in zip(possible_files1, possible_files2):
            if os.path.exists(f1) and os.path.exists(f2):
                data1, data2 = read_two_samples(f1, f2)
                if data1 is not None and data2 is not None:
                    print(f"Загружены данные из {f1} и {f2}")
                    break

        # Если данные не найдены, используем заглушку
        if data1 is None or data2 is None:
            print("Внимание: файлы данных не найдены, используются примерные значения")
            data1 = np.array([15, 15.5, 14.5, 16, 14, 15.2, 15.8, 14.8, 15.3, 15.7])
            data2 = np.array([11, 12, 11.5, 12.2, 11.8, 11.3, 11.9, 11.6, 12.1, 11.4])

        mean_w = result.get('mean_w', 0.0)
        std_w = result.get('std_w', 1.0)
        w_stat = result.get('w_statistic', mean_w)
        z_stat = result.get('z_statistic', 0.0)
        z_crit = result.get('critical_value', 1.96)
        reject_h0 = result.get('reject_h0', False)

        fig = plt.figure(figsize=(18, 6))

        # Подграфик 1: Сравнение средних значений (используем реальные данные)
        ax1 = plt.subplot(1, 3, 1)
        mean1 = np.mean(data1)
        mean2 = np.mean(data2)
        labels = ['Sample 1', 'Sample 2']
        colors_bars = ['#4472C4', '#ED7D31']

        bars = ax1.bar(labels, [mean1, mean2], color=colors_bars, alpha=0.7, edgecolor='black', linewidth=1.5)
        ax1.set_ylabel('Mean Value', fontsize=12)
        ax1.set_title('Сравнение двух выборок (Wilcoxon)', fontsize=14, fontweight='bold')
        ax1.grid(True, alpha=0.3, axis='y')
        ax1.legend(['Mean'], fontsize=10, loc='upper right')

        for bar in bars:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.2f}', ha='center', va='bottom', fontsize=10)

        # Подграфик 2: Диаграмма размаха (box plot) - используем реальные данные
        ax2 = plt.subplot(1, 3, 2)
        box_data = [data1, data2]
        bp = ax2.boxplot(box_data, labels=['Выборка 1', 'Выборка 2'],
                         patch_artist=True, widths=0.6)

        colors_box = ['lightblue', 'lightcoral']
        for patch, color in zip(bp['boxes'], colors_box):
            patch.set_facecolor(color)
            patch.set_alpha(0.7)

        ax2.set_ylabel('Значение', fontsize=12)
        ax2.set_title('Диаграмма размаха двух выборок', fontsize=14, fontweight='bold')
        ax2.grid(True, alpha=0.3, axis='y')

        if result.get('reject_h0'):
            test_result = 'H₀ ОТВЕРГАЕТСЯ\nРаспределения различаются'
            box_color = 'lightcoral'
        else:
            test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ\nНет оснований отвергнуть равенство распределений'
            box_color = 'lightgreen'

        info_text = f'{test_result}\n\n'
        info_text += f'n₁ = {result.get("n1", 0)}\n'
        info_text += f'n₂ = {result.get("n2", 0)}\n'
        info_text += f'W = {result.get("w_statistic", 0.0):.2f}\n'
        info_text += f'U = {result.get("u_statistic", 0.0):.2f}\n'
        info_text += f'Z = {z_stat:.4f}\n'
        info_text += f'p = {result.get("p_value", 0.0):.4f}'

        ax2.text(0.02, 0.98, info_text, transform=ax2.transAxes,
                fontsize=9, verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.9))

        # Подграфик 3: Рассеяние данных выборок - используем реальные данные
        ax3 = plt.subplot(1, 3, 3)
        n1_points = len(data1)
        n2_points = len(data2)
        sample1_x = np.arange(n1_points)
        sample1_y = data1
        sample2_x = np.arange(n2_points)
        sample2_y = data2

        ax3.scatter(sample1_x, sample1_y, c='red', s=80, alpha=0.6, label='Выборка 1', edgecolors='black')
        ax3.scatter(sample2_x + n1_points + 2, sample2_y, c='blue', s=80, alpha=0.6, label='Выборка 2', edgecolors='black')

        ax3.set_xlabel('Индекс наблюдения', fontsize=12)
        ax3.set_ylabel('Значение', fontsize=12)
        ax3.set_title('Рассеяние данных выборок', fontsize=14, fontweight='bold')
        ax3.legend(fontsize=10, loc='upper right')
        ax3.grid(True, alpha=0.3)

        plt.tight_layout()
        plt.savefig('output/plot_wilcoxon_normal_approx.png', dpi=300, bbox_inches='tight')
        print(f"Комплексный график Wilcoxon сохранен: output/plot_wilcoxon_normal_approx.png")
        plt.close()

        print("Визуализация критерия Уилкоксона завершена!")
    else:
        print("Ошибка при чтении результатов критерия Уилкоксона")

if __name__ == "__main__":
    main()
