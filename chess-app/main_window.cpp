// mainwindow.cpp

#include "main_window.hpp"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include "chess/chess_game.hpp" // ChessGame
// #include "widget/chess_board_widget.hpp" // ChessBoardWidget
#include "widget/chess_board_view.hpp" // 新しいヘッダをインクルード
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>

// コンストラクタ
MainWindow::MainWindow(ChessGame *game, QWidget *parent)
    // m_fd の代わりに m_serialManager を初期化
    : QMainWindow(parent), m_game(game)
{
    setWindowTitle("Chess");
    setupUI();
    setupConnections();
    // m_boardWidget->setBoardFromFEN(m_game->getBoardStateFEN(false));
    m_boardView->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (新)
}
void MainWindow::setupUI()
{
    // 中央ウィジェット
    QWidget *central = new QWidget;
    setCentralWidget(central);

    // スタイルシートファイルのパスを指定
    QFile file("./css/style.css");
    // ファイルを開く
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        file.close();
        central->setStyleSheet(styleSheet);
    }

    // UI要素の初期化
    m_label = new QLabel("Hello World");

    // 盤面: 新しい ChessBoardView を初期化
    m_boardView = new ChessBoardView(this);

    // ★ 修正後のレイアウト設定 ★
    // layout1 (盤面 + 情報パネル)
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->setContentsMargins(20, 20, 20, 20);
    layout1->setSpacing(15);

    // layout2 (情報パネル: タイトル、ログなど)
    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->setContentsMargins(20, 20, 20, 20);
    layout2->setSpacing(15);

    // 情報パネルの内容を追加
    layout2->addWidget(m_label);
    // ... (他の情報ウィジェットがあればここに追加) ...

    // メインレイアウトに盤面と情報パネルを追加
    layout1->addWidget(m_boardView); // ★ m_boardView をレイアウトに追加 ★
    layout1->addLayout(layout2);

    // 中央ウィジェットに最終レイアウトを設定
    central->setLayout(layout1);
}

// シグナルとスロットの接続
void MainWindow::setupConnections()
{
    // モーター制御ボタン (スロットはそのまま)
    QObject::connect(m_boardView, &ChessBoardView::squareClicked, // (新) m_boardViewを使用
                     this, &MainWindow::handleSquareClick);
    // 合法手情報をBoardWidgetに渡す
    QObject::connect(this, &MainWindow::updateLegalMoves,
                     m_boardView, &ChessBoardView::handleLegalMoves); // (新) m_boardViewを使用
}

