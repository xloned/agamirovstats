#!/usr/bin/env python3
"""
Визуализация результатов критерия Граббса для выявления выбросов
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import sys

def read_grubbs_results(filename):
    """Чтение результатов критерия Граббса из файла"""
    data = []
    outliers = []
    test_results = {}

    with open(filename, 'r') as f:
        section = None
        for line in f:
            line = line.strip()
            if not line or line.startswith('==='):
                continue

            if '# Данные' in line:
                section = 'data'
                continue
            elif 'Критерий Граббса' in line:
                section = 'results'
                continue

            if section == 'data':
                parts = line.split()
                if len(parts) >= 1:
                    try:
                        value = float(parts[0])
                        data.append(value)
                    except:
                        pass

            elif section == 'results':
                if 'G-статистика' in line:
                    test_results['G_stat'] = float(line.split(':')[1].strip())
                elif 'Критическое значение' in line:
                    test_results['G_crit'] = float(line.split(':')[1].strip())
                elif 'Подозрительное значение' in line:
                    outlier_val = float(line.split(':')[1].strip())
                    outliers.append(outlier_val)
                elif 'Вывод' in line:
                    test_results['conclusion'] = line.split(':')[1].strip()

    return np.array(data), outliers, test_results

def plot_grubbs_test(data, outliers, test_results, output_file):
    """Построение визуализации результатов критерия Граббса"""

    if len(data) == 0:
        print("Нет данных для построения графиков")
        return

    fig = plt.figure(figsize=(14, 10), dpi=100)
    fig.suptitle('Критерий Граббса - Тест на выбросы', fontsize=16, fontweight='bold')

    # График 1: Box plot с выделением выбросов
    ax1 = plt.subplot(2, 2, 1)

    bp = ax1.boxplot(data, vert=True, patch_artist=True, widths=0.5)
    bp['boxes'][0].set_facecolor('#3498db')
    bp['boxes'][0].set_alpha(0.7)

    # Отмечаем выбросы красным
    for outlier in outliers:
        if outlier in data:
            idx = np.where(data == outlier)[0]
            for i in idx:
                ax1.plot(1, outlier, 'ro', markersize=15, markeredgewidth=2,
                        markeredgecolor='darkred', label='Выброс' if i == idx[0] else "")

    ax1.set_ylabel('Значение', fontsize=11, fontweight='bold')
    ax1.set_title('Box Plot с выбросами', fontsize=12, fontweight='bold')
    ax1.grid(True, alpha=0.3, axis='y')
    ax1.set_xticklabels(['Данные'])
    if len(outliers) > 0:
        ax1.legend()

    # График 2: Гистограмма с отмеченными выбросами
    ax2 = plt.subplot(2, 2, 2)

    n, bins, patches = ax2.hist(data, bins=20, color='#3498db', alpha=0.7, edgecolor='black')

    # Отмечаем области с выбросами
    for outlier in outliers:
        ax2.axvline(outlier, color='red', linestyle='--', linewidth=2.5,
                   label=f'Выброс: {outlier:.2f}' if outlier == outliers[0] else "")

    ax2.set_xlabel('Значение', fontsize=11, fontweight='bold')
    ax2.set_ylabel('Частота', fontsize=11, fontweight='bold')
    ax2.set_title('Гистограмма данных', fontsize=12, fontweight='bold')
    ax2.grid(True, alpha=0.3, axis='y')
    if len(outliers) > 0:
        ax2.legend()

    # График 3: Индексный график (значения по порядку)
    ax3 = plt.subplot(2, 2, 3)

    indices = np.arange(len(data))
    ax3.plot(indices, data, 'o-', color='#3498db', markersize=6, linewidth=1.5, alpha=0.7)

    # Отмечаем выбросы
    for outlier in outliers:
        outlier_indices = np.where(data == outlier)[0]
        for idx in outlier_indices:
            ax3.plot(idx, outlier, 'ro', markersize=12, markeredgewidth=2,
                    markeredgecolor='darkred', zorder=5)

    # Линии среднего и границ
    mean_val = np.mean(data)
    std_val = np.std(data, ddof=1)
    ax3.axhline(mean_val, color='green', linestyle='--', linewidth=2, label=f'Среднее: {mean_val:.2f}')
    ax3.axhline(mean_val + 2*std_val, color='orange', linestyle=':', linewidth=1.5, alpha=0.7, label='±2σ')
    ax3.axhline(mean_val - 2*std_val, color='orange', linestyle=':', linewidth=1.5, alpha=0.7)
    ax3.axhline(mean_val + 3*std_val, color='red', linestyle=':', linewidth=1.5, alpha=0.7, label='±3σ')
    ax3.axhline(mean_val - 3*std_val, color='red', linestyle=':', linewidth=1.5, alpha=0.7)

    ax3.set_xlabel('Индекс наблюдения', fontsize=11, fontweight='bold')
    ax3.set_ylabel('Значение', fontsize=11, fontweight='bold')
    ax3.set_title('Индексный график', fontsize=12, fontweight='bold')
    ax3.grid(True, alpha=0.3)
    ax3.legend(loc='best', fontsize=9)

    # График 4: Текстовая информация о тесте
    ax4 = plt.subplot(2, 2, 4)
    ax4.axis('off')

    # Формируем текст с результатами
    info_text = "РЕЗУЛЬТАТЫ КРИТЕРИЯ ГРАББСА\n"
    info_text += "=" * 40 + "\n\n"
    info_text += f"Размер выборки: {len(data)}\n"
    info_text += f"Среднее: {np.mean(data):.4f}\n"
    info_text += f"Ст. отклонение: {np.std(data, ddof=1):.4f}\n"
    info_text += f"Минимум: {np.min(data):.4f}\n"
    info_text += f"Максимум: {np.max(data):.4f}\n\n"

    if 'G_stat' in test_results:
        info_text += f"G-статистика: {test_results['G_stat']:.4f}\n"
    if 'G_crit' in test_results:
        info_text += f"Критическое значение: {test_results['G_crit']:.4f}\n\n"

    if len(outliers) > 0:
        info_text += f"Обнаружено выбросов: {len(outliers)}\n"
        for i, outlier in enumerate(outliers, 1):
            info_text += f"  {i}. Значение: {outlier:.4f}\n"
    else:
        info_text += "Выбросы не обнаружены\n"

    info_text += "\n" + "=" * 40 + "\n"
    if 'conclusion' in test_results:
        info_text += f"\nВЫВОД:\n{test_results['conclusion']}"

    ax4.text(0.1, 0.5, info_text, fontsize=11, family='monospace',
            verticalalignment='center', bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"График сохранен: {output_file}")
    plt.close()

def main():
    if len(sys.argv) < 3:
        print("Использование: python plot_grubbs.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    try:
        data, outliers, test_results = read_grubbs_results(input_file)
        plot_grubbs_test(data, outliers, test_results, output_file)
    except Exception as e:
        print(f"Ошибка: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
