#pragma once

// Включаем Windows.h до .NET заголовков для избежания конфликтов COM интерфейсов
#include <windows.h>

// Отключаем предупреждения о переопределении
#pragma warning(push)
#pragma warning(disable: 4099)  // тип переопределения
#pragma warning(disable: 4244)  // преобразование типов

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace StatisticalAnalysis {

    /// <summary>
    /// Главная форма приложения статистического анализа
    /// </summary>
    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm(void)
        {
            InitializeComponent();
        }

    protected:
        /// <summary>
        /// Очистка ресурсов
        /// </summary>
        ~MainForm()
        {
            if (components)
            {
                delete components;
            }
        }

    private:
        // Компоненты UI
        System::Windows::Forms::TabControl^ tabControl;
        System::Windows::Forms::TabPage^ tabPageMLE;
        System::Windows::Forms::TabPage^ tabPageMLS;
        System::Windows::Forms::TabPage^ tabPageData;
        System::Windows::Forms::TabPage^ tabPageChart;

        // MLE вкладка
        System::Windows::Forms::GroupBox^ groupBoxMLEType;
        System::Windows::Forms::RadioButton^ radioMLENormal;
        System::Windows::Forms::RadioButton^ radioMLEWeibull;
        System::Windows::Forms::Button^ buttonMLERun;
        System::Windows::Forms::RichTextBox^ textBoxMLEResults;

        // MLS вкладка
        System::Windows::Forms::GroupBox^ groupBoxMLSType;
        System::Windows::Forms::RadioButton^ radioMLSNormal;
        System::Windows::Forms::Button^ buttonMLSRun;
        System::Windows::Forms::RichTextBox^ textBoxMLSResults;

        // Вкладка данных
        System::Windows::Forms::GroupBox^ groupBoxFileSelect;
        System::Windows::Forms::ListBox^ listBoxFiles;
        System::Windows::Forms::Button^ buttonLoadFile;
        System::Windows::Forms::Button^ buttonEditData;
        System::Windows::Forms::CheckBox^ checkBoxCensored;
        System::Windows::Forms::Label^ labelDataInfo;

        // Вкладка графиков
        System::Windows::Forms::PictureBox^ pictureBoxChart;
        System::Windows::Forms::Button^ buttonExportChart;
        System::Windows::Forms::ComboBox^ comboBoxChartType;
        System::Windows::Forms::Label^ labelChartTitle;

        // Меню
        System::Windows::Forms::MenuStrip^ menuStrip;
        System::Windows::Forms::ToolStripMenuItem^ menuFile;
        System::Windows::Forms::ToolStripMenuItem^ menuFileOpen;
        System::Windows::Forms::ToolStripMenuItem^ menuFileSave;
        System::Windows::Forms::ToolStripMenuItem^ menuFileExit;
        System::Windows::Forms::ToolStripMenuItem^ menuHelp;
        System::Windows::Forms::ToolStripMenuItem^ menuHelpAbout;

        // Статус бар
        System::Windows::Forms::StatusStrip^ statusStrip;
        System::Windows::Forms::ToolStripStatusLabel^ statusLabel;
        System::Windows::Forms::ToolStripProgressBar^ progressBar;

        // Данные
        System::Collections::Generic::List<double>^ currentData;
        System::Collections::Generic::List<int>^ currentCensored;
        String^ currentFileName;

        System::ComponentModel::Container^ components;

        /// <summary>
        /// Инициализация компонентов формы
        /// </summary>
        void InitializeComponent(void)
        {
            this->components = gcnew System::ComponentModel::Container();
            this->Text = L"Статистический анализ - Windows Forms";
            this->Size = System::Drawing::Size(1000, 700);
            this->StartPosition = FormStartPosition::CenterScreen;

            // Инициализация данных
            this->currentData = gcnew System::Collections::Generic::List<double>();
            this->currentCensored = gcnew System::Collections::Generic::List<int>();
            this->currentFileName = L"";

            // Создание меню
            InitializeMenu();

            // Создание вкладок
            InitializeTabs();

            // Создание статус бара
            InitializeStatusBar();

            // Загрузка списка файлов
            LoadFileList();
        }

        /// <summary>
        /// Инициализация меню
        /// </summary>
        void InitializeMenu(void)
        {
            this->menuStrip = gcnew MenuStrip();

            // Меню File
            this->menuFile = gcnew ToolStripMenuItem(L"&Файл");
            this->menuFileOpen = gcnew ToolStripMenuItem(L"&Открыть...");
            this->menuFileSave = gcnew ToolStripMenuItem(L"&Сохранить результаты...");
            this->menuFileExit = gcnew ToolStripMenuItem(L"В&ыход");

            this->menuFile->DropDownItems->Add(this->menuFileOpen);
            this->menuFile->DropDownItems->Add(this->menuFileSave);
            this->menuFile->DropDownItems->Add(gcnew ToolStripSeparator());
            this->menuFile->DropDownItems->Add(this->menuFileExit);

            // Меню Help
            this->menuHelp = gcnew ToolStripMenuItem(L"&Справка");
            this->menuHelpAbout = gcnew ToolStripMenuItem(L"&О программе");
            this->menuHelp->DropDownItems->Add(this->menuHelpAbout);

            this->menuStrip->Items->Add(this->menuFile);
            this->menuStrip->Items->Add(this->menuHelp);

            // Обработчики событий
            this->menuFileOpen->Click += gcnew EventHandler(this, &MainForm::OnFileOpen);
            this->menuFileSave->Click += gcnew EventHandler(this, &MainForm::OnFileSave);
            this->menuFileExit->Click += gcnew EventHandler(this, &MainForm::OnFileExit);
            this->menuHelpAbout->Click += gcnew EventHandler(this, &MainForm::OnHelpAbout);

            this->Controls->Add(this->menuStrip);
            this->MainMenuStrip = this->menuStrip;
        }

        /// <summary>
        /// Инициализация вкладок
        /// </summary>
        void InitializeTabs(void)
        {
            this->tabControl = gcnew TabControl();
            this->tabControl->Location = Point(10, 30);
            this->tabControl->Size = System::Drawing::Size(960, 580);

            // Создание вкладок
            InitializeMLETab();
            InitializeMLSTab();
            InitializeDataTab();
            InitializeChartTab();

            this->Controls->Add(this->tabControl);
        }

        /// <summary>
        /// Инициализация вкладки MLE
        /// </summary>
        void InitializeMLETab(void)
        {
            this->tabPageMLE = gcnew TabPage(L"MLE - Метод максимального правдоподобия");

            // Группа выбора типа распределения
            this->groupBoxMLEType = gcnew GroupBox();
            this->groupBoxMLEType->Text = L"Тип распределения";
            this->groupBoxMLEType->Location = Point(10, 10);
            this->groupBoxMLEType->Size = System::Drawing::Size(200, 100);

            this->radioMLENormal = gcnew RadioButton();
            this->radioMLENormal->Text = L"Нормальное";
            this->radioMLENormal->Location = Point(10, 20);
            this->radioMLENormal->Checked = true;

            this->radioMLEWeibull = gcnew RadioButton();
            this->radioMLEWeibull->Text = L"Вейбулла";
            this->radioMLEWeibull->Location = Point(10, 50);

            this->groupBoxMLEType->Controls->Add(this->radioMLENormal);
            this->groupBoxMLEType->Controls->Add(this->radioMLEWeibull);

            // Кнопка запуска
            this->buttonMLERun = gcnew Button();
            this->buttonMLERun->Text = L"Выполнить анализ";
            this->buttonMLERun->Location = Point(10, 120);
            this->buttonMLERun->Size = System::Drawing::Size(200, 30);
            this->buttonMLERun->Click += gcnew EventHandler(this, &MainForm::OnMLERun);

            // Результаты
            this->textBoxMLEResults = gcnew RichTextBox();
            this->textBoxMLEResults->Location = Point(220, 10);
            this->textBoxMLEResults->Size = System::Drawing::Size(710, 520);
            this->textBoxMLEResults->Font = gcnew System::Drawing::Font(L"Consolas", 10);
            this->textBoxMLEResults->ReadOnly = true;

            this->tabPageMLE->Controls->Add(this->groupBoxMLEType);
            this->tabPageMLE->Controls->Add(this->buttonMLERun);
            this->tabPageMLE->Controls->Add(this->textBoxMLEResults);

            this->tabControl->TabPages->Add(this->tabPageMLE);
        }

        /// <summary>
        /// Инициализация вкладки MLS
        /// </summary>
        void InitializeMLSTab(void)
        {
            this->tabPageMLS = gcnew TabPage(L"MLS - Метод наименьших квадратов");

            // Группа выбора типа
            this->groupBoxMLSType = gcnew GroupBox();
            this->groupBoxMLSType->Text = L"Тип распределения";
            this->groupBoxMLSType->Location = Point(10, 10);
            this->groupBoxMLSType->Size = System::Drawing::Size(200, 80);

            this->radioMLSNormal = gcnew RadioButton();
            this->radioMLSNormal->Text = L"Нормальное (метод Дэйвида)";
            this->radioMLSNormal->Location = Point(10, 20);
            this->radioMLSNormal->Size = System::Drawing::Size(180, 40);
            this->radioMLSNormal->Checked = true;

            this->groupBoxMLSType->Controls->Add(this->radioMLSNormal);

            // Кнопка запуска
            this->buttonMLSRun = gcnew Button();
            this->buttonMLSRun->Text = L"Выполнить анализ";
            this->buttonMLSRun->Location = Point(10, 100);
            this->buttonMLSRun->Size = System::Drawing::Size(200, 30);
            this->buttonMLSRun->Click += gcnew EventHandler(this, &MainForm::OnMLSRun);

            // Результаты
            this->textBoxMLSResults = gcnew RichTextBox();
            this->textBoxMLSResults->Location = Point(220, 10);
            this->textBoxMLSResults->Size = System::Drawing::Size(710, 520);
            this->textBoxMLSResults->Font = gcnew System::Drawing::Font(L"Consolas", 10);
            this->textBoxMLSResults->ReadOnly = true;

            this->tabPageMLS->Controls->Add(this->groupBoxMLSType);
            this->tabPageMLS->Controls->Add(this->buttonMLSRun);
            this->tabPageMLS->Controls->Add(this->textBoxMLSResults);

            this->tabControl->TabPages->Add(this->tabPageMLS);
        }

        /// <summary>
        /// Инициализация вкладки данных
        /// </summary>
        void InitializeDataTab(void)
        {
            this->tabPageData = gcnew TabPage(L"Данные");

            // Группа выбора файла
            this->groupBoxFileSelect = gcnew GroupBox();
            this->groupBoxFileSelect->Text = L"Входные файлы";
            this->groupBoxFileSelect->Location = Point(10, 10);
            this->groupBoxFileSelect->Size = System::Drawing::Size(300, 400);

            this->listBoxFiles = gcnew ListBox();
            this->listBoxFiles->Location = Point(10, 20);
            this->listBoxFiles->Size = System::Drawing::Size(280, 330);
            this->listBoxFiles->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::OnFileSelected);

            this->buttonLoadFile = gcnew Button();
            this->buttonLoadFile->Text = L"Загрузить файл";
            this->buttonLoadFile->Location = Point(10, 360);
            this->buttonLoadFile->Size = System::Drawing::Size(130, 30);
            this->buttonLoadFile->Click += gcnew EventHandler(this, &MainForm::OnLoadFile);

            this->buttonEditData = gcnew Button();
            this->buttonEditData->Text = L"Редактировать";
            this->buttonEditData->Location = Point(150, 360);
            this->buttonEditData->Size = System::Drawing::Size(130, 30);
            this->buttonEditData->Click += gcnew EventHandler(this, &MainForm::OnEditData);

            this->groupBoxFileSelect->Controls->Add(this->listBoxFiles);
            this->groupBoxFileSelect->Controls->Add(this->buttonLoadFile);
            this->groupBoxFileSelect->Controls->Add(this->buttonEditData);

            // Информация о данных
            this->checkBoxCensored = gcnew CheckBox();
            this->checkBoxCensored->Text = L"Цензурированные данные";
            this->checkBoxCensored->Location = Point(320, 10);
            this->checkBoxCensored->Size = System::Drawing::Size(200, 30);
            this->checkBoxCensored->Enabled = false;

            this->labelDataInfo = gcnew Label();
            this->labelDataInfo->Location = Point(320, 50);
            this->labelDataInfo->Size = System::Drawing::Size(600, 480);
            this->labelDataInfo->Font = gcnew System::Drawing::Font(L"Consolas", 10);
            this->labelDataInfo->Text = L"Выберите файл для просмотра информации";

            this->tabPageData->Controls->Add(this->groupBoxFileSelect);
            this->tabPageData->Controls->Add(this->checkBoxCensored);
            this->tabPageData->Controls->Add(this->labelDataInfo);

            this->tabControl->TabPages->Add(this->tabPageData);
        }

        /// <summary>
        /// Инициализация вкладки графиков
        /// </summary>
        void InitializeChartTab(void)
        {
            this->tabPageChart = gcnew TabPage(L"Графики");

            // Выбор типа графика
            this->comboBoxChartType = gcnew ComboBox();
            this->comboBoxChartType->Location = Point(10, 10);
            this->comboBoxChartType->Size = System::Drawing::Size(300, 25);
            this->comboBoxChartType->DropDownStyle = ComboBoxStyle::DropDownList;
            this->comboBoxChartType->Items->Add(L"MLE - Нормальное распределение");
            this->comboBoxChartType->Items->Add(L"MLE - Распределение Вейбулла");
            this->comboBoxChartType->Items->Add(L"MLS - Нормальное распределение");
            this->comboBoxChartType->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::OnChartTypeChanged);

            // Кнопка экспорта
            this->buttonExportChart = gcnew Button();
            this->buttonExportChart->Text = L"Экспорт графика";
            this->buttonExportChart->Location = Point(320, 10);
            this->buttonExportChart->Size = System::Drawing::Size(150, 25);
            this->buttonExportChart->Click += gcnew EventHandler(this, &MainForm::OnExportChart);

            // Заголовок
            this->labelChartTitle = gcnew Label();
            this->labelChartTitle->Location = Point(10, 45);
            this->labelChartTitle->Size = System::Drawing::Size(900, 25);
            this->labelChartTitle->Font = gcnew System::Drawing::Font(L"Arial", 12, FontStyle::Bold);
            this->labelChartTitle->Text = L"";

            // Изображение графика
            this->pictureBoxChart = gcnew PictureBox();
            this->pictureBoxChart->Location = Point(10, 75);
            this->pictureBoxChart->Size = System::Drawing::Size(920, 455);
            this->pictureBoxChart->SizeMode = PictureBoxSizeMode::Zoom;
            this->pictureBoxChart->BorderStyle = BorderStyle::FixedSingle;

            this->tabPageChart->Controls->Add(this->comboBoxChartType);
            this->tabPageChart->Controls->Add(this->buttonExportChart);
            this->tabPageChart->Controls->Add(this->labelChartTitle);
            this->tabPageChart->Controls->Add(this->pictureBoxChart);

            this->tabControl->TabPages->Add(this->tabPageChart);
        }

        /// <summary>
        /// Инициализация статус бара
        /// </summary>
        void InitializeStatusBar(void)
        {
            this->statusStrip = gcnew StatusStrip();

            this->statusLabel = gcnew ToolStripStatusLabel();
            this->statusLabel->Text = L"Готов к работе";
            this->statusLabel->Spring = true;
            this->statusLabel->TextAlign = ContentAlignment::MiddleLeft;

            this->progressBar = gcnew ToolStripProgressBar();
            this->progressBar->Visible = false;

            this->statusStrip->Items->Add(this->statusLabel);
            this->statusStrip->Items->Add(this->progressBar);

            this->Controls->Add(this->statusStrip);
        }

        // Обработчики событий меню
        void OnFileOpen(Object^ sender, EventArgs^ e);
        void OnFileSave(Object^ sender, EventArgs^ e);
        void OnFileExit(Object^ sender, EventArgs^ e);
        void OnHelpAbout(Object^ sender, EventArgs^ e);

        // Обработчики событий анализа
        void OnMLERun(Object^ sender, EventArgs^ e);
        void OnMLSRun(Object^ sender, EventArgs^ e);

        // Обработчики событий данных
        void OnFileSelected(Object^ sender, EventArgs^ e);
        void OnLoadFile(Object^ sender, EventArgs^ e);
        void OnEditData(Object^ sender, EventArgs^ e);

        // Обработчики событий графиков
        void OnChartTypeChanged(Object^ sender, EventArgs^ e);
        void OnExportChart(Object^ sender, EventArgs^ e);

        // Вспомогательные методы
        void LoadFileList(void);
        void UpdateDataInfo(void);
        void ShowStatus(String^ message);
        void ClearOutputFolder(void);
    };
}

// Восстанавливаем предупреждения
#pragma warning(pop)