void MainWindow::handleSquareClick(const QString &algebraicCoord)
{
    qDebug() << "クリックされました:" << algebraicCoord;
    if (m_turnWhite)
    {

        // 1. 既に駒が選択されている場合 (2回目のクリック)
        if (!s_selectedSquare.isEmpty())
        {
            QString moveStr = s_selectedSquare + algebraicCoord; // 例: "e2e4"

            // 実際には m_game->makeMove() を呼び出し、移動が合法かチェックする
            // if (m_game->makeMove(move.toStdString())) { ... }

            Move thisMove;
            m_game->algebraicToMove(moveStr.toStdString(), thisMove);

            if (m_game->isPromotionMove(thisMove))
            { // ★ 1. プロモーションチェック
                // QInputDialog や QMessageBox + ボタンなどで、昇格先の駒 (Q, R, B, N) を選択させる
                QString promotionResult = QInputDialog::getItem(
                    this,
                    tr("Promotion"),
                    tr("Select piece for promotion:"),
                    QStringList() << "Queen" << "Rook" << "Bishop" << "Knight",
                    0,    // 初期選択
                    false // 編集不可
                );

                if (promotionResult.isEmpty())
                {
                    // キャンセルされた場合: 何もせず選択をリセット
                    s_selectedSquare.clear();
                    emit updateLegalMoves({});
                    return;
                }

                // 2. 選択された駒を Move オブジェクトに追加
                char promotionChar;
                if (promotionResult == "Queen")
                    promotionChar = 'Q';
                else if (promotionResult == "Rook")
                    promotionChar = 'R';
                else if (promotionResult == "Bishop")
                    promotionChar = 'B';
                else if (promotionResult == "Knight")
                    promotionChar = 'N';
                else
                    promotionChar = '*';

                // Moveのプロモーションフィールドを更新（これは 'chess_game.hpp' の Move 構造体にフィールドが必要）
                thisMove.promotedTo = promotionChar; // ★ 3. Moveオブジェクトに昇格先を設定
            }
            Move determinedMove;

            bool islegal = false;
            for (const auto &move : currentLegalMoves)
            {
                if (move == thisMove)
                {
                    islegal = true;
                    determinedMove = move;
                }
            }

            //! プロモーションの処理必要

            if (islegal)
            { // 人間 (白) の手実行前
                qDebug() << "Before Human Move FEN:" << m_game->getBoardStateFEN(m_turnWhite);
                m_game->makeMove(determinedMove);
                m_turnWhite = !m_turnWhite;

                if (m_game->isEnd(m_turnWhite))
                {
                    // 移動後の盤面更新と、合法手リストのリセット
                    // m_boardWidget->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (旧)
                    m_boardView->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (新)

                    // 合法手リストを空にして、強調表示を消す
                    emit updateLegalMoves({});
                    currentLegalMoves = {};
                    s_selectedSquare.clear(); // 選択解除
                    QMessageBox::information(this, tr("You win"), tr("you are good chess player"));
                }

                // m_boardWidget->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (旧)
                m_boardView->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (新)

                std::cout << "AI (Black) is thinking...\n";

                // AI (黒) の手実行前
                qDebug() << "Before AI Move FEN:" << m_game->getBoardStateFEN(m_turnWhite);

                Move aiMove = m_game->bestMove(m_turnWhite);
                m_game->makeMove(aiMove);
                m_turnWhite = !m_turnWhite;

                if (m_game->isEnd(m_turnWhite))
                {
                    // 移動後の盤面更新と、合法手リストのリセット
                    // m_boardWidget->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (旧)
                    m_boardView->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (新)
                    // 合法手リストを空にして、強調表示を消す
                    emit updateLegalMoves({});
                    currentLegalMoves = {};
                    s_selectedSquare.clear(); // 選択解除
                    QMessageBox::information(this, tr("You lose"), tr("monkey"));
                }

                // AI (黒) の手実行後
                qDebug() << "After AI Move FEN:" << m_game->getBoardStateFEN(m_turnWhite);
            }

            // 移動後の盤面更新と、合法手リストのリセット
            // m_boardWidget->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (旧)
            m_boardView->setBoardFromFEN(m_game->getBoardStateFEN(false)); // (新)
            // 合法手リストを空にして、強調表示を消す
            emit updateLegalMoves({});
            currentLegalMoves = {};
            s_selectedSquare.clear(); // 選択解除
        }
        // 2. 駒が選択されていない場合 (1回目のクリック)
        else
        {
            // 選択されたマスに駒があるかどうかのチェックも必要ですが、ここではスキップ
            s_selectedSquare = algebraicCoord;
            qDebug() << "駒を選択:" << s_selectedSquare;

            int r;
            int c;
            m_game->algebraicToCoords(algebraicCoord.toStdString(), r, c);

            currentLegalMoves = m_game->generateMoves(true);

            std::vector<std::pair<int, int>> coords = {};

            for (const auto &move : currentLegalMoves)
            {
                if (move.from.first == r && move.from.second == c)
                    coords.push_back({move.to.first, move.to.second});
            }

            if (coords.empty())
                s_selectedSquare.clear(); // 選択解除

            qDebug() << "合法ムーブ" << coords;

            // 合法手リストをBoardWidgetに送信して描画させる
            emit updateLegalMoves(coords);
        }
    }
}
