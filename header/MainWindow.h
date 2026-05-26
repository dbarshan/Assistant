#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QListWidgetItem>
#include <QVector>
#include <QUrl>
#include <QCloseEvent>
#include <QSystemTrayIcon>

// Forward declaration of the generated UI class
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// --- Struct to represent an active chat conversation/activity ---
struct HistoryItem {
    QString assistant;  // "gemini", "chatgpt", or "grok"
    QString title;      // The current page title
    QUrl url;           // The exact URL of the chat thread
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void showGemini();
    void showChatGPT();
    void showGrok();
    void showInfo();
    void updateNavigationState();
    
    // UI Redesign Slots
    void handleHistoryClick(QListWidgetItem *item);
    void recordHistory(const QUrl &url);
    void recordHistoryTitle(const QString &title);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void quitApplication();
    void restoreWindow();

private:
    void setupUI();
    void setupWebViews();
    void applyTheme();
    QWebEngineView* currentWebView() const;
    void setupSystemTray();

    // Generated UI pointer
    Ui::MainWindow *ui;

    // History log memory
    QVector<HistoryItem> historyData;
    int activeHistoryIndex; // Index in historyData for the currently loading chat

    // Web Views
    QWebEngineView *viewGemini;
    QWebEngineView *viewChatGPT;
    QWebEngineView *viewGrok;

    // Persistent shared profile for Single Sign-On (SSO)
    QWebEngineProfile *sharedProfile;

    // System Tray Integration
    QSystemTrayIcon *trayIcon;
    bool isQuitting;

    // State
    bool isDarkTheme;
    QString currentAssistant;
    QPoint dragPosition;
};

#endif // MAINWINDOW_H
