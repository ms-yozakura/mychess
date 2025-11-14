// main.cpp (最終版)

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QThread>

// 依存するヘッダ
#include "main_window.hpp"
#include "chess/chess_game.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ChessGame game;
    MainWindow window(&game);

    window.show();

    int result = app.exec();
    return result;
}