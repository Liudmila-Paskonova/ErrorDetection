#include "visualizer/Marker.h"

marker::Marker::Marker(const std::string &stmt, const std::string &dir,
                       const std::vector<std::pair<std::string, std::string>> &vec,
                       const std::vector<std::set<size_t>> &errors, const std::string &out, QWidget *parent)
    : QMainWindow(parent), statement(stmt), directory(dir), subPairs(vec), errorLines(errors), outFile(out), curID(0)
{
    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Header (problem statement)
    probStmt = new QLabel(this);
    auto stmtUpd = std::format("{} / {}\n{}", curID + 1, subPairs.size(), statement);
    probStmt->setText(stmtUpd.c_str());
    probStmt->setAlignment(Qt::AlignCenter);
    probStmt->setStyleSheet("font-size: 20px; font-weight: bold; color: black; padding: 10px; background-color: white");

    // "Next" button
    footerButton = new QPushButton("Next", this);
    footerButton->setStyleSheet(
        "font-size: 14px; color: black; background-color: #ABCDEF; border: none; padding: 10px;  border: none;");
    footerButton->setFixedHeight(50);
    connect(footerButton, &QPushButton::clicked, this, &marker::Marker::nextPair);

    // PT submission is placed on the left, OK - on the right
    std::ifstream okFile(directory + "/" + subPairs[curID].first);
    std::ifstream ptFile(directory + "/" + subPairs[curID].second);
    std::stringstream buf1, buf2;
    buf1 << okFile.rdbuf();
    okFile.close();
    buf2 << ptFile.rdbuf();
    ptFile.close();

    // Scrollable widget for the left part
    PTsnippet = new QScrollArea(this);
    PTsnippet->setWidgetResizable(true);
    // set PT part
    buttonContainer = new QWidget(this);
    buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    processLeftPart(buf2.str());
    PTsnippet->setWidget(buttonContainer);
    PTsnippet->setStyleSheet("background-color: #ffe0e0;");

    // set OK part
    OKsnippet = new QTextEdit(this);
    OKsnippet->setReadOnly(true);
    OKsnippet->setStyleSheet("text-align: left; background-color: #e0ffe0; color: black; font-size : 16px; "
                             "font-family: monospace;  padding: 0px; margin: 0px; border: none;");
    std::stringstream buffertemp;
    std::string line2;
    size_t k = 0;
    while (std::getline(buf1, line2, '\n')) {
        buffertemp << std::format("{:>5}  ", ++k) << line2 << '\n';
    }
    OKsnippet->setText(buffertemp.str().c_str());

    // Scroller for the stmt
    QScrollArea *SCroll = new QScrollArea(this);
    SCroll->setWidgetResizable(true);
    SCroll->setWidget(probStmt);
    SCroll->setMaximumSize(QSize(10000, 200));

    // Add scrolling for the left & right parts
    QSplitter *splitter = new QSplitter(this);
    splitter->addWidget(PTsnippet);
    splitter->addWidget(OKsnippet);

    // Construct the app
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(SCroll);
    layout->addWidget(splitter);
    layout->addWidget(footerButton);
}

void
marker::Marker::nextPair()
{
    outFile << subPairs[curID].second;
    if (pressedLines.empty()) {
        pressedLines.insert(errorLines[curID].begin(), errorLines[curID].end());
    }
    for (auto &u : pressedLines) {
        outFile << " " << u;
    }
    outFile << "\n";
    pressedLines.clear();

    if (subPairs.empty()) {
        return;
    }

    // Increment the index
    curID = (curID + 1) % subPairs.size();

    auto stmtUpd = std::format("{} / {}\n{}", curID + 1, subPairs.size(), statement);
    probStmt->setText(stmtUpd.c_str());

    // At the last iteration change name of the button
    if (curID == subPairs.size() - 1) {
        footerButton->setText("Done");
        footerButton->setStyleSheet(
            "font-size: 14px; color: black; background-color: #42AAFF; border: none; padding: 10px;  border: none;");
    }

    if (curID == 0) {
        QApplication::quit();
        return;
    }

    // PT submission is placed on the left, OK - on the right
    std::ifstream okFile(directory + "/" + subPairs[curID].first);
    std::ifstream ptFile(directory + "/" + subPairs[curID].second);
    std::stringstream buf1, buf2;
    buf1 << okFile.rdbuf();
    okFile.close();
    buf2 << ptFile.rdbuf();
    ptFile.close();

    std::stringstream buffertemp;
    std::string line2;
    size_t k = 0;
    while (std::getline(buf1, line2, '\n')) {
        buffertemp << std::format("{:>5}  ", ++k) << line2 << '\n';
    }
    // Update PT snippet
    processLeftPart(buf2.str());
    // Update OK snippet
    OKsnippet->setText(buffertemp.str().c_str());
}

marker::Marker::~Marker()
{

    delete probStmt;
    delete footerButton;
    delete OKsnippet;

    QLayoutItem *child;
    while ((child = buttonLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    outFile.close();
}

void
marker::Marker::processLeftPart(const std::string &text)
{
    std::string defaultStyle = "   text-align: left; "
                               "   font-family: monospace; "
                               "   background-color: #ffe0e0; "
                               "   color: black; "
                               "   font-size: 16px; "
                               "   padding: 0px; "
                               "   margin: 0px; ";

    std::string pressedStyle = "   text-align: left; "
                               "   font-family: monospace; "
                               "   background-color: red; "
                               "   color: black; "
                               "   font-size: 16px; "
                               "   padding: 0px; "
                               "   margin: 0px; ";

    // Freeing memory for previous buttons entities
    QLayoutItem *child;
    while ((child = buttonLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    std::stringstream buffer(text);
    std::string line;
    // Each line is a button
    size_t k = 0;
    auto vec = errorLines[curID];
    while (std::getline(buffer, line)) {
        std::string border;
        if (vec.find(k) != vec.end()) {
            border = " border: 1px solid red; ";
        } else {
            border = " border : none; ";
        }

        QPushButton *button =
            new QPushButton(QString::fromStdString((std::format("{:>5}  ", ++k) + line)), buttonContainer);
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        button->setMinimumHeight(22);
        button->setStyleSheet(QString::fromStdString(defaultStyle + border));
        buttonLayout->addWidget(button);

        // Change color the button's color to red when pressed
        QObject::connect(button, &QPushButton::clicked, [k, button, defaultStyle, pressedStyle, border, this]() {
            if (button->styleSheet().contains("background-color: red")) {
                button->setStyleSheet(QString::fromStdString(defaultStyle + border));
                if (pressedLines.find(k - 1) != pressedLines.end()) {
                    pressedLines.erase(k - 1);
                }
            } else {
                button->setStyleSheet(QString::fromStdString(pressedStyle + border));
                pressedLines.insert(k - 1);
            }
        });
    }

    // Refresh the button container
    buttonContainer->setLayout(buttonLayout);
}
