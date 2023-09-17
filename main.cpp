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
#include <QThread>

class LogLoader : public QObject {
    Q_OBJECT

public:
    LogLoader(QString filePath, QString filterText, QTextEdit* logText, bool isScrollToBottom) :
        filePath(filePath), filterText(filterText), logText(logText), isScrollToBottom(isScrollToBottom) {}

public slots:
    void loadLog() {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString logContent;
            QList<QString> lastLines;
            int lineCount = 100;

            while (!in.atEnd()) {
                QString line = in.readLine();
                if (filterText.isEmpty() || line.contains(filterText, Qt::CaseInsensitive)) {
                    lastLines.append(line);
                    if (lastLines.size() > lineCount) {
                        lastLines.pop_front();
                    }
                }
            }
            file.close();
            for (const QString& line : lastLines) {
                logContent.append(line + "\n");
            }
            emit logLoaded(logContent, isScrollToBottom);
        }
    }

signals:
    void logLoaded(const QString& logContent, bool isScrollToBottom);

private:
    QString filePath;
    QString filterText;
    QTextEdit* logText;
    bool isScrollToBottom;
};


class LogViewer : public QWidget {
    Q_OBJECT

public:
    LogViewer(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        setupFileWatcher();
        resize(1280, 1000);
        setWindowTitle("LogMonitor");
    }

private slots:
    void updateLog() {
        if (!currentFilePath.isEmpty()) {
            LogLoader* loader = new LogLoader(currentFilePath, filterLineEdit->text().trimmed(), logText, isScrollToBottom);
            connect(loader, SIGNAL(logLoaded(QString,bool)), this, SLOT(handleLogUpdate(QString,bool)));
            QThread* thread = new QThread;
            connect(thread, SIGNAL(started()), loader, SLOT(loadLog()));
            loader->moveToThread(thread);
            thread->start();
        }
    }

    void handleLogUpdate(const QString& logContent, bool scrollToBottom) {
        QScrollBar* scrollBar = logText->verticalScrollBar();
        int scrollValue = scrollBar->value();
        logText->setPlainText(logContent);

        if (scrollToBottom) {
            scrollToBottomButton->setStyleSheet("background-color: lightblue;");
            scrollBar->setValue(scrollBar->maximum());
        } else {
            scrollToBottomButton->setStyleSheet("background-color: white;");
            scrollBar->setValue(scrollValue);
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
        timer->start(500);
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
