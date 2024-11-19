#ifndef VISUALIZER_MARKER_H
#define VISUALIZER_MARKER_H

#include <QMainWindow>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <QTextBrowser>
#include <QScrollArea>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <set>

namespace marker
{
class Marker : public QMainWindow
{
    Q_OBJECT

  public:
    explicit Marker(const std::string &stmt, const std::string &dir,
                    const std::vector<std::pair<std::string, std::string>> &vec,
                    const std::vector<std::set<size_t>> &errors, const std::string &out, QWidget *parent = nullptr);
    ~Marker();

  private:
    void nextPair();
    void processLeftPart(const std::string &text);

    QScrollArea *SCroll;
    QScrollArea *PTsnippet;
    QWidget *buttonContainer;
    QVBoxLayout *buttonLayout;
    QTextEdit *OKsnippet;
    QPushButton *footerButton;

    std::string statement;
    std::string directory;
    std::vector<std::pair<std::string, std::string>> subPairs;
    std::vector<std::set<size_t>> errorLines;
    std::ofstream outFile;
    size_t curID;
    std::set<size_t> pressedLines;
};
} // namespace marker

#endif
