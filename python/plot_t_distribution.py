#!/usr/bin/env python3
"""
Визуализация распределения Стьюдента для трех случаев:
1. Неизвестная σ - t-распределение с разными степенями свободы
2. Известная σ - нормальное распределение с разными σ
3. Неизвестное μ - χ² распределение для дисперсии
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import sys

def read_ci_results(filename):
    """Чтение результатов доверительных интервалов"""
    params = {}

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            parts = line.split()
            if len(parts) == 2:
                try:
                    params[parts[0]] = float(parts[1])
                except ValueError:
                    continue

    return params

def plot_t_varying_df(params, output_file):
    """
    График 1: Распределение Стьюдента при НЕИЗВЕСТНОЙ σ
    Показывает как t-распределение сходится к нормальному при увеличении df
    """
    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle('Распределение Стьюдента при неизвестной σ', fontsize=14, fontweight='bold')

    x = np.linspace(-4, 4, 1000)

    # Различные степени свободы
    df_values = [3, 5, 10, 30]
    colors = plt.cm.viridis(np.linspace(0, 0.8, len(df_values)))

    # t-распределения с разными df
    for i, df in enumerate(df_values):
        t_pdf = stats.t.pdf(x, df)
        ax.plot(x, t_pdf, color=colors[i], linewidth=2.5,
                label=f't(df={df})', alpha=0.9)

    # Нормальное распределение для сравнения
    normal_pdf = stats.norm.pdf(x, 0, 1)
    ax.plot(x, normal_pdf, 'k--', linewidth=2.5, label='N(0,1) (df=∞)', alpha=0.8)

    ax.set_xlabel('x', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.legend(fontsize=11, loc='upper right')
    ax.grid(True, alpha=0.3)

    # Аннотация
    ax.text(0.02, 0.98, 'При увеличении df графики\nдвигаются параллельно\nк N(0,1)',
            transform=ax.transAxes, fontsize=10,
            verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def plot_normal_varying_sigma(params, output_file):
    """
    График 2: Нормальное распределение при ИЗВЕСТНОЙ σ
    Показывает как меняется ширина распределения при изменении σ
    """
    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle('Нормальное распределение при известной σ', fontsize=14, fontweight='bold')

    mean = 0
    base_sigma = 1

    # Различные значения σ
    sigma_values = [0.5, 0.8, 1.0, 1.5, 2.0]
    colors = plt.cm.plasma(np.linspace(0, 0.9, len(sigma_values)))

    # Диапазон значений
    x = np.linspace(-6, 6, 1000)

    # Нормальные распределения с разными σ
    for i, sigma in enumerate(sigma_values):
        pdf = stats.norm.pdf(x, mean, sigma)
        ax.plot(x, pdf, color=colors[i], linewidth=2.5,
                label=f'σ = {sigma:.1f}', alpha=0.9)

    ax.set_xlabel('x', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.legend(fontsize=11, loc='upper right')
    ax.grid(True, alpha=0.3)

    # Аннотация
    ax.text(0.02, 0.98, 'При изменении σ графики\nменяют ширину (угол наклона)',
            transform=ax.transAxes, fontsize=10,
            verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def plot_chi_squared_for_variance(params, output_file):
    """
    График 3: χ² распределение при НЕИЗВЕСТНОМ μ
    Показывает распределение для оценки дисперсии
    """
    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle('Распределение χ² для дисперсии при неизвестном μ', fontsize=14, fontweight='bold')

    # Различные степени свободы
    df_values = [5, 10, 15, 20, 30]
    colors = plt.cm.coolwarm(np.linspace(0, 0.9, len(df_values)))

    # Диапазон значений
    x = np.linspace(0.01, 60, 1000)

    # χ² распределения с разными df
    for i, df in enumerate(df_values):
        chi_pdf = stats.chi2.pdf(x, df)
        ax.plot(x, chi_pdf, color=colors[i], linewidth=2.5,
                label=f'χ²(df={df})', alpha=0.9)

    ax.set_xlabel('x', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.legend(fontsize=11, loc='upper right')
    ax.grid(True, alpha=0.3)

    # Аннотация
    ax.text(0.98, 0.98, 'χ² распределение используется\nдля построения доверительного\nинтервала для σ²',
            transform=ax.transAxes, fontsize=10,
            verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def main():
    # Читаем результаты
    results_file = '../output/confidence_intervals.txt'
    params = read_ci_results(results_file)

    if not params:
        print(f"Ошибка: не удалось прочитать файл {results_file}")
        # Используем значения по умолчанию
        params = {'sample_size': 20, 'confidence': 0.95}

    print("\nСоздание визуализаций распределений...")

    # График 1: t-распределение при разных df (неизвестная σ)
    output1 = '../output/plot_t_varying_df.png'
    plot_t_varying_df(params, output1)

    # График 2: Нормальное распределение при разных σ (известная σ)
    output2 = '../output/plot_normal_varying_sigma.png'
    plot_normal_varying_sigma(params, output2)

    # График 3: χ² распределение (неизвестное μ)
    output3 = '../output/plot_chi_squared.png'
    plot_chi_squared_for_variance(params, output3)

    print("\nВсе графики успешно созданы!")
    print("  1. График t-распределения (неизвестная σ)")
    print("  2. График нормального распределения (известная σ)")
    print("  3. График χ² распределения (неизвестное μ)")

if __name__ == '__main__':
    main()
