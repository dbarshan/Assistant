#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDesktopServices>
#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QUrl>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QWebEngineHistory>

// --- Dynamic High-DPI Vector Icon Painters for Window Controls ---
static QIcon createMinimizeIcon(const QColor &color) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 1.5, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);
    painter.drawLine(3, 8, 13, 8);
    return QIcon(pixmap);
}

static QIcon createMaximizeIcon(const QColor &color) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    
    // Draw sleek minimalist fullscreen corners
    // Top-left
    painter.drawLine(3, 6, 3, 3);
    painter.drawLine(3, 3, 6, 3);
    // Top-right
    painter.drawLine(10, 3, 13, 3);
    painter.drawLine(13, 3, 13, 6);
    // Bottom-right
    painter.drawLine(13, 10, 13, 13);
    painter.drawLine(13, 13, 10, 13);
    // Bottom-left
    painter.drawLine(6, 13, 3, 13);
    painter.drawLine(3, 13, 3, 10);
    
    return QIcon(pixmap);
}

static QIcon createCloseIcon(const QColor &color) {
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 1.5, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(pen);
    painter.drawLine(4, 4, 12, 12);
    painter.drawLine(12, 4, 4, 12);
    return QIcon(pixmap);
}

// --- Custom WebEngine Page for Security & External Link Redirection ---
class CustomWebPage : public QWebEnginePage {
public:
    CustomWebPage(QWebEngineProfile *profile, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent) {}

protected:
    // Intercept link clicks and open external URLs in the default system browser
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override {
        if (type == NavigationTypeLinkClicked && isMainFrame) {
            QString host = url.host().toLower();
            // List of hosts allowed to load internally (includes login and auth redirects)
            bool isAllowed = host.contains("gemini.google.com") ||
                             host.contains("chatgpt.com") ||
                             host.contains("grok.com") ||
                             host.contains("x.com") ||
                             host.contains("x.ai") ||          // <-- ADDED for Grok centralized accounts & auth
                             host.contains("twitter.com") ||
                             host.contains("google.") ||
                             host.contains("openai.com") ||
                             host.contains("appleid.apple.com") ||
                             host.contains("auth0.openai.com") ||
                             host.contains("auth0.com");

            if (!isAllowed) {
                QDesktopServices::openUrl(url);
                return false; // Deny loading inside the app's webview
            }
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

    // Intercept window.open requests (target="_blank") and route them directly inside
    // the current view to allow seamless popup-based OAuth login flows (like Grok's Google login)
    QWebEnginePage* createWindow(QWebEnginePage::WebWindowType type) override {
        Q_UNUSED(type);
        return this; // Return the current page to force-load the popup inside this view!
    }

    // Intercept and suppress all JavaScript console logs from polluting the standard output
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) override {
        Q_UNUSED(level);
        Q_UNUSED(message);
        Q_UNUSED(lineNumber);
        Q_UNUSED(sourceID);
        // Suppressed to keep the desktop application terminal output perfectly clean!
    }
};

// --- MainWindow Implementation ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), activeHistoryIndex(-1), isQuitting(false), isDarkTheme(true), currentAssistant("gemini") {
    setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);
    setupUI();
    setupWebViews();
    applyTheme();
    setupSystemTray();
    
    // Sleek normal floating window size
    resize(1200, 800);
    showNormal();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (isQuitting) {
        event->accept(); // Exit application completely
    } else {
        event->ignore(); // Ignore close to keep app running
        this->hide();    // Send window to system tray
    }
}

