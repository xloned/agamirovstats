#!/usr/bin/env python3
"""
Визуализация персентилей с доверительными интервалами
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import sys

def read_percentiles(filename):
    """Чтение персентилей из файла (табличный формат)"""
    percentiles = {}
    dist_type = None

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            # Определяем тип распределения
            if line.startswith('distribution_type'):
                parts = line.split()
                if len(parts) >= 2:
                    dist_type = parts[1]
                    if dist_type not in percentiles:
                        percentiles[dist_type] = []
                continue

            # Пропускаем заголовки
            if dist_type is None:
                continue

            # Парсим табличные данные: p value lower upper width
            parts = line.split()
            if len(parts) >= 4:
                try:
                    prob = float(parts[0]) * 100  # конвертируем из 0.01 в 1.0%
                    value = float(parts[1])
                    lower = float(parts[2])
                    upper = float(parts[3])

                    percentiles[dist_type].append({
                        'prob': prob,
                        'value': value,
                        'lower': lower,
                        'upper': upper
                    })
                except:
                    pass  # Игнорируем строки которые не парсятся

    return percentiles

def plot_percentiles(percentiles, output_file):
    """Построение визуализации персентилей"""

    n_dists = len(percentiles)

    if n_dists == 0:
        print("Нет данных для построения графиков")
        return

    # Создаём фигуру
    fig = plt.figure(figsize=(14, 6 * n_dists), dpi=100)
    fig.suptitle('Персентили с 95% доверительными интервалами', fontsize=16, fontweight='bold')

    dist_titles = {
        'normal': 'Нормальное распределение',
        'weibull': 'Распределение Вейбулла'
    }

    colors = {
        'normal': '#3498db',
        'weibull': '#e74c3c'
    }

    plot_idx = 1
    for dist_key in ['normal', 'weibull']:
        if dist_key not in percentiles:
            continue

        data = percentiles[dist_key]
        if len(data) == 0:
            continue

        # Сортируем по вероятности
        data = sorted(data, key=lambda x: x['prob'])

        probs = [p['prob'] for p in data]
        values = [p['value'] for p in data]
        lowers = [p['lower'] for p in data]
        uppers = [p['upper'] for p in data]

        ax = plt.subplot(n_dists, 1, plot_idx)
        plot_idx += 1

        # Основная линия персентилей
        ax.plot(probs, values,
               'o-',
               color=colors[dist_key],
               linewidth=2.5,
               markersize=8,
               label='Персентиль',
               zorder=3)

        # Доверительные интервалы
        for i in range(len(probs)):
            ax.plot([probs[i], probs[i]],
                   [lowers[i], uppers[i]],
                   color=colors[dist_key],
                   alpha=0.5,
                   linewidth=6,
                   zorder=2)

        # Заливка области между границами
        ax.fill_between(probs, lowers, uppers,
                       color=colors[dist_key],
                       alpha=0.2,
                       label='95% ДИ')

        # Настройка осей
        ax.set_xlabel('Вероятность (%)', fontsize=12, fontweight='bold')
        ax.set_ylabel('Значение квантиля', fontsize=12, fontweight='bold')
        ax.set_title(dist_titles[dist_key], fontsize=13, fontweight='bold')
        ax.grid(True, alpha=0.3, linestyle='--')
        ax.legend(loc='upper left', fontsize=10)

        # Добавим аннотации для некоторых ключевых персентилей
        key_probs = [5.0, 50.0, 95.0]
        for prob in key_probs:
            for i, p in enumerate(data):
                if abs(p['prob'] - prob) < 0.1:
                    ax.annotate(f"{p['value']:.2f}",
                              xy=(p['prob'], p['value']),
                              xytext=(10, 10),
                              textcoords='offset points',
                              fontsize=9,
                              bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.7),
                              arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'))
                    break

    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def main():
    if len(sys.argv) < 3:
        print("Использование: python plot_percentiles.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    try:
        percentiles = read_percentiles(input_file)
        plot_percentiles(percentiles, output_file)
    except Exception as e:
        print(f"Ошибка: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
