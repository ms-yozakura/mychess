#include "chess_board_scene.hpp"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QDebug>
#include <algorithm>
#include <QVariantAnimation> 

// --- 定数と補助関数 ---

// (0, 0)が盤面の左上角ではなく、座標表示マージンを含めた全体の左上になるように設定
ChessBoardScene::ChessBoardScene(QObject *parent) 
    : QGraphicsScene(parent)
{
    // シーンの境界を設定 (マージンを含めた全体)
    setSceneRect(-MARGIN, -MARGIN, BOARD_SIZE + 2 * MARGIN, BOARD_SIZE + 2 * MARGIN);
}

// 駒文字から絵文字への変換 (引数を QChar に変更)
QChar ChessBoardScene::getPieceEmoji(QChar pieceChar) const
{
    switch (pieceChar.toLower().toLatin1()) // QCharを小文字のcharに変換
    {
    case 'p': return QChar(L'♟');
    case 'k': return QChar(L'♚');
    case 'q': return QChar(L'♛');
    case 'n': return QChar(L'♞');
    case 'b': return QChar(L'♝');
    case 'r': return QChar(L'♜');
    default: return QChar(0);
    }
}

// (ランク 0-7, ファイル 0-7) からマスの中央のシーン座標を計算
QPointF ChessBoardScene::getCenterPos(int rank, int file) const
{
    qreal x = file * SQUARE_SIZE + SQUARE_SIZE / 2.0;
    qreal y = rank * SQUARE_SIZE + SQUARE_SIZE / 2.0;
    return QPointF(x, y);
}

// (ランク 0-7, ファイル 0-7) からマス目の矩形を計算
QRectF ChessBoardScene::getSquareRect(int rank, int file) const
{
    return QRectF(
        file * SQUARE_SIZE,
        rank * SQUARE_SIZE,
        SQUARE_SIZE,
        SQUARE_SIZE);
}

// --- 描画とイベント ---

// 盤面 (マス目と座標) を描画
void ChessBoardScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    painter->setRenderHint(QPainter::Antialiasing);

    // 1. マス目の描画
    const QColor lightColor("#a0a7ad");
    const QColor darkColor("#474e59");

    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            QColor color = ((rank + file) % 2 == 0) ? darkColor : lightColor;
            
            QRectF squareRect(
                file * SQUARE_SIZE,
                rank * SQUARE_SIZE,
                SQUARE_SIZE,
                SQUARE_SIZE);
                
            painter->fillRect(squareRect, color);
        }
    }

    // 2. 座標の描画 (MARGINは負の座標、つまり盤面の外側)
    QFont coordFont("Arial", MARGIN * 0.5);
    painter->setFont(coordFont);
    painter->setPen(Qt::white);

    // 列アルファベット (a-h)
    for (int file = 0; file < 8; file++)
    {
        char fileChar = 'a' + file;
        QString s(fileChar);
        // 下側のマージンに描画
        painter->drawText(
            QRectF(file * SQUARE_SIZE, BOARD_SIZE, SQUARE_SIZE, MARGIN),
            Qt::AlignTop | Qt::AlignHCenter, s);
    }

    // 行番号 (8-1)
    for (int rank = 0; rank < 8; rank++)
    {
        QString s = QString::number(8 - rank);
        // 左側のマージンに描画
        painter->drawText(
            QRectF(-MARGIN, rank * SQUARE_SIZE, MARGIN, SQUARE_SIZE),
            Qt::AlignRight | Qt::AlignVCenter, s);
    }
}

// マウスイベント処理 (マス目全体を対象)
void ChessBoardScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF scenePos = event->scenePos();
    
    // 盤面内かどうかチェック
    if (scenePos.x() < 0 || scenePos.x() >= BOARD_SIZE ||
        scenePos.y() < 0 || scenePos.y() >= BOARD_SIZE)
    {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    // シーン座標から (ランク, ファイル) に変換
    int file = static_cast<int>(scenePos.x() / SQUARE_SIZE);
    int rank = static_cast<int>(scenePos.y() / SQUARE_SIZE);

    // 代数座標に変換 (例: rank=0, file=0 -> "a8")
    char fileChar = 'a' + file;
    char rankChar = '8' - rank;
    QString algebraicCoord = QString("%1%2").arg(fileChar).arg(rankChar);

    qDebug() << "Scene Clicked:" << algebraicCoord;
    emit squareClicked(algebraicCoord);

    event->accept();
}

// --- FEN処理とアニメーションの核 ---

