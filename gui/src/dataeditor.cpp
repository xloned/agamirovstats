#include "dataeditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

DataEditor::DataEditor(QWidget *parent)
    : QWidget(parent)
    , modified(false)
{
    setupUI();
}

DataEditor::~DataEditor()
{
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void DataEditor::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Таблица данных
    table = new QTableWidget(0, 2, this);
    table->setHorizontalHeaderLabels({"№", "Значение"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(table, &QTableWidget::cellChanged, this, &DataEditor::onCellChanged);

    mainLayout->addWidget(table);

    // Панель кнопок
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    addButton = new QPushButton("Добавить", this);
    deleteButton = new QPushButton("Удалить", this);
    clearButton = new QPushButton("Очистить все", this);

    connect(addButton, &QPushButton::clicked, this, &DataEditor::onAddRow);
    connect(deleteButton, &QPushButton::clicked, this, &DataEditor::onDeleteRow);
    connect(clearButton, &QPushButton::clicked, this, &DataEditor::onClearAll);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

/**
 * @brief Загрузить данные из файла
 */
void DataEditor::loadFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл: " + fileName);
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    parseFileContent(content);
    modified = false;

    emit dataLoaded(currentData.size());
}

/**
 * @brief Сохранить данные в файл
 */
void DataEditor::saveToFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл: " + fileName);
        return;
    }

    QTextStream out(&file);
    for (double value : currentData) {
        out << value << "\n";
    }
    file.close();

    modified = false;
    emit dataSaved(fileName);
}

/**
 * @brief Получить данные
 */
std::vector<double> DataEditor::getData() const
{
    return currentData;
}

/**
 * @brief Установить данные
 */
void DataEditor::setData(const std::vector<double>& data)
{
    currentData = data;
    updateTable();
    modified = false;
}

/**
 * @brief Установить режим только для чтения
 */
void DataEditor::setReadOnly(bool readOnly)
{
    table->setEditTriggers(readOnly ? QAbstractItemView::NoEditTriggers : QAbstractItemView::AllEditTriggers);
    addButton->setEnabled(!readOnly);
    deleteButton->setEnabled(!readOnly);
    clearButton->setEnabled(!readOnly);
}

/**
 * @brief Добавить строку
 */
void DataEditor::onAddRow()
{
    int row = table->rowCount();
    table->insertRow(row);

    QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
    indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
    table->setItem(row, 0, indexItem);

    QTableWidgetItem* valueItem = new QTableWidgetItem("0.0");
    table->setItem(row, 1, valueItem);

    currentData.push_back(0.0);
    modified = true;
    emit dataChanged();
}

/**
 * @brief Удалить строку
 */
void DataEditor::onDeleteRow()
{
    int currentRow = table->currentRow();
    if (currentRow >= 0) {
        table->removeRow(currentRow);
        currentData.erase(currentData.begin() + currentRow);

        // Обновить номера строк
        for (int i = currentRow; i < table->rowCount(); i++) {
            table->item(i, 0)->setText(QString::number(i + 1));
        }

        modified = true;
        emit dataChanged();
    }
}

/**
 * @brief Очистить все данные
 */
void DataEditor::onClearAll()
{
    table->setRowCount(0);
    currentData.clear();
    modified = true;
    emit dataChanged();
}

/**
 * @brief Обработка изменения ячейки
 */
void DataEditor::onCellChanged(int row, int column)
{
    if (column == 1) {  // Столбец значений
        bool ok;
        double value = table->item(row, column)->text().toDouble(&ok);
        if (ok && row < static_cast<int>(currentData.size())) {
            currentData[row] = value;
            modified = true;
            emit dataChanged();
        }
    }
}

/**
 * @brief Обновить таблицу
 */
void DataEditor::updateTable()
{
    table->setRowCount(0);

    for (size_t i = 0; i < currentData.size(); i++) {
        int row = table->rowCount();
        table->insertRow(row);

        QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(row, 0, indexItem);

        QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(currentData[i], 'f', 6));
        table->setItem(row, 1, valueItem);
    }
}

/**
 * @brief Парсинг содержимого файла
 */
void DataEditor::parseFileContent(const QString& content)
{
    currentData.clear();
    QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty() && !trimmed.startsWith('#')) {
            bool ok;
            double value = trimmed.toDouble(&ok);
            if (ok) {
                currentData.push_back(value);
            }
        }
    }

    updateTable();
}
