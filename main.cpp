#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QFileDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollBar>

class LogViewer : public QWidget {
    Q_OBJECT

public:
    LogViewer(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        setupFileWatcher();
        resize(800, 600);
        setWindowTitle("LogMonitor");
    }

private slots:
    void updateLog() {
        if (!currentFilePath.isEmpty()) {
            QFile file(currentFilePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString filterText = filterLineEdit->text().trimmed();
                QString logContent;

                QScrollBar *scrollBar = logText->verticalScrollBar();
                int scrollValue = scrollBar->value();

                while (!in.atEnd()) {
                    QString line = in.readLine();
                    if (filterText.isEmpty() || line.contains(filterText, Qt::CaseInsensitive)) {
                        logContent.append(line + "\n");
                    }
                }
                logText->setPlainText(logContent);
                file.close();

                if(isScrollToBottom) {
                    scrollToBottomButton->setStyleSheet("background-color: lightblue;");
                    scrollBar->setValue(scrollBar->maximum());
                }
                else {
                    scrollToBottomButton->setStyleSheet("background-color: white;");
                    scrollBar->setValue(scrollValue);
                }
            }
        }
    }

    void scrollToBottom() {
        isScrollToBottom = !isScrollToBottom;
    }

    void chooseLogFile() {
        QString selectedFilePath = QFileDialog::getOpenFileName(this, tr("Choose Log File"), "", tr("Log Files (*.log);;All Files (*)"));
        if (!selectedFilePath.isEmpty()) {
            currentFilePath = selectedFilePath;
            updateLog();
        }
    }

private:
    void setupUI() {
        QVBoxLayout *layout = new QVBoxLayout;

        logText = new QTextEdit;
        layout->addWidget(logText);

        filterLineEdit = new QLineEdit;
        filterLineEdit->setPlaceholderText("请输入过滤条件...");
        filterLineEdit->setFixedHeight(30);
        layout->addWidget(filterLineEdit);

        selectFileButton = new QPushButton(tr("打开文件"));
        selectFileButton->setStyleSheet("background-color: white;");
        connect(selectFileButton, SIGNAL(clicked()), this, SLOT(chooseLogFile()));
        layout->addWidget(selectFileButton);

        scrollToBottomButton = new QPushButton(tr("滚动到底部"));
        scrollToBottomButton->setStyleSheet("background-color: white;");
        connect(scrollToBottomButton, SIGNAL(clicked()), this, SLOT(scrollToBottom()));
        layout->addWidget(scrollToBottomButton);

        setLayout(layout);
    }

    void setupFileWatcher() {
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateLog()));
        timer->start(100);
    }

    QPushButton *selectFileButton;
    QPushButton *scrollToBottomButton;
    QTextEdit *logText;
    QLineEdit *filterLineEdit;
    QString currentFilePath;
    bool isScrollToBottom = true;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    LogViewer viewer;
    viewer.show();

    return app.exec();
}

#include "main.moc"
