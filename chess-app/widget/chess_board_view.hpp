#pragma once

#include <QGraphicsView>
#include "chess_board_scene.hpp"

class ChessBoardView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ChessBoardView(QWidget *parent = nullptr);

    // シーンのラッパー
    void setBoardFromFEN(const std::string &fen);

    // ウィジェットの推奨サイズを設定
    QSize sizeHint() const override
    {
        return QSize(600, 600);
    }

signals:
    // Sceneから受け取ったクリックシグナルを中継
    void squareClicked(const QString &algebraicCoord);

private:
    ChessBoardScene *m_boardScene;

protected:
    /**
     * @brief Viewのサイズが変更されたときに、Sceneをフィットさせる
     */
    void resizeEvent(QResizeEvent *event) override; // ★ この宣言を追加 ★

public slots: // MainWindow::setupConnectionsで接続するために必須

    /**
     * @brief 合法手リストを受け取り、シーンにハイライトを要求する
     * @param legalCells 合法な移動先のマス目のリスト。要素は {rank, file} (0-7) 形式。
     */
    void handleLegalMoves(const std::vector<std::pair<int, int>> &legalCells);
};