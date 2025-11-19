#include "chess_board_view.hpp"
#include <QDebug>
#include <QResizeEvent>

// コンストラクタ
ChessBoardView::ChessBoardView(QWidget *parent)
    : QGraphicsView(parent)
{
    // 1. Sceneの作成と設定
    m_boardScene = new ChessBoardScene(this);
    setScene(m_boardScene);

    // 2. Viewの基本設定
    // アンチエイリアスを有効にして描画を滑らかにする
    setRenderHint(QPainter::Antialiasing); 
    // シーンの原点 (0, 0) をViewの中央に配置しない
    setTransformationAnchor(AnchorViewCenter); 
    // スクロールバーを非表示にする
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 3. 信号/スロットの接続
    // Sceneで発生したクリック信号を、Viewの外部 (MainWindow) に中継する
    QObject::connect(m_boardScene, &ChessBoardScene::squareClicked,
                     this, &ChessBoardView::squareClicked);

    // View全体を暗い背景でクリア
    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor("#1f2326")); // 背景色
    setPalette(pal);
}

// FEN文字列を受け取り、Sceneに処理を委譲する (アニメーション開始)
void ChessBoardView::setBoardFromFEN(const std::string &fen)
{
    if (m_boardScene) {
        m_boardScene->setBoardFromFEN(fen);
        // シーンの内容に合わせてViewをフィットさせる
        fitInView(m_boardScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

// 合法手リストを受け取り、Sceneに処理を委譲する (ハイライト描画)
void ChessBoardView::handleLegalMoves(const std::vector<std::pair<int, int>> &legalCells)
{
    if (m_boardScene) {
        m_boardScene->handleLegalMoves(legalCells);
    }
}

// ウィンドウサイズ変更時に盤面をViewにフィットさせる
void ChessBoardView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    if (m_boardScene) {
        fitInView(m_boardScene->sceneRect(), Qt::KeepAspectRatio);
    }
}