void ChessBoardScene::setBoardFromFEN(const std::string &fen)
{
    // 1. FENから駒配置を取得
    size_t firstSpace = fen.find(' ');
    std::string placement = (firstSpace == std::string::npos) ? fen : fen.substr(0, firstSpace);

    QMap<QString, QChar> newPiecePlacement; // 新しい盤面状態
    int rank = 0;
    int file = 0;

    for (char c : placement)
    {
        if (c == '/')
        {
            rank++;
            file = 0;
        }
        else if (isdigit(c))
        {
            file += (c - '0');
        }
        else if (isalpha(c))
        {
            // 代数座標 (例: "a8", "h1")
            QString coord = QString("%1%2").arg(QChar('a' + file)).arg(QChar('8' - rank));
            newPiecePlacement[coord] = c;
            file++;
        }
    }

    // 2. 変更点の検出と処理 (削除/移動/新規配置)

    QMap<QString, QGraphicsSimpleTextItem*> oldPieceItems = m_pieceItems;
    m_pieceItems.clear(); // 新しいマップを構築

    for (auto it = newPiecePlacement.begin(); it != newPiecePlacement.end(); ++it)
    {
        // ★修正点1: QMapのイテレータは it.key() と it.value() を使用
        QString newCoord = it.key();  
        QChar pieceChar = it.value(); 
        
        // FENの座標から(ランク,ファイル)を逆算
        int targetFile = newCoord.at(0).toLatin1() - 'a';
        int targetRank = '8' - newCoord.at(1).toLatin1();
        QPointF targetCenterPos = getCenterPos(targetRank, targetFile);

        // a) 移動元を検索: この駒（種類と色）が、旧盤面のどこにあったかを探す
        QGraphicsSimpleTextItem *pieceItem = nullptr;

        for (auto oldIt = oldPieceItems.begin(); oldIt != oldPieceItems.end(); )
        {
            QChar oldChar = oldIt.value()->data(0).toChar(); 
            if (oldChar == pieceChar) 
            {
                pieceItem = oldIt.value();
                oldIt = oldPieceItems.erase(oldIt); 
                break;
            }
            else
            {
                ++oldIt;
            }
        }
        
        // b) 駒アイテムの配置・アニメーション
        if (pieceItem)
        {
            pieceItem->setBrush(pieceChar.isUpper() ? Qt::white : Qt::black);

            // ★ 移動アニメーション ★ (QGraphicsItemはQObjectではないため QVariantAnimationを使用)
            QPointF startPos = pieceItem->pos();
            
            if (startPos != targetCenterPos) 
            {
                QVariantAnimation *animation = new QVariantAnimation(this);
                animation->setDuration(300); 
                animation->setStartValue(QVariant::fromValue(startPos));
                animation->setEndValue(QVariant::fromValue(targetCenterPos));
                animation->setEasingCurve(QEasingCurve::OutQuad);

                // ★修正点2: 第3引数に QObject を継承した this (Scene) を使用★
                QObject::connect(animation, &QVariantAnimation::valueChanged, 
                    this, [pieceItem](const QVariant &value){
                        pieceItem->setPos(value.toPointF());
                    });

                animation->start(QAbstractAnimation::DeleteWhenStopped); 
            }
            
            m_pieceItems[newCoord] = pieceItem;
        }
        else 
        {
            // ★ 新規配置 ★ (getPieceEmojiの引数としてQCharを渡す)
            pieceItem = new QGraphicsSimpleTextItem(getPieceEmoji(pieceChar));
            QFont font("Arial Unicode MS", SQUARE_SIZE * 0.7);
            pieceItem->setFont(font);
            pieceItem->setBrush(pieceChar.isUpper() ? Qt::white : Qt::black);
            pieceItem->setPen(Qt::NoPen);
            
            QRectF bounds = pieceItem->boundingRect();
            pieceItem->setTransform(QTransform::fromTranslate(-bounds.width() / 2, -bounds.height() / 2));
            
            pieceItem->setPos(targetCenterPos);
            addItem(pieceItem);

            pieceItem->setData(0, pieceChar);
            
            m_pieceItems[newCoord] = pieceItem;
        }
    }

    // 3. 除去された駒の処理 (キャプチャなど)
    for (QGraphicsItem *item : oldPieceItems)
    {
        removeItem(item);
        delete item;
    }
}



// 合法手リストを受け取り、ハイライトを描画/更新する
void ChessBoardScene::handleLegalMoves(const std::vector<std::pair<int, int>> &legalCells)
{
    // 1. 既存のハイライトアイテムを全て削除
    for (QGraphicsItem *item : m_highlightItems)
    {
        removeItem(item);
        delete item;
    }
    m_highlightItems.clear();

    // 2. 新しい合法手に基づいてハイライトを追加
    if (legalCells.empty()) {
        return;
    }

    QColor highlightColor(0, 255, 0, 80); 
    QBrush highlightBrush(highlightColor);
    
    for (const auto &cell : legalCells)
    {
        int rank = cell.first;
        int file = cell.second;
        
        QRectF rect = getSquareRect(rank, file);
        
        qreal dotSize = SQUARE_SIZE * 0.3;
        qreal offset = (SQUARE_SIZE - dotSize) / 2.0;

        QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(
            rect.x() + offset, 
            rect.y() + offset, 
            dotSize, 
            dotSize);
        
        dot->setBrush(highlightBrush);
        dot->setPen(Qt::NoPen);
        
        dot->setZValue(-1); 
        
        addItem(dot);
        m_highlightItems.append(dot);
    }
}