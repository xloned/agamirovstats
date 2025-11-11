#!/usr/bin/env python3
"""
Визуализация доверительных интервалов для параметров распределений
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import sys

def read_confidence_intervals(filename):
    """Чтение доверительных интервалов из файла"""
    intervals = {}
    params = {}

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('==='):
                continue

            parts = line.split()
            if len(parts) < 2:
                continue

            key = parts[0]
            value = float(parts[1])
            params[key] = value

    # Формируем интервалы из прочитанных параметров
    # Интервал для μ при известной σ
    if 'ci_mean_known_sigma_lower' in params and 'ci_mean_known_sigma_upper' in params:
        if 'known_sigma' not in intervals:
            intervals['known_sigma'] = []
        intervals['known_sigma'].append({
            'name': 'μ (среднее)',
            'lower': params['ci_mean_known_sigma_lower'],
            'upper': params['ci_mean_known_sigma_upper'],
            'center': (params['ci_mean_known_sigma_lower'] + params['ci_mean_known_sigma_upper']) / 2,
            'width': params.get('ci_mean_known_sigma_width', 0)
        })

    # Интервал для μ при неизвестной σ
    if 'ci_mean_unknown_sigma_lower' in params and 'ci_mean_unknown_sigma_upper' in params:
        if 'unknown_sigma' not in intervals:
            intervals['unknown_sigma'] = []
        intervals['unknown_sigma'].append({
            'name': 'μ (среднее)',
            'lower': params['ci_mean_unknown_sigma_lower'],
            'upper': params['ci_mean_unknown_sigma_upper'],
            'center': (params['ci_mean_unknown_sigma_lower'] + params['ci_mean_unknown_sigma_upper']) / 2,
            'width': params.get('ci_mean_unknown_sigma_width', 0)
        })

    # Интервалы для σ и σ² при неизвестном μ
    if 'ci_variance_lower' in params and 'ci_variance_upper' in params:
        if 'unknown_mu' not in intervals:
            intervals['unknown_mu'] = []
        intervals['unknown_mu'].append({
            'name': 'σ² (дисперсия)',
            'lower': params['ci_variance_lower'],
            'upper': params['ci_variance_upper'],
            'center': params.get('ci_variance_point', (params['ci_variance_lower'] + params['ci_variance_upper']) / 2),
            'width': params['ci_variance_upper'] - params['ci_variance_lower']
        })

    if 'ci_sigma_lower' in params and 'ci_sigma_upper' in params:
        if 'unknown_mu' not in intervals:
            intervals['unknown_mu'] = []
        intervals['unknown_mu'].append({
            'name': 'σ (ст. откл.)',
            'lower': params['ci_sigma_lower'],
            'upper': params['ci_sigma_upper'],
            'center': params.get('ci_sigma_point', (params['ci_sigma_lower'] + params['ci_sigma_upper']) / 2),
            'width': params['ci_sigma_upper'] - params['ci_sigma_lower']
        })

    return intervals

def plot_confidence_intervals(intervals, output_file):
    """Построение визуализации доверительных интервалов"""

    # Подсчитываем количество подграфиков
    n_sections = len(intervals)

    if n_sections == 0:
        print("Нет данных для построения графиков")
        return

    # Создаём фигуру
    fig = plt.figure(figsize=(14, 5 * n_sections), dpi=100)
    fig.suptitle('Доверительные интервалы (95%)', fontsize=16, fontweight='bold')

    section_titles = {
        'known_sigma': 'При известной σ (нормальное распределение)',
        'unknown_sigma': 'При неизвестной σ (t-распределение)',
        'unknown_mu': 'При неизвестном μ (χ² для дисперсии)'
    }

    colors = {
        'known_sigma': '#2ecc71',
        'unknown_sigma': '#3498db',
        'unknown_mu': '#e74c3c'
    }

    plot_idx = 1
    for section_key in ['known_sigma', 'unknown_sigma', 'unknown_mu']:
        if section_key not in intervals:
            continue

        section_data = intervals[section_key]
        n_params = len(section_data)

        if n_params == 0:
            continue

        ax = plt.subplot(n_sections, 1, plot_idx)
        plot_idx += 1

        # Рисуем интервалы
        y_positions = np.arange(n_params)

        for i, interval in enumerate(section_data):
            # Горизонтальная линия интервала
            ax.plot([interval['lower'], interval['upper']],
                   [i, i],
                   color=colors[section_key],
                   linewidth=3,
                   marker='|',
                   markersize=15,
                   markeredgewidth=2,
                   label=interval['name'] if i == 0 else "")

            # Центральная точка (оценка)
            ax.plot(interval['center'], i,
                   'o',
                   color=colors[section_key],
                   markersize=10,
                   markeredgecolor='black',
                   markeredgewidth=1.5)

            # Подписи значений
            ax.text(interval['lower'], i - 0.3,
                   f"{interval['lower']:.4f}",
                   ha='center',
                   fontsize=9,
                   color='darkgray')
            ax.text(interval['upper'], i - 0.3,
                   f"{interval['upper']:.4f}",
                   ha='center',
                   fontsize=9,
                   color='darkgray')
            ax.text(interval['center'], i + 0.3,
                   f"{interval['center']:.4f}",
                   ha='center',
                   fontsize=10,
                   fontweight='bold',
                   color='black')

        # Настройка осей
        ax.set_yticks(y_positions)
        ax.set_yticklabels([interval['name'] for interval in section_data])
        ax.set_xlabel('Значение параметра', fontsize=11)
        ax.set_title(section_titles[section_key], fontsize=12, fontweight='bold')
        ax.grid(True, alpha=0.3, axis='x')
        ax.set_ylim(-0.5, n_params - 0.5)

        # Добавляем вертикальную линию в центре
        x_min = min([interval['lower'] for interval in section_data])
        x_max = max([interval['upper'] for interval in section_data])
        x_center = (x_min + x_max) / 2
        ax.axvline(x_center, color='gray', linestyle='--', alpha=0.3, linewidth=1)

    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def main():
    if len(sys.argv) < 3:
        print("Использование: python plot_confidence_intervals.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    try:
        intervals = read_confidence_intervals(input_file)
        plot_confidence_intervals(intervals, output_file)
    except Exception as e:
        print(f"Ошибка: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