void MainWindow::setupUI() {
    // Load Brand Logo Icon into the pre-defined label
    ui->brandIcon->setPixmap(QPixmap(":/resources/icon.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Connect standard signals
    connect(ui->btnQuit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
    connect(ui->historyList, &QListWidget::itemClicked, this, &MainWindow::handleHistoryClick);

    // --- UI REDESIGN: Hide legacy sidebar and show stackedWidget ---
    ui->sidebar->hide();
    ui->stackedWidget->show();

    // Remove the default horizontal layout of centralWidget
    if (ui->centralWidget->layout()) {
        delete ui->centralWidget->layout();
    }

    // Create vertical layout for central widget
    QVBoxLayout *mainVLayout = new QVBoxLayout(ui->centralWidget);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    // Create the top bar widget and style it via QSS
    QWidget *topBar = new QWidget(ui->centralWidget);
    topBar->setObjectName("topBar");

    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 0, 20, 0);
    topBarLayout->setSpacing(4);
    topBarLayout->setAlignment(Qt::AlignVCenter);

    // Hide components we no longer show
    ui->brandTitle->hide();
    ui->navSection->hide();
    ui->btnSettings->hide();
    ui->btnQuit->hide();

    // Create a flat button for the "Assistant" dropdown menu
    QPushButton *btnAssistant = new QPushButton("Assistant", topBar);
    btnAssistant->setObjectName("btnAssistantMenu");

    QMenu *assistantMenu = new QMenu(btnAssistant);
    assistantMenu->setObjectName("assistantMenu");

    QAction *actionGemini = assistantMenu->addAction("Google Gemini");
    QAction *actionChatGPT = assistantMenu->addAction("OpenAI ChatGPT");
    QAction *actionGrok = assistantMenu->addAction("xAI Grok");

    connect(actionGemini, &QAction::triggered, this, &MainWindow::showGemini);
    connect(actionChatGPT, &QAction::triggered, this, &MainWindow::showChatGPT);
    connect(actionGrok, &QAction::triggered, this, &MainWindow::showGrok);

    btnAssistant->setMenu(assistantMenu);

    // Left: Brand Logo + Assistant Dropdown Button
    topBarLayout->addWidget(ui->brandIcon);
    topBarLayout->addWidget(btnAssistant);

    // Stretch spacer to push window controls to the far right
    topBarLayout->addStretch(1);

    // Create custom window controls
    QPushButton *btnMinimize = new QPushButton(topBar);
    QPushButton *btnMaximize = new QPushButton(topBar);
    QPushButton *btnClose = new QPushButton(topBar);

    btnMinimize->setObjectName("btnWindowMinimize");
    btnMaximize->setObjectName("btnWindowMaximize");
    btnClose->setObjectName("btnWindowClose");

    btnMinimize->setIconSize(QSize(16, 16));
    btnMaximize->setIconSize(QSize(16, 16));
    btnClose->setIconSize(QSize(16, 16));

    btnMinimize->setToolTip("Minimize");
    btnMaximize->setToolTip("Maximize / Restore");
    btnClose->setToolTip("Close");

    topBarLayout->addWidget(btnMinimize);
    topBarLayout->addWidget(btnMaximize);
    topBarLayout->addWidget(btnClose);

    connect(btnMinimize, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(btnMaximize, &QPushButton::clicked, this, [this]() {
        if (this->isFullScreen()) {
            this->showNormal();
        } else {
            this->showFullScreen();
        }
    });
    connect(btnClose, &QPushButton::clicked, this, &MainWindow::close);

    // Put topBar and stackedWidget into the main vertical layout
    mainVLayout->addWidget(topBar);
    mainVLayout->addWidget(ui->stackedWidget);
}

void MainWindow::setupWebViews() {
    // Configure Persistent Storage Paths inside Workspace for portability
    QString baseDir = QDir::currentPath() + "/.qt_profiles";
    
    // Use a modern Firefox on Linux User Agent to bypass secure browser check issues
    QString firefoxUA = "Mozilla/5.0 (X11; Linux x86_64; rv:133.0) Gecko/20100101 Firefox/133.0";
    QWebEngineProfile::defaultProfile()->setHttpUserAgent(firefoxUA);
    
    // Create a single shared profile to enable True Single Sign-On (SSO) across all AI assistants
    sharedProfile = new QWebEngineProfile("persist_shared", this);
    sharedProfile->setPersistentStoragePath(baseDir + "/shared");
    sharedProfile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    sharedProfile->setHttpUserAgent(firefoxUA);

    // 1. Google Gemini Page & View
    viewGemini = new QWebEngineView(ui->stackedWidget);
    viewGemini->setPage(new CustomWebPage(sharedProfile, viewGemini));
    viewGemini->setUrl(QUrl("https://gemini.google.com/"));

    // 2. OpenAI ChatGPT Page & View
    viewChatGPT = new QWebEngineView(ui->stackedWidget);
    viewChatGPT->setPage(new CustomWebPage(sharedProfile, viewChatGPT));
    viewChatGPT->setUrl(QUrl("https://chatgpt.com/"));

    // 3. xAI Grok Page & View
    viewGrok = new QWebEngineView(ui->stackedWidget);
    viewGrok->setPage(new CustomWebPage(sharedProfile, viewGrok));
    viewGrok->setUrl(QUrl("https://grok.com/"));

    // Add WebViews to Stacked Widget
    ui->stackedWidget->addWidget(viewGemini);
    ui->stackedWidget->addWidget(viewChatGPT);
    ui->stackedWidget->addWidget(viewGrok);

    // Initial Active Webview
    ui->stackedWidget->setCurrentWidget(viewGemini);

    // Connect page change and page load finished signals to refresh navigation buttons
    auto hookupSignals = [this](QWebEngineView* view) {
        connect(view, &QWebEngineView::loadFinished, this, &MainWindow::updateNavigationState);
        connect(view->page(), &QWebEnginePage::urlChanged, this, &MainWindow::updateNavigationState);
        
        // Track URLs and Titles for dynamic History logging
        connect(view, &QWebEngineView::urlChanged, this, &MainWindow::recordHistory);
        connect(view, &QWebEngineView::titleChanged, this, &MainWindow::recordHistoryTitle);
    };

    hookupSignals(viewGemini);
    hookupSignals(viewChatGPT);
    hookupSignals(viewGrok);

    updateNavigationState();
}

QWebEngineView* MainWindow::currentWebView() const {
    QWidget* current = ui->stackedWidget->currentWidget();
    return qobject_cast<QWebEngineView*>(current);
}

void MainWindow::updateNavigationState() {
    QWebEngineView* active = currentWebView();
    if (!active) return;

    bool canBack = active->history()->canGoBack();
    bool canForward = active->history()->canGoForward();

    ui->btnBack->setEnabled(canBack);
    ui->btnForward->setEnabled(canForward);

    ui->btnBack->setProperty("disabledState", !canBack);
    ui->btnForward->setProperty("disabledState", !canForward);

    ui->btnBack->style()->unpolish(ui->btnBack);
    ui->btnBack->style()->polish(ui->btnBack);
    ui->btnForward->style()->unpolish(ui->btnForward);
    ui->btnForward->style()->polish(ui->btnForward);
}

// --- Dynamic Chat History & Activity Logger ---

void MainWindow::recordHistory(const QUrl &url) {
    QString host = url.host().toLower();
    QString path = url.path().toLower();
    
    // We only log URLs representing active chats, ignoring logins and static landings
    bool isChat = false;
    if (currentAssistant == "gemini" && host.contains("gemini.google.com") && (path.contains("/app") || path.contains("/chat"))) {
        isChat = true;
    } else if (currentAssistant == "chatgpt" && host.contains("chatgpt.com") && path.contains("/c/")) {
        isChat = true;
    } else if (currentAssistant == "grok" && host.contains("grok.com") && (path.contains("/chat") || path.length() > 5)) {
        isChat = true;
    }

    if (!isChat) {
        activeHistoryIndex = -1;
        return;
    }

    // Check if the chat URL already exists in our history vector
    int foundIndex = -1;
    for (int i = 0; i < historyData.size(); ++i) {
        if (historyData[i].url == url) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        // Move existing chat to top
        HistoryItem matchedItem = historyData.takeAt(foundIndex);
        historyData.prepend(matchedItem);
        activeHistoryIndex = 0;

        // Sync with UI List
        QListWidgetItem *listItem = ui->historyList->takeItem(foundIndex);
        ui->historyList->insertItem(0, listItem);
    } else {
        // Create new history item
        HistoryItem newItem;
        newItem.assistant = currentAssistant;
        newItem.url = url;
        newItem.title = "New Activity"; // Temporary title, will get updated via recordHistoryTitle
        
        historyData.prepend(newItem);
        activeHistoryIndex = 0;

        // Add formatted row to list
        QString brandLabel = (currentAssistant == "gemini") ? "Gemini" : ((currentAssistant == "chatgpt") ? "ChatGPT" : "Grok");
        QListWidgetItem *newListItem = new QListWidgetItem(QString("%1 • %2").arg(brandLabel, newItem.title));
        
        // Setup icons dynamically in the row data
        newListItem->setData(Qt::UserRole, currentAssistant);
        ui->historyList->insertItem(0, newListItem);
    }
    
    ui->historyList->setCurrentRow(0);
}

void MainWindow::recordHistoryTitle(const QString &title) {
    if (activeHistoryIndex == -1 || activeHistoryIndex >= historyData.size()) return;
    
    // Skip empty, default, or loading titles
    QString cleanTitle = title.trimmed();
    if (cleanTitle.isEmpty() || cleanTitle.toLower() == "gemini" || 
        cleanTitle.toLower() == "chatgpt" || cleanTitle.toLower() == "grok" ||
        cleanTitle.contains("Loading") || cleanTitle.length() < 3) {
        return;
    }

    // Save updated title
    historyData[activeHistoryIndex].title = cleanTitle;

    // Refresh text label in the sidebar list
    QString brandLabel = (historyData[activeHistoryIndex].assistant == "gemini") ? "Gemini" : 
                         ((historyData[activeHistoryIndex].assistant == "chatgpt") ? "ChatGPT" : "Grok");
    
    QListWidgetItem *item = ui->historyList->item(activeHistoryIndex);
    if (item) {
        item->setText(QString("%1 • %2").arg(brandLabel, cleanTitle));
    }
}

void MainWindow::handleHistoryClick(QListWidgetItem *item) {
    int row = ui->historyList->row(item);
    if (row < 0 || row >= historyData.size()) return;

    HistoryItem clickedItem = historyData[row];
    
    // Switch Active Assistant
    if (clickedItem.assistant == "gemini") {
        showGemini();
    } else if (clickedItem.assistant == "chatgpt") {
        showChatGPT();
    } else if (clickedItem.assistant == "grok") {
        showGrok();
    }

    // Force load the exact chat URL
    QWebEngineView* active = currentWebView();
    if (active && active->url() != clickedItem.url) {
        active->load(clickedItem.url);
    }
}

// --- Active Assistant View Swapping slots ---

void MainWindow::showGemini() {
    ui->stackedWidget->setCurrentWidget(viewGemini);
    currentAssistant = "gemini";
    updateNavigationState();
}

void MainWindow::showChatGPT() {
    ui->stackedWidget->setCurrentWidget(viewChatGPT);
    currentAssistant = "chatgpt";
    updateNavigationState();
}

void MainWindow::showGrok() {
    ui->stackedWidget->setCurrentWidget(viewGrok);
    currentAssistant = "grok";
    updateNavigationState();
}

// --- Dark/Light Theme Swapping ---

void MainWindow::applyTheme() {
    QFile file(":/resources/stylesheet.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString fullQss = stream.readAll();
        file.close();

        QString themeSelector = isDarkTheme ? "dark-theme" : "light-theme";
        this->setProperty("theme", themeSelector);
        
        this->style()->unpolish(this);
        this->style()->polish(this);
        
        // Propagate down to UI base layout
        ui->centralWidget->setProperty("theme", themeSelector);
        ui->centralWidget->style()->unpolish(ui->centralWidget);
        ui->centralWidget->style()->polish(ui->centralWidget);

        this->setStyleSheet(fullQss);

        // Update window control icons dynamically to match the active theme's colors
        QPushButton *btnMin = findChild<QPushButton*>("btnWindowMinimize");
        QPushButton *btnMax = findChild<QPushButton*>("btnWindowMaximize");
        QPushButton *btnCls = findChild<QPushButton*>("btnWindowClose");

        QColor iconColor = isDarkTheme ? QColor("#9ca3af") : QColor("#4b5563");
        if (btnMin) btnMin->setIcon(createMinimizeIcon(iconColor));
        if (btnMax) btnMax->setIcon(createMaximizeIcon(iconColor));
        if (btnCls) btnCls->setIcon(createCloseIcon(iconColor));
    } else {
        qWarning() << "Could not open resources/stylesheet.qss from resource system";
    }
}

// --- Application Information Modal ---

void MainWindow::showInfo() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About Assistant");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("<h3>Assistant Application</h3>"
                   "<p>Assistant is a secure, native Qt6 C++ application powered by Chromium "
                   "(Qt WebEngine) developed for unified multitasking with AI systems.</p>");
    msgBox.exec();
}

// --- Frameless Window Mouse Dragging Handlers ---
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Only allow dragging via the topBar widget or its non-button children
        QWidget *child = childAt(event->position().toPoint());
        bool isDraggingArea = false;
        
        QWidget *p = child;
        while (p) {
            if (p->objectName() == "topBar") {
                isDraggingArea = true;
                break;
            }
            p = p->parentWidget();
        }
        
        // Prevent dragging when clicking interactive buttons
        if (isDraggingArea && child && qobject_cast<QPushButton*>(child)) {
            isDraggingArea = false;
        }

        if (isDraggingArea) {
            dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    dragPosition = QPoint();
}

// --- System Tray Implementation ---
void MainWindow::setupSystemTray() {
    // 1. Create System Tray Icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/resources/icon.png"));
    trayIcon->setToolTip("Assistant");

    // 2. Create Tray Context Menu
    QMenu *trayMenu = new QMenu(this);
    trayMenu->setObjectName("trayMenu");

    QAction *restoreAction = trayMenu->addAction("Open Assistant");
    trayMenu->addSeparator();
    QAction *quitAction = trayMenu->addAction("Quit");

    connect(restoreAction, &QAction::triggered, this, &MainWindow::restoreWindow);
    connect(quitAction, &QAction::triggered, this, &MainWindow::quitApplication);

    trayIcon->setContextMenu(trayMenu);

    // 3. Connect activation signal (e.g. left click or double click to restore)
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);

    // 4. Show Tray Icon
    trayIcon->show();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        restoreWindow();
    }
}

void MainWindow::restoreWindow() {
    this->showNormal();
    this->activateWindow();
    this->raise();
}

void MainWindow::quitApplication() {
    isQuitting = true;
    this->close();
    QCoreApplication::quit();
}
