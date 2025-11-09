#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Установить информацию о приложении
    app.setApplicationName("Statistical Analysis");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Agamirov Methods");

    // Установить стиль для macOS
    #ifdef Q_OS_MACOS
        app.setStyle(QStyleFactory::create("Fusion"));
    #endif

    // Установить шрифт
    QFont font = app.font();
    font.setPointSize(11);
    app.setFont(font);

    // Создать и показать главное окно
    MainWindow window;
    window.setWindowTitle("Статистический анализ - Методы MLS метод");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
