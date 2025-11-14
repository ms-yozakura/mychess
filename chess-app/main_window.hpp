

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <vector>
#include <utility>
#include "chess/types.hpp"

class ChessGame;
class ChessBoardWidget;
struct Move;

class MainWindow : public QMainWindow
{

Q_OBJECT // Qt のメタオブジェクトシステムを有効にする
    public :
    // コンストラクタ: ChessGameとSerialManagerのポインタを受け取る
    explicit MainWindow(ChessGame *game, QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    /**
     *  @brief ChessBoardWidgetのセルクリックシグナルを受け取るためのスロット
     * @param algebraicCooed 'a2' 'f5'などセルを表す文字列
     **/
    void handleSquareClick(const QString &algebraicCoord);

signals: /**
          * @brief ChessBoardWidgetに合法手のリストを伝えるためのシグナル
          * @param legalCells 合法な移動先のマス目のリスト。要素は {rank, file} (0-7) 形式。
          */
    void updateLegalMoves(const std::vector<std::pair<int, int>> &legalCells);

private:
    ChessGame *m_game;

    // UIエレメント
    QLabel *m_label;
    ChessBoardWidget *m_boardWidget;
    QString s_selectedSquare;

    bool m_turnWhite = true;

    std::vector<Move> currentLegalMoves;

    void setupUI();

    void setupConnections();
};