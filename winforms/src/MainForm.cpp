#include "../include/MainForm.h"
#include <msclr/marshal_cppstd.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// Подключение расчётных модулей из основного проекта
#include "../../include/mle_methods.h"
#include "../../include/order.h"

using namespace System;
using namespace System::IO;
using namespace System::Windows::Forms;
using namespace System::Diagnostics;
using namespace msclr::interop;

namespace StatisticalAnalysis {

    // ==================== Обработчики меню ====================

    void MainForm::OnFileOpen(Object^ sender, EventArgs^ e)
    {
        OpenFileDialog^ openDialog = gcnew OpenFileDialog();
        openDialog->Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
        openDialog->Title = "Выберите файл данных";
        openDialog->InitialDirectory = Path::GetFullPath("input");

        if (openDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
        {
            this->currentFileName = openDialog->FileName;
            OnLoadFile(sender, e);
        }
    }

    void MainForm::OnFileSave(Object^ sender, EventArgs^ e)
    {
        SaveFileDialog^ saveDialog = gcnew SaveFileDialog();
        saveDialog->Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
        saveDialog->Title = "Сохранить результаты";
        saveDialog->InitialDirectory = Path::GetFullPath("output");

        if (saveDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
        {
            // Определяем какие результаты сохранять
            String^ content = "";
            if (this->tabControl->SelectedTab == this->tabPageMLE)
            {
                content = this->textBoxMLEResults->Text;
            }
            else if (this->tabControl->SelectedTab == this->tabPageMLS)
            {
                content = this->textBoxMLSResults->Text;
            }

            try
            {
                File::WriteAllText(saveDialog->FileName, content);
                ShowStatus("Результаты сохранены: " + Path::GetFileName(saveDialog->FileName));
            }
            catch (Exception^ ex)
            {
                MessageBox::Show("Ошибка сохранения: " + ex->Message, "Ошибка",
                    MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
    }

    void MainForm::OnFileExit(Object^ sender, EventArgs^ e)
    {
        Application::Exit();
    }

    void MainForm::OnHelpAbout(Object^ sender, EventArgs^ e)
    {
        MessageBox::Show(
            "Статистический анализ - Windows Forms\n\n"
            "Программа для оценки параметров распределений\n"
            "методами MLE и MLS с поддержкой цензурированных данных.\n\n"
            "Версия 1.0\n"
            "© 2024 xloned",
            "О программе",
            MessageBoxButtons::OK,
            MessageBoxIcon::Information
        );
    }

    // ==================== Обработчики анализа ====================

    void MainForm::OnMLERun(Object^ sender, EventArgs^ e)
    {
        if (this->currentData->Count == 0)
        {
            MessageBox::Show("Сначала загрузите файл с данными!", "Предупреждение",
                MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        try
        {
            ShowStatus("Выполняется MLE анализ...");
            this->progressBar->Visible = true;
            this->textBoxMLEResults->Clear();

            // Конвертируем данные из managed в native
            std::vector<double> data;
            std::vector<int> censored;

            for (int i = 0; i < this->currentData->Count; i++)
            {
                data.push_back(this->currentData[i]);
            }

            for (int i = 0; i < this->currentCensored->Count; i++)
            {
                censored.push_back(this->currentCensored[i]);
            }

            MLEResult result;

            if (this->radioMLENormal->Checked)
            {
                // MLE для нормального распределения
                result = mle_normal_complete(data);

                this->textBoxMLEResults->AppendText("=== MLE - Нормальное распределение ===\n\n");
            }
            else if (this->radioMLEWeibull->Checked)
            {
                // MLE для распределения Вейбулла
                result = mle_weibull_complete(data);

                this->textBoxMLEResults->AppendText("=== MLE - Распределение Вейбулла ===\n\n");
            }

            // Вывод результатов
            this->textBoxMLEResults->AppendText("Оценки параметров:\n");
            for (size_t i = 0; i < result.parameters.size(); i++)
            {
                String^ line = String::Format("  Параметр {0}: {1:F6} ± {2:F6}\n",
                    i + 1, result.parameters[i], result.std_errors[i]);
                this->textBoxMLEResults->AppendText(line);
            }

            this->textBoxMLEResults->AppendText("\n");
            this->textBoxMLEResults->AppendText(String::Format("Лог-правдоподобие: {0:F6}\n", result.log_likelihood));
            this->textBoxMLEResults->AppendText(String::Format("Итераций: {0}\n", result.iterations));
            this->textBoxMLEResults->AppendText(String::Format("Сходимость: {0}\n", result.converged ? "Да" : "Нет"));

            // Освобождаем память
            free_mle_result(result);

            // Запускаем Python скрипт для построения графика
            String^ pythonScript = "";
            String^ mode = "mle";

            if (this->radioMLENormal->Checked)
            {
                pythonScript = "python/plot_normal.py";
            }
            else
            {
                pythonScript = "python/plot_weibull.py";
            }

            // Формируем команду для запуска Python
            String^ command = String::Format(
                "python/venv/bin/python {0} input/{1} output mle",
                pythonScript,
                Path::GetFileName(this->currentFileName)
            );

            Process^ process = gcnew Process();
            process->StartInfo->FileName = "bash";
            process->StartInfo->Arguments = "-c \"" + command + "\"";
            process->StartInfo->UseShellExecute = false;
            process->StartInfo->RedirectStandardOutput = true;
            process->StartInfo->RedirectStandardError = true;
            process->Start();
            process->WaitForExit();

            String^ output = process->StandardOutput->ReadToEnd();
            String^ error = process->StandardError->ReadToEnd();

            if (!String::IsNullOrEmpty(error))
            {
                this->textBoxMLEResults->AppendText("\nПредупреждение при построении графика:\n" + error);
            }

            ShowStatus("MLE анализ завершён");
            this->progressBar->Visible = false;
        }
        catch (Exception^ ex)
        {
            MessageBox::Show("Ошибка при выполнении анализа: " + ex->Message, "Ошибка",
                MessageBoxButtons::OK, MessageBoxIcon::Error);
            this->progressBar->Visible = false;
            ShowStatus("Ошибка анализа");
        }
    }

    void MainForm::OnMLSRun(Object^ sender, EventArgs^ e)
    {
        if (this->currentData->Count == 0)
        {
            MessageBox::Show("Сначала загрузите файл с данными!", "Предупреждение",
                MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        try
        {
            ShowStatus("Выполняется MLS анализ...");
            this->progressBar->Visible = true;
            this->textBoxMLSResults->Clear();

            // Конвертируем данные
            std::vector<double> data;
            for (int i = 0; i < this->currentData->Count; i++)
            {
                data.push_back(this->currentData[i]);
            }

            // MLS для нормального распределения (метод Дэйвида)
            MLEResult result = mls_normal_complete(data);

            this->textBoxMLSResults->AppendText("=== MLS - Нормальное распределение (метод Дэйвида) ===\n\n");

            // Вывод результатов
            this->textBoxMLSResults->AppendText("Оценки параметров:\n");
            for (size_t i = 0; i < result.parameters.size(); i++)
            {
                String^ line = String::Format("  Параметр {0}: {1:F6} ± {2:F6}\n",
                    i + 1, result.parameters[i], result.std_errors[i]);
                this->textBoxMLSResults->AppendText(line);
            }

            this->textBoxMLSResults->AppendText("\n");
            this->textBoxMLSResults->AppendText(String::Format("Лог-правдоподобие: {0:F6}\n", result.log_likelihood));
            this->textBoxMLSResults->AppendText(String::Format("Итераций: {0}\n", result.iterations));
            this->textBoxMLSResults->AppendText(String::Format("Сходимость: {0}\n", result.converged ? "Да" : "Нет"));

            free_mle_result(result);

            // Запуск Python для графика
            String^ command = String::Format(
                "python/venv/bin/python python/plot_normal.py input/{0} output mls",
                Path::GetFileName(this->currentFileName)
            );

            Process^ process = gcnew Process();
            process->StartInfo->FileName = "bash";
            process->StartInfo->Arguments = "-c \"" + command + "\"";
            process->StartInfo->UseShellExecute = false;
            process->Start();
            process->WaitForExit();

            ShowStatus("MLS анализ завершён");
            this->progressBar->Visible = false;
        }
        catch (Exception^ ex)
        {
            MessageBox::Show("Ошибка при выполнении анализа: " + ex->Message, "Ошибка",
                MessageBoxButtons::OK, MessageBoxIcon::Error);
            this->progressBar->Visible = false;
            ShowStatus("Ошибка анализа");
        }
    }

    // ==================== Обработчики данных ====================

    void MainForm::OnFileSelected(Object^ sender, EventArgs^ e)
    {
        if (this->listBoxFiles->SelectedIndex >= 0)
        {
            String^ fileName = this->listBoxFiles->SelectedItem->ToString();
            this->currentFileName = "input/" + fileName;
            UpdateDataInfo();
        }
    }

    void MainForm::OnLoadFile(Object^ sender, EventArgs^ e)
    {
        if (String::IsNullOrEmpty(this->currentFileName))
        {
            MessageBox::Show("Сначала выберите файл!", "Предупреждение",
                MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        try
        {
            this->currentData->Clear();
            this->currentCensored->Clear();

            StreamReader^ reader = gcnew StreamReader(this->currentFileName);
            String^ line;
            bool hasCensored = false;

            while ((line = reader->ReadLine()) != nullptr)
            {
                line = line->Trim();
                if (String::IsNullOrEmpty(line))
                    continue;

                array<String^>^ parts = line->Split(gcnew array<wchar_t>{' ', '\t'},
                    StringSplitOptions::RemoveEmptyEntries);

                double value = Double::Parse(parts[0]);
                this->currentData->Add(value);

                if (parts->Length > 1)
                {
                    int censorFlag = Int32::Parse(parts[1]);
                    this->currentCensored->Add(censorFlag);
                    if (censorFlag == 1)
                        hasCensored = true;
                }
                else
                {
                    this->currentCensored->Add(0);
                }
            }

            reader->Close();

            this->checkBoxCensored->Checked = hasCensored;
            UpdateDataInfo();

            ShowStatus(String::Format("Загружено {0} значений из {1}",
                this->currentData->Count, Path::GetFileName(this->currentFileName)));
        }
        catch (Exception^ ex)
        {
            MessageBox::Show("Ошибка загрузки файла: " + ex->Message, "Ошибка",
                MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void MainForm::OnEditData(Object^ sender, EventArgs^ e)
    {
        MessageBox::Show("Редактор данных будет добавлен в следующей версии", "Информация",
            MessageBoxButtons::OK, MessageBoxIcon::Information);
    }

    // ==================== Обработчики графиков ====================

    void MainForm::OnChartTypeChanged(Object^ sender, EventArgs^ e)
    {
        if (this->comboBoxChartType->SelectedIndex < 0)
            return;

        String^ chartType = this->comboBoxChartType->SelectedItem->ToString();
        String^ fileName = "";

        if (chartType->Contains("MLE") && chartType->Contains("Нормальное"))
        {
            fileName = "output/plot_mle_normal.png";
            this->labelChartTitle->Text = "MLE - Нормальное распределение";
        }
        else if (chartType->Contains("MLE") && chartType->Contains("Вейбулла"))
        {
            fileName = "output/plot_mle_weibull.png";
            this->labelChartTitle->Text = "MLE - Распределение Вейбулла";
        }
        else if (chartType->Contains("MLS"))
        {
            fileName = "output/plot_mls_normal.png";
            this->labelChartTitle->Text = "MLS - Нормальное распределение";
        }

        if (File::Exists(fileName))
        {
            try
            {
                this->pictureBoxChart->Image = Image::FromFile(fileName);
                ShowStatus("График загружен: " + Path::GetFileName(fileName));
            }
            catch (Exception^ ex)
            {
                MessageBox::Show("Ошибка загрузки графика: " + ex->Message, "Ошибка",
                    MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
        else
        {
            this->pictureBoxChart->Image = nullptr;
            MessageBox::Show("Файл графика не найден. Сначала выполните анализ!", "Предупреждение",
                MessageBoxButtons::OK, MessageBoxIcon::Warning);
        }
    }

    void MainForm::OnExportChart(Object^ sender, EventArgs^ e)
    {
        if (this->pictureBoxChart->Image == nullptr)
        {
            MessageBox::Show("Сначала загрузите график!", "Предупреждение",
                MessageBoxButtons::OK, MessageBoxIcon::Warning);
            return;
        }

        SaveFileDialog^ saveDialog = gcnew SaveFileDialog();
        saveDialog->Filter = "PNG Image (*.png)|*.png|All files (*.*)|*.*";
        saveDialog->Title = "Экспорт графика";

        if (saveDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
        {
            try
            {
                this->pictureBoxChart->Image->Save(saveDialog->FileName);
                ShowStatus("График экспортирован: " + Path::GetFileName(saveDialog->FileName));
            }
            catch (Exception^ ex)
            {
                MessageBox::Show("Ошибка экспорта: " + ex->Message, "Ошибка",
                    MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
    }

    // ==================== Вспомогательные методы ====================

    void MainForm::LoadFileList(void)
    {
        try
        {
            this->listBoxFiles->Items->Clear();

            if (Directory::Exists("input"))
            {
                array<String^>^ files = Directory::GetFiles("input", "*.txt");

                for each (String^ file in files)
                {
                    this->listBoxFiles->Items->Add(Path::GetFileName(file));
                }

                ShowStatus(String::Format("Найдено {0} файлов данных", files->Length));
            }
        }
        catch (Exception^ ex)
        {
            MessageBox::Show("Ошибка загрузки списка файлов: " + ex->Message, "Ошибка",
                MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }

    void MainForm::UpdateDataInfo(void)
    {
        if (this->currentData->Count == 0)
        {
            this->labelDataInfo->Text = "Нет загруженных данных";
            return;
        }

        // Вычисляем статистику
        double sum = 0, sum_sq = 0;
        double min_val = Double::MaxValue;
        double max_val = Double::MinValue;
        int censored_count = 0;

        for (int i = 0; i < this->currentData->Count; i++)
        {
            double val = this->currentData[i];
            sum += val;
            sum_sq += val * val;

            if (val < min_val) min_val = val;
            if (val > max_val) max_val = val;

            if (this->currentCensored[i] == 1)
                censored_count++;
        }

        double mean = sum / this->currentData->Count;
        double variance = (sum_sq / this->currentData->Count) - (mean * mean);
        double std_dev = Math::Sqrt(variance);

        String^ info = String::Format(
            "Файл: {0}\n\n"
            "Статистика:\n"
            "  Размер выборки: {1}\n"
            "  Среднее: {2:F4}\n"
            "  Ст. отклонение: {3:F4}\n"
            "  Минимум: {4:F4}\n"
            "  Максимум: {5:F4}\n",
            Path::GetFileName(this->currentFileName),
            this->currentData->Count,
            mean,
            std_dev,
            min_val,
            max_val
        );

        if (censored_count > 0)
        {
            info += String::Format("\n  Цензурировано: {0} ({1:F1}%)",
                censored_count,
                100.0 * censored_count / this->currentData->Count);
        }

        this->labelDataInfo->Text = info;
    }

    void MainForm::ShowStatus(String^ message)
    {
        this->statusLabel->Text = message;
        this->statusStrip->Refresh();
    }

    void MainForm::ClearOutputFolder(void)
    {
        try
        {
            if (Directory::Exists("output"))
            {
                array<String^>^ files = Directory::GetFiles("output");
                int count = 0;

                for each (String^ file in files)
                {
                    String^ ext = Path::GetExtension(file)->ToLower();
                    if (ext == ".png" || ext == ".txt")
                    {
                        File::Delete(file);
                        count++;
                    }
                }

                ShowStatus(String::Format("Очищена папка output: {0} файлов удалено", count));
            }
        }
        catch (Exception^ ex)
        {
            // Игнорируем ошибки очистки
        }
    }
}
