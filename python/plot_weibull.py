#!/usr/bin/env python3
"""
Визуализация результатов MLE/MLS для распределения Вейбулла
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import scipy.special
import sys

def read_results(filename):
    """Чтение результатов из файла"""
    params = {}
    data = []
    censored = []
    
    with open(filename, 'r') as f:
        section = None
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                if '# Данные' in line:
                    section = 'data'
                continue
            
            if section == 'data':
                parts = line.split()
                if len(parts) == 2:
                    data.append(float(parts[0]))
                    censored.append(int(parts[1]))
            else:
                parts = line.split()
                if len(parts) == 2:
                    params[parts[0]] = float(parts[1])
    
    return params, np.array(data), np.array(censored)

def plot_mle_weibull(params, data, censored, output_file):
    """Построение графиков для MLE распределения Вейбулла"""
    scale = params['parameter_1']  # λ (lambda)
    shape = params['parameter_2']  # k (форма)

    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle('MLE для распределения Вейбулла', fontsize=14, fontweight='bold')

    complete_data = data[censored == 0]

    # Гистограмма данных
    ax.hist(complete_data, bins=20, density=True, alpha=0.6,
            color='skyblue', edgecolor='black', label='Данные')

    # Точки данных на оси x
    ax.scatter(complete_data, np.zeros_like(complete_data),
              color='blue', alpha=0.5, s=50, marker='|', linewidths=2,
              label='Наблюдения')

    # Теоретическое распределение
    x = np.linspace(0.01, data.max() * 1.5, 1000)
    pdf = stats.weibull_min.pdf(x, shape, scale=scale)
    ax.plot(x, pdf, 'r-', linewidth=2.5, label=f'Weibull(λ={scale:.2f}, k={shape:.2f})')

    ax.set_xlabel('Значение', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.legend(fontsize=11, loc='best')
    ax.grid(True, alpha=0.3)

    # Информация о параметрах
    info_text = f'λ = {scale:.4f} ± {params.get("std_error_1", 0):.4f}\nk = {shape:.4f} ± {params.get("std_error_2", 0):.4f}\nn = {len(complete_data)}'
    ax.text(0.98, 0.98, info_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def plot_mls_weibull(params, data, censored, output_file):
    """Построение графиков для MLS распределения Вейбулла (с цензурой)"""
    scale = params['parameter_1']  # λ (lambda)
    shape = params['parameter_2']  # k (форма)

    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle('MLS для распределения Вейбулла (цензурированные данные)',
                 fontsize=14, fontweight='bold')

    complete_data = data[censored == 0]
    censored_data = data[censored == 1]

    # Гистограмма полных данных
    ax.hist(complete_data, bins=20, density=True, alpha=0.6,
            color='skyblue', edgecolor='black', label='Полные наблюдения')

    # Полные данные на оси x
    ax.scatter(complete_data, np.zeros_like(complete_data),
              color='blue', alpha=0.5, s=50, marker='|', linewidths=2)

    # Цензурированные данные на оси x
    if len(censored_data) > 0:
        ax.scatter(censored_data, np.zeros_like(censored_data),
                  color='red', marker='>', s=80, label='Цензурированные',
                  zorder=5, alpha=0.7)

    # Теоретическое распределение
    x = np.linspace(0.01, data.max() * 1.5, 1000)
    pdf = stats.weibull_min.pdf(x, shape, scale=scale)
    ax.plot(x, pdf, 'r-', linewidth=2.5, label=f'Weibull(λ={scale:.2f}, k={shape:.2f})')

    ax.set_xlabel('Значение', fontsize=12)
    ax.set_ylabel('Плотность вероятности', fontsize=12)
    ax.legend(fontsize=11, loc='best')
    ax.grid(True, alpha=0.3)

    # Информация о параметрах
    n_total = len(data)
    n_censored = len(censored_data)
    info_text = f'λ = {scale:.4f} ± {params.get("std_error_1", 0):.4f}\nk = {shape:.4f} ± {params.get("std_error_2", 0):.4f}\nn = {n_total} (цензурировано: {n_censored})'
    ax.text(0.98, 0.98, info_text, transform=ax.transAxes,
            fontsize=10, verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))

    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def main():
    if len(sys.argv) < 2:
        print("Использование: python plot_weibull.py [mle|mls]")
        sys.exit(1)
    
    mode = sys.argv[1].lower()
    
    if mode == 'mle':
        results_file = '../output/mle_weibull_complete.txt'
        output_file = '../output/plot_mle_weibull.png'
        params, data, censored = read_results(results_file)
        plot_mle_weibull(params, data, censored, output_file)
    elif mode == 'mls':
        results_file = '../output/mls_weibull_censored.txt'
        output_file = '../output/plot_mls_weibull.png'
        params, data, censored = read_results(results_file)
        plot_mls_weibull(params, data, censored, output_file)
    else:
        print("Ошибка: укажите 'mle' или 'mls'")
        sys.exit(1)

if __name__ == '__main__':
    main()