#pragma once

#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsEllipseItem> // ハイライト用の円形アイテム
#include <QPropertyAnimation>
#include <QGraphicsRectItem>
#include <QVariantAnimation>
#include <QList>
#include <map>
#include <string>
#include <QMap>
#include <vector>
#include <utility> // std::pair用

#include "../chess/types.hpp"

class ChessBoardScene : public QGraphicsScene
{
    Q_OBJECT

    // 定数定義 (簡略化のため、マスのサイズを固定)
    const int MARGIN = 40;                  // 座標表示用マージン
    const int SQUARE_SIZE = 100;            // 1マスのサイズ
    const int BOARD_SIZE = 8 * SQUARE_SIZE; // 盤面全体 (800)

public:
    explicit ChessBoardScene(QObject *parent = nullptr);

    /**
     * FENを受け取り、盤面状態を更新し、駒の移動をアニメーション付きで実行します。
     * @param fen FEN文字列全体。
     */
    void setBoardFromFEN(const std::string &fen);

    // legalCellsの描画はQGraphicsItemで表現するか、前景(drawForeground)で描画できます。
    // 今回は割愛しますが、必要に応じて追加できます。
    /**
     * @brief 合法手リストを受け取り、盤面にハイライトを描画/更新します。
     * @param legalCells 合法な移動先のマス目のリスト。要素は {rank, file} (0-7) 形式。
     */
    void handleLegalMoves(const std::vector<std::pair<int, int>> &legalCells);

signals:
    // クリックされたマスの代数座標 (例: "e4") を通知
    void squareClicked(const QString &algebraicCoord);

protected:
    // 盤面 (マス目と座標) を描画
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    // マウスプレスイベント (アイテムがない場所のクリックを処理)
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // 現在の盤面上の駒アイテムを管理 (例: "a1" -> QGraphicsSimpleTextItem*)
    QMap<QString, QGraphicsSimpleTextItem *> m_pieceItems;

    // 合法手のハイライトアイテムを管理
    QList<QGraphicsItem *> m_highlightItems;

    // 補助関数: (ランク 0-7, ファイル 0-7) からマス目の矩形を計算
    QRectF getSquareRect(int rank, int file) const;

    // 補助関数: (ランク, ファイル) -> シーン座標 (マスの中央) を計算
    QPointF getCenterPos(int rank, int file) const;

    // 補助関数: FENの文字 -> 絵文字を変換
    QChar getPieceEmoji(QChar pieceChar) const;
};