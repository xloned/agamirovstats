#ifndef DATAEDITOR_H
#define DATAEDITOR_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <vector>

/**
 * @brief Редактор данных для просмотра и редактирования INP/OUT файлов
 *
 * Позволяет:
 * - Просматривать и редактировать входные данные
 * - Добавлять/удалять значения
 * - Сохранять изменения
 * - Импортировать данные из файлов
 */
class DataEditor : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DataEditor)

public:
    explicit DataEditor(QWidget *parent = nullptr);
    ~DataEditor();

    // Загрузка и сохранение данных
    void loadFromFile(const QString& fileName);
    void saveToFile(const QString& fileName);

    // Получение данных
    std::vector<double> getData() const;
    void setData(const std::vector<double>& data);

    // Управление редактированием
    void setReadOnly(bool readOnly);
    bool isModified() const { return modified; }

signals:
    void dataChanged();
    void dataLoaded(int count);
    void dataSaved(const QString& fileName);

private slots:
    void onAddRow();
    void onDeleteRow();
    void onClearAll();
    void onCellChanged(int row, int column);

private:
    void setupUI();
    void updateTable();
    void parseFileContent(const QString& content);

private:
    QTableWidget* table;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* clearButton;
    QLineEdit* searchBox;

    std::vector<double> currentData;
    bool modified;
};

#endif // DATAEDITOR_H
