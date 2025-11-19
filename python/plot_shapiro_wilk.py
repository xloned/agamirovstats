#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Скрипт для визуализации результатов критерия Шапиро-Уилка
Создает Q-Q plot и гистограмму с наложением нормального распределения
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

def read_shapiro_wilk_results(filename):
    """
    Читает результаты теста Шапиро-Уилка из файла

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

            # Извлекаем размер выборки
            if line.startswith('Размер выборки:'):
                result['n'] = int(line.split('= ')[1])

            # Извлекаем уровень значимости
            elif 'Уровень значимости:' in line and 'α = ' in line:
                result['alpha'] = float(line.split('α = ')[1])

            # Извлекаем W-статистику
            elif line.startswith('W-статистика = '):
                result['w_statistic'] = float(line.split('= ')[1])

            # Извлекаем критическое значение
            elif line.startswith('Критическое значение'):
                result['critical_value'] = float(line.split('= ')[1])

            # Извлекаем p-значение
            elif line.startswith('Приблизительное p-value = '):
                result['p_value'] = float(line.split('= ')[1])

            # Определяем результат теста
            elif 'H0 ОТВЕРГАЕТСЯ' in line and 'не является нормальной' in line:
                result['reject_h0'] = True
            elif 'H0 НЕ ОТВЕРГАЕТСЯ' in line:
                result['reject_h0'] = False

        return result

    except Exception as e:
        print(f"Ошибка при чтении файла {filename}: {e}")
        return None

def read_data_from_input(filename):
    """
    Читает данные из входного файла для построения Q-Q plot

    Args:
        filename: путь к файлу с данными

    Returns:
        numpy array: массив данных
    """
    try:
        # Пробуем разные форматы входных файлов
        data = []
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):
                    # Пробуем разделить по пробелам/табам
                    parts = line.split()
                    if parts:
                        try:
                            # Берем первое значение (на случай если файл с флагами цензуры)
                            data.append(float(parts[0]))
                        except ValueError:
                            continue
        return np.array(data)
    except Exception as e:
        print(f"Ошибка при чтении данных из {filename}: {e}")
        return None

def plot_qq_plot(data, result, output_filename):
    """
    Создает Q-Q plot для проверки нормальности

    Args:
        data: массив данных
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
    """
    if data is None or len(data) < 3:
        print(f"Ошибка: недостаточно данных для Q-Q plot")
        return

    # Создаем график
    fig, ax = plt.subplots(figsize=(10, 10))

    # Q-Q plot
    stats.probplot(data, dist="norm", plot=ax)

    # Настройки графика
    ax.set_title('Q-Q Plot (Проверка нормальности)', fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)

    # Добавляем информацию о результате теста
    if result:
        w_stat = result.get('w_statistic', 0.0)
        p_value = result.get('p_value', 0.0)
        reject_h0 = result.get('reject_h0', False)

        if reject_h0:
            test_result = 'H₀ ОТВЕРГАЕТСЯ\nДанные НЕ нормальны'
            box_color = 'lightcoral'
        else:
            test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ\nНет оснований отвергнуть нормальность'
            box_color = 'lightgreen'

        info_text = f'Критерий Шапиро-Уилка\n\n{test_result}\n\n'
        info_text += f'W = {w_stat:.6f}\n'
        info_text += f'W_crit = {result.get("critical_value", 0.0):.6f}\n'
        info_text += f'p-value = {p_value:.6f}\n'
        info_text += f'n = {result.get("n", len(data))}'

        ax.text(0.02, 0.98, info_text, transform=ax.transAxes,
                fontsize=11, verticalalignment='top',
                bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.9))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"Q-Q plot сохранен: {output_filename}")
    plt.close()

def plot_histogram_with_normal(data, result, output_filename):
    """
    Создает гистограмму данных с наложением нормального распределения

    Args:
        data: массив данных
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
    """
    if data is None or len(data) < 3:
        print(f"Ошибка: недостаточно данных для гистограммы")
        return

    # Вычисляем параметры выборки
    mean = np.mean(data)
    std = np.std(data, ddof=1)

    # Создаем график
    fig, ax = plt.subplots(figsize=(12, 7))

    # Гистограмма
    n, bins, patches = ax.hist(data, bins='auto', density=True, alpha=0.7,
                                 color='skyblue', edgecolor='black', label='Данные')

    # Наложение нормального распределения
    x = np.linspace(data.min(), data.max(), 1000)
    y = stats.norm.pdf(x, mean, std)
    ax.plot(x, y, 'r-', linewidth=2.5, label='Нормальное распределение\n' +
            f'μ={mean:.3f}, σ={std:.3f}')

    # Настройки графика
    ax.set_xlabel('Значение', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.set_title('Гистограмма с наложением нормального распределения',
                 fontsize=14, fontweight='bold')
    ax.legend(fontsize=10, loc='upper right')
    ax.grid(True, alpha=0.3)

    # Добавляем информацию о результате теста
    if result:
        w_stat = result.get('w_statistic', 0.0)
        p_value = result.get('p_value', 0.0)
        reject_h0 = result.get('reject_h0', False)

        if reject_h0:
            test_result = 'H₀ ОТВЕРГАЕТСЯ (не нормально)'
            box_color = 'lightcoral'
        else:
            test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ (нормально)'
            box_color = 'lightgreen'

        info_text = f'Критерий Шапиро-Уилка\n\n'
        info_text += f'W = {w_stat:.6f}\n'
        info_text += f'p = {p_value:.6f}\n'
        info_text += f'{test_result}\n\n'
        info_text += f'n = {result.get("n", len(data))}\n'
        info_text += f'μ̂ = {mean:.4f}\n'
        info_text += f'σ̂ = {std:.4f}'

        ax.text(0.98, 0.98, info_text, transform=ax.transAxes,
                fontsize=10, verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.9))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"Гистограмма сохранена: {output_filename}")
    plt.close()

def plot_w_statistic_interpretation(result, output_filename):
    """
    Создает график интерпретации W-статистики

    Args:
        result: словарь с результатами теста
        output_filename: имя выходного файла для графика
    """
    if result is None:
        return

    w_stat = result.get('w_statistic', 0.0)
    w_crit = result.get('critical_value', 0.0)
    reject_h0 = result.get('reject_h0', False)

    # Создаем график
    fig, ax = plt.subplots(figsize=(12, 6))

    # Шкала W от 0 до 1
    w_range = np.linspace(0, 1, 1000)

    # Рисуем шкалу
    ax.barh(0, 1, height=0.3, left=0, color='lightgray', edgecolor='black', linewidth=2)

    # Закрашиваем область принятия H0
    ax.barh(0, 1-w_crit, height=0.3, left=w_crit, color='lightgreen', alpha=0.7,
            label='Область принятия H₀ (нормальность)')

    # Закрашиваем критическую область
    ax.barh(0, w_crit, height=0.3, left=0, color='lightcoral', alpha=0.7,
            label='Критическая область (отклонение от нормальности)')

    # Отмечаем критическое значение
    ax.axvline(w_crit, color='red', linestyle='--', linewidth=2,
               label=f'W_критическое = {w_crit:.4f}')

    # Отмечаем наблюдаемое значение
    if reject_h0:
        color = 'darkred'
        marker_label = f'W_наблюдаемое = {w_stat:.4f} (H₀ отвергается)'
    else:
        color = 'darkgreen'
        marker_label = f'W_наблюдаемое = {w_stat:.4f} (H₀ не отвергается)'

    ax.axvline(w_stat, color=color, linestyle='-', linewidth=3,
               label=marker_label)
    ax.plot(w_stat, 0, 'o', color=color, markersize=15, zorder=5)

    # Настройки графика
    ax.set_xlim(0, 1)
    ax.set_ylim(-0.5, 0.5)
    ax.set_xlabel('W-статистика', fontsize=12)
    ax.set_yticks([])
    ax.set_title('Интерпретация W-статистики Шапиро-Уилка\n' +
                 '(значения ближе к 1 указывают на нормальность)',
                 fontsize=14, fontweight='bold')
    ax.legend(fontsize=10, loc='upper center', bbox_to_anchor=(0.5, -0.1), ncol=2)
    ax.grid(True, alpha=0.3, axis='x')

    # Добавляем зоны интерпретации
    ax.text(0.5, 0.35, 'Отклонение от нормальности', ha='center', fontsize=10,
            bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.5))
    ax.text(0.95, 0.35, 'Нормальность', ha='center', fontsize=10,
            bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.5))

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300, bbox_inches='tight')
    print(f"График интерпретации W сохранен: {output_filename}")
    plt.close()

def main():
    """
    Основная функция: читает результаты и создает графики
    """
    result_file = 'output/shapiro_wilk_result.txt'

    # Пробуем найти входной файл с данными
    possible_input_files = [
        'input/data_normal.txt',
        'input/data.txt'
    ]

    data = None
    for input_file in possible_input_files:
        if os.path.exists(input_file):
            data = read_data_from_input(input_file)
            if data is not None and len(data) > 0:
                print(f"Данные загружены из {input_file}")
                break

    if not os.path.exists(result_file):
        print(f"Файл {result_file} не найден")
        return

    print("Создание графиков для критерия Шапиро-Уилка...")

    result = read_shapiro_wilk_results(result_file)

    if result:
        # Создаем комплексный график с двумя подграфиками
        if data is not None and len(data) >= 3:
            fig = plt.figure(figsize=(16, 8))

            # Подграфик 1: Гистограмма с нормальным распределением
            ax1 = plt.subplot(1, 2, 1)
            mean = np.mean(data)
            std = np.std(data, ddof=1)

            n, bins, patches = ax1.hist(data, bins='auto', density=True, alpha=0.7,
                                         color='skyblue', edgecolor='black', label='Данные')

            x = np.linspace(data.min(), data.max(), 1000)
            y = stats.norm.pdf(x, mean, std)
            ax1.plot(x, y, 'r-', linewidth=2.5, label='Нормальное распределение\n' +
                    f'μ={mean:.3f}, σ={std:.3f}')

            ax1.set_xlabel('Значение', fontsize=12)
            ax1.set_ylabel('Плотность вероятности', fontsize=12)
            ax1.set_title('Гистограмма распределения данных', fontsize=14, fontweight='bold')
            ax1.legend(fontsize=10, loc='upper right')
            ax1.grid(True, alpha=0.3)

            w_stat = result.get('w_statistic', 0.0)
            p_value = result.get('p_value', 0.0)
            reject_h0 = result.get('reject_h0', False)

            if reject_h0:
                test_result = 'H₀ ОТВЕРГАЕТСЯ (не нормально)'
                box_color = 'lightcoral'
            else:
                test_result = 'H₀ НЕ ОТВЕРГАЕТСЯ (нормально)'
                box_color = 'lightgreen'

            info_text = f'Критерий Шапиро-Уилка\n\n'
            info_text += f'W = {w_stat:.6f}\n'
            info_text += f'p = {p_value:.6f}\n'
            info_text += f'{test_result}\n\n'
            info_text += f'n = {result.get("n", len(data))}\n'
            info_text += f'μ̂ = {mean:.4f}\n'
            info_text += f'σ̂ = {std:.4f}'

            ax1.text(0.98, 0.98, info_text, transform=ax1.transAxes,
                    fontsize=10, verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor=box_color, alpha=0.9))

            # Подграфик 2: Q-Q plot
            ax2 = plt.subplot(1, 2, 2)
            stats.probplot(data, dist="norm", plot=ax2)

            ax2.set_title('Q-Q Plot (Проверка нормальности)', fontsize=14, fontweight='bold')
            ax2.grid(True, alpha=0.3)

            if reject_h0:
                test_result2 = 'H₀ ОТВЕРГАЕТСЯ\nДанные НЕ нормальны'
                box_color2 = 'lightcoral'
            else:
                test_result2 = 'H₀ НЕ ОТВЕРГАЕТСЯ\nНет оснований отвергнуть нормальность'
                box_color2 = 'lightgreen'

            info_text2 = f'Критерий Шапиро-Уилка\n\n{test_result2}\n\n'
            info_text2 += f'W = {w_stat:.6f}\n'
            info_text2 += f'W_crit = {result.get("critical_value", 0.0):.6f}\n'
            info_text2 += f'p-value = {p_value:.6f}\n'
            info_text2 += f'n = {result.get("n", len(data))}'

            ax2.text(0.02, 0.98, info_text2, transform=ax2.transAxes,
                    fontsize=11, verticalalignment='top',
                    bbox=dict(boxstyle='round', facecolor=box_color2, alpha=0.9))

            plt.tight_layout()
            plt.savefig('output/plot_shapiro_wilk_qq.png', dpi=300, bbox_inches='tight')
            print(f"Комплексный график Shapiro-Wilk сохранен: output/plot_shapiro_wilk_qq.png")
            plt.close()

            print("Визуализация критерия Шапиро-Уилка завершена!")
        else:
            print("Данные не найдены или недостаточно данных для построения графиков")
    else:
        print("Ошибка при чтении результатов теста Шапиро-Уилка")

if __name__ == "__main__":
    main()
