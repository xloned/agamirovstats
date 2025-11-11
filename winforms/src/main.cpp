#include "../include/MainForm.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace StatisticalAnalysis;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
    // Включение визуальных стилей
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Создание и запуск главной формы
    Application::Run(gcnew MainForm());

    return 0;
}
