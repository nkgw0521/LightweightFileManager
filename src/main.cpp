#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QSplitter>
#include <QTabBar>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QAbstractItemView>
#include <QLocale>
#include <QStatusBar>
#include <QStackedWidget>
#include <QToolButton>
#include <QStyle>
#include <QSettings>
#include <QInputDialog>
#include <QCloseEvent>
#include <QSignalBlocker>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QClipboard>
#include <QShortcut>
#include <QKeySequence>
#include <QSet>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProcess>
#include <QStandardPaths>
#include <QIcon>
#include <QColor>
#include <QPen>
#include <QRectF>
#include <QFont>
#include <QSizePolicy>
#include <QEvent>
#include <QScrollArea>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <functional>
#include <memory>
#include <vector>
#include <rust/cxx.h>
#include "fm-core/src/lib.rs.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
QString rustString(const rust::String& value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}

QString coreCreateDirectory(const QString& path) {
    const auto utf8 = path.toUtf8().toStdString();
    return rustString(create_directory(rust::Str(utf8.data(), utf8.size())));
}

QString coreRenamePath(const QString& from, const QString& to) {
    const auto fromUtf8 = from.toUtf8().toStdString();
    const auto toUtf8 = to.toUtf8().toStdString();
    return rustString(rename_path(rust::Str(fromUtf8.data(), fromUtf8.size()),
                                  rust::Str(toUtf8.data(), toUtf8.size())));
}

QString coreRemovePath(const QString& path) {
    const auto utf8 = path.toUtf8().toStdString();
    return rustString(remove_path(rust::Str(utf8.data(), utf8.size())));
}

QString coreCopyPath(const QString& from, const QString& to) {
    const auto fromUtf8 = from.toUtf8().toStdString();
    const auto toUtf8 = to.toUtf8().toStdString();
    return rustString(copy_path(rust::Str(fromUtf8.data(), fromUtf8.size()),
                                rust::Str(toUtf8.data(), toUtf8.size())));
}

QString coreMovePath(const QString& from, const QString& to) {
    const auto fromUtf8 = from.toUtf8().toStdString();
    const auto toUtf8 = to.toUtf8().toStdString();
    return rustString(move_path(rust::Str(fromUtf8.data(), fromUtf8.size()),
                                rust::Str(toUtf8.data(), toUtf8.size())));
}

QIcon applicationIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1b2532"));
    painter.drawRoundedRect(QRectF(2, 2, 60, 60), 14, 14);
    painter.setPen(QPen(QColor("#74a8f7"), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(QColor("#25384f"));
    painter.drawRoundedRect(QRectF(13, 15, 38, 34), 4, 4);
    painter.drawLine(QPointF(32, 16), QPointF(32, 48));
    painter.setPen(QPen(QColor("#9fc0ff"), 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(20, 27), QPointF(27, 27));
    painter.drawLine(QPointF(37, 27), QPointF(44, 27));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#f3bd58"));
    QPainterPath folder;
    folder.moveTo(16, 35);
    folder.lineTo(26, 35);
    folder.lineTo(29, 39);
    folder.lineTo(47, 39);
    folder.lineTo(45, 47);
    folder.lineTo(16, 47);
    folder.closeSubpath();
    painter.drawPath(folder);
    return QIcon(pixmap);
}

QIcon fileItemIcon(bool directory) {
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    if (directory) {
        QPainterPath folder;
        folder.moveTo(2, 5);
        folder.lineTo(8, 5);
        folder.lineTo(10, 8);
        folder.lineTo(18, 8);
        folder.lineTo(17, 17);
        folder.lineTo(2, 17);
        folder.closeSubpath();
        painter.fillPath(folder, QColor("#f3bd58"));
        painter.setPen(QPen(QColor("#ffe0a0"), 1));
        painter.drawLine(QPointF(3, 9), QPointF(17, 9));
    } else {
        QPainterPath document;
        document.moveTo(5, 2);
        document.lineTo(12, 2);
        document.lineTo(16, 6);
        document.lineTo(16, 18);
        document.lineTo(5, 18);
        document.closeSubpath();
        painter.fillPath(document, QColor("#8294ab"));
        painter.setPen(QPen(QColor("#dbe4ef"), 1));
        painter.drawLine(QPointF(8, 10), QPointF(13, 10));
        painter.drawLine(QPointF(8, 13), QPointF(13, 13));
    }
    return QIcon(pixmap);
}

QString terminalActionText() {
#if defined(Q_OS_WIN)
    return QObject::tr("ターミナルを開く");
#else
    return QObject::tr("端末を開く");
#endif
}

QIcon toolbarIcon(const QString& glyph) {
    QPixmap pixmap(28, 28);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor("#d6e7ff"), 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    if (glyph == QStringLiteral("back")) {
        painter.drawLine(QPointF(21, 14), QPointF(7, 14));
        painter.drawLine(QPointF(7, 14), QPointF(14, 7));
        painter.drawLine(QPointF(7, 14), QPointF(14, 21));
    } else if (glyph == QStringLiteral("forward")) {
        painter.drawLine(QPointF(7, 14), QPointF(21, 14));
        painter.drawLine(QPointF(21, 14), QPointF(14, 7));
        painter.drawLine(QPointF(21, 14), QPointF(14, 21));
    } else if (glyph == QStringLiteral("up")) {
        painter.drawLine(QPointF(14, 21), QPointF(14, 7));
        painter.drawLine(QPointF(14, 7), QPointF(7, 14));
        painter.drawLine(QPointF(14, 7), QPointF(21, 14));
        painter.drawLine(QPointF(7, 22), QPointF(21, 22));
    } else if (glyph == QStringLiteral("refresh")) {
        painter.drawArc(QRectF(5, 5, 18, 18), 40 * 16, 285 * 16);
        painter.drawLine(QPointF(21, 7), QPointF(21, 13));
        painter.drawLine(QPointF(21, 7), QPointF(15, 7));
    }
    return QIcon(pixmap);
}

QIcon terminalIcon() {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#9fc0ff"), 1.6));
    painter.setBrush(QColor("#24384f"));
    painter.drawRoundedRect(QRectF(2, 3, 20, 17), 3, 3);
    painter.drawLine(QPointF(6, 8), QPointF(9, 11));
    painter.drawLine(QPointF(9, 11), QPointF(6, 14));
    painter.drawLine(QPointF(11, 15), QPointF(16, 15));
    return QIcon(pixmap);
}

QIcon explorerIcon() {
    QPixmap pixmap(28, 28);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#9fc0ff"), 1.7));
    painter.setBrush(QColor("#24384f"));
    painter.drawRoundedRect(QRectF(2, 3, 24, 22), 3, 3);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#f3bd58"));
    QPainterPath folder;
    folder.moveTo(6, 12);
    folder.lineTo(12, 12);
    folder.lineTo(14, 14);
    folder.lineTo(23, 14);
    folder.lineTo(22, 21);
    folder.lineTo(6, 21);
    folder.closeSubpath();
    painter.drawPath(folder);
    return QIcon(pixmap);
}

QIcon workspaceTabIcon(const QString& glyph) {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor("#9fc0ff"));
    painter.setFont(QFont(QStringLiteral("Segoe UI Symbol"), 15));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, glyph);
    return QIcon(pixmap);
}

QIcon splitIndicatorIcon(int paneCount) {
    QPixmap pixmap(28, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor("#9fc0ff"), 1.8));
    painter.setBrush(QColor("#24384f"));
    painter.drawRoundedRect(QRectF(2, 2, 24, 20), 3, 3);
    if (paneCount > 1) painter.drawLine(QPointF(14, 4), QPointF(14, 20));
    return QIcon(pixmap);
}

class FilePane final : public QWidget {
public:
    FilePane(const QString& startPath, const QStringList& savedPaths, int selectedTab,
             std::function<void()> onChanged, QWidget* parent = nullptr)
        : QWidget(parent), m_path(startPath), m_onChanged(std::move(onChanged)) {
        setObjectName("filePane");
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        m_tabs = new QTabBar(this);
        m_tabs->setExpanding(false);
        m_tabs->setTabsClosable(true);
        m_tabs->setMovable(true);
        auto* tabRow = new QHBoxLayout;
        tabRow->setContentsMargins(8, 4, 8, 4);
        tabRow->setSpacing(6);
        tabRow->addWidget(m_tabs, 1);
        auto* addTabButton = new QToolButton(this);
        addTabButton->setText("＋");
        addTabButton->setToolTip(tr("場所タブを追加"));
        addTabButton->setFixedSize(30, 28);
        tabRow->addWidget(addTabButton);
        layout->addLayout(tabRow);

        auto* pathBar = new QLineEdit(this);
        m_pathBar = pathBar;
        pathBar->installEventFilter(this);
        pathBar->setText(m_path);
        pathBar->setClearButtonEnabled(true);
        pathBar->setPlaceholderText(tr("フォルダーのパス"));
        layout->addWidget(pathBar);

        m_table = new QTableWidget(0, 3, this);
        m_table->setHorizontalHeaderLabels({tr("名前"), tr("更新日時"), tr("サイズ")});
        m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_table->setAlternatingRowColors(true);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_table->setShowGrid(false);
        m_table->setFocusPolicy(Qt::StrongFocus);
        m_table->setContextMenuPolicy(Qt::CustomContextMenu);
        m_table->installEventFilter(this);
        m_table->viewport()->installEventFilter(this);
        m_table->verticalHeader()->hide();
        m_table->horizontalHeader()->setStretchLastSection(true);
        m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        layout->addWidget(m_table, 1);
        m_tabs->installEventFilter(this);
        addTabButton->installEventFilter(this);

        connect(pathBar, &QLineEdit::returnPressed, this, [this, pathBar] {
            activate();
            setPath(pathBar->text());
        });
        connect(addTabButton, &QToolButton::clicked, this, [this] {
            addLocationTab(m_path);
            notifyChanged();
        });
        connect(m_tabs, &QTabBar::tabCloseRequested, this, [this](int index) {
            if (m_tabs->count() > 1) {
                m_tabs->removeTab(index);
                if (m_tabs->count() > 0) m_tabs->setCurrentIndex(qMin(index, m_tabs->count() - 1));
                notifyChanged();
            }
        });
        connect(m_tabs, &QTabBar::currentChanged, this, [this](int index) {
            const auto destination = m_tabs->tabData(index).toString();
            if (!destination.isEmpty()) {
                m_path = destination;
                m_pathBar->setText(m_path);
                refresh();
                recordHistory(m_path);
                activate();
                notifyChanged();
            }
        });
        connect(m_table, &QTableWidget::cellClicked, this, [this] { activate(); });
        connect(m_table, &QTableWidget::cellDoubleClicked, this, [this, pathBar](int row, int) {
            const auto* item = m_table->item(row, 0);
            if (!item) return;
            const auto destination = item->data(Qt::UserRole).toString();
            if (item->data(Qt::UserRole + 1).toBool() && QFileInfo(destination).isDir()) setPath(destination);
            else QDesktopServices::openUrl(QUrl::fromLocalFile(destination));
            pathBar->setText(m_path);
        });
        connect(m_table, &QWidget::customContextMenuRequested, this, [this](const QPoint& position) {
            showContextMenu(position);
        });
        addShortcut(QKeySequence::Copy, [this] { copySelection(false); });
        addShortcut(QKeySequence::Cut, [this] { copySelection(true); });
        addShortcut(QKeySequence::Paste, [this] { pasteClipboard(); });
        addShortcut(QKeySequence::Delete, [this] { deleteSelection(); });
        addShortcut(QKeySequence(Qt::Key_F2), [this] { renameSelection(); });
        addShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+N")), [this] { createFolder(); });
        addShortcut(QKeySequence(Qt::Key_Return), [this] { openSelection(); });
        addShortcut(QKeySequence(Qt::Key_Enter), [this] { openSelection(); });

        {
            const QSignalBlocker blocker(m_tabs);
            for (const auto& path : savedPaths) {
                const auto clean = QDir::cleanPath(path);
                if (QFileInfo(clean).isDir()) addLocationTab(clean);
            }
            if (m_tabs->count() == 0) addLocationTab(startPath);
            m_tabs->setCurrentIndex(qBound(0, selectedTab, m_tabs->count() - 1));
        }
        m_path = m_tabs->tabData(m_tabs->currentIndex()).toString();
        m_pathBar->setText(m_path);
        m_history = {m_path};
        m_historyIndex = 0;
        refresh();
    }

    QStringList locationPaths() const {
        QStringList paths;
        for (int i = 0; i < m_tabs->count(); ++i) paths.append(m_tabs->tabData(i).toString());
        return paths;
    }

    int selectedTab() const { return m_tabs->currentIndex(); }
    QString currentPath() const { return m_path; }
    bool canGoBack() const { return m_historyIndex > 0; }
    bool canGoForward() const { return m_historyIndex >= 0 && m_historyIndex + 1 < m_history.size(); }
    void setCallbacks(std::function<void(FilePane*)> activated, std::function<void()> historyChanged) {
        m_onActivated = std::move(activated);
        m_onHistoryChanged = std::move(historyChanged);
    }
    void setActive(bool active) {
        if (property("activePane").toBool() == active) return;
        setProperty("activePane", active);
        style()->unpolish(this);
        style()->polish(this);
        update();
    }
    void goBack() { if (canGoBack()) navigateHistory(m_historyIndex - 1); }
    void goForward() { if (canGoForward()) navigateHistory(m_historyIndex + 1); }
    void goUp() {
        const auto parent = QFileInfo(m_path).absolutePath();
        if (parent != m_path) setPath(parent);
    }
    void refreshDirectory() { refresh(); }
    void addLocationTabAtCurrent() { addLocationTab(m_path); notifyChanged(); }
    void copySelectedItems() { copySelection(false); }
    void cutSelectedItems() { copySelection(true); }
    void pasteItems() { pasteClipboard(); }
    void renameSelectedItem() { renameSelection(); }
    void deleteSelectedItems() { deleteSelection(); }
    void createNewFolder() { createFolder(); }
    bool openTerminalAt(const QString& path) const {
#if defined(Q_OS_WIN)
        const auto systemRoot = qEnvironmentVariable("SystemRoot", QStringLiteral("C:/Windows"));
        auto command = QDir(systemRoot).filePath(QStringLiteral("System32/cmd.exe"));
        if (!QFileInfo::exists(command)) command = QStandardPaths::findExecutable(QStringLiteral("cmd.exe"));
        if (command.isEmpty()) return false;
        const auto nativeCommand = QDir::toNativeSeparators(command);
        auto commandLine = QStringLiteral("\"") + nativeCommand + QStringLiteral("\"");
        auto mutableCommandLine = commandLine.toStdWString();
        const auto workingDirectory = QDir::toNativeSeparators(path).toStdWString();
        STARTUPINFOW startupInfo{};
        startupInfo.cb = sizeof(startupInfo);
        PROCESS_INFORMATION processInfo{};
        const auto started = CreateProcessW(
            reinterpret_cast<LPCWSTR>(nativeCommand.utf16()), mutableCommandLine.data(),
            nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, workingDirectory.c_str(),
            &startupInfo, &processInfo);
        if (!started) return false;
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return true;
#elif defined(Q_OS_LINUX)
        const auto terminal = QStandardPaths::findExecutable(QStringLiteral("x-terminal-emulator"));
        return !terminal.isEmpty() && QProcess::startDetached(terminal, {}, path);
#else
        Q_UNUSED(path);
        return false;
#endif
    }
    bool openExplorerAt(const QString& path) const {
        const QFileInfo info(path);
        return info.isDir() && QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        Q_UNUSED(watched);
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn) activate();
        return QWidget::eventFilter(watched, event);
    }

private:
    QString m_path;
    QTabBar* m_tabs = nullptr;
    QLineEdit* m_pathBar = nullptr;
    QTableWidget* m_table = nullptr;
    quint64 m_requestId = 0;
    std::function<void()> m_onChanged;
    std::function<void(FilePane*)> m_onActivated;
    std::function<void()> m_onHistoryChanged;
    QStringList m_history;
    int m_historyIndex = -1;
    bool m_replayingHistory = false;

    static QString folderLabel(const QString& path) {
        const auto name = QFileInfo(path).fileName();
        return name.isEmpty() ? QDir::toNativeSeparators(path) : name;
    }

    void addLocationTab(const QString& path) {
        const auto index = m_tabs->addTab(folderLabel(path));
        m_tabs->setTabData(index, QDir::cleanPath(path));
        m_tabs->setCurrentIndex(index);
    }

    void notifyChanged() {
        if (m_onChanged) m_onChanged();
    }

    void activate() {
        if (m_onActivated) m_onActivated(this);
    }

    void recordHistory(const QString& path) {
        if (m_replayingHistory || (m_historyIndex >= 0 && m_history.at(m_historyIndex) == path)) return;
        while (m_history.size() > m_historyIndex + 1) m_history.removeLast();
        m_history.append(path);
        m_historyIndex = m_history.size() - 1;
        if (m_onHistoryChanged) m_onHistoryChanged();
    }

    void navigateHistory(int index) {
        if (index < 0 || index >= m_history.size()) return;
        m_replayingHistory = true;
        setPath(m_history.at(index));
        m_replayingHistory = false;
        m_historyIndex = index;
        if (m_onHistoryChanged) m_onHistoryChanged();
    }

    void setPath(const QString& path) {
        const auto clean = QDir::cleanPath(path);
        if (!QFileInfo(clean).isDir()) return;
        const auto current = m_tabs->currentIndex();
        if (current >= 0 && m_tabs->tabData(current).toString() == clean) return;
        for (int i = 0; i < m_tabs->count(); ++i) {
            if (i == current) continue;
            if (m_tabs->tabData(i).toString() == clean) {
                m_tabs->setCurrentIndex(i);
                return;
            }
        }
        if (current >= 0) {
            m_tabs->setTabData(current, clean);
            m_tabs->setTabText(current, folderLabel(clean));
            m_path = clean;
            m_pathBar->setText(m_path);
            recordHistory(m_path);
            refresh();
        }
        notifyChanged();
    }

    void refresh() {
        const auto utf8Path = m_path.toUtf8().toStdString();
        const auto requestId = ++m_requestId;
        auto* watcher = new QFutureWatcher<QString>(this);
        connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, requestId] {
            const auto json = watcher->result();
            watcher->deleteLater();
            if (requestId == m_requestId) populate(json);
        });
        watcher->setFuture(QtConcurrent::run([utf8Path] {
            return rustString(read_directory(rust::Str(utf8Path.data(), utf8Path.size())));
        }));
    }

    void populate(const QString& json) {
        const auto document = QJsonDocument::fromJson(json.toUtf8());
        const auto entries = document.array();
        m_table->setRowCount(entries.size());
        for (qsizetype row = 0; row < entries.size(); ++row) {
            const auto entry = entries.at(row).toObject();
            const auto name = entry.value("name").toString();
            const auto path = entry.value("path").toString();
            const auto isDir = entry.value("is_dir").toBool();
            auto* nameItem = new QTableWidgetItem(name);
            nameItem->setIcon(fileItemIcon(isDir));
            nameItem->setData(Qt::UserRole, path);
            nameItem->setData(Qt::UserRole + 1, isDir);
            auto* timeItem = new QTableWidgetItem(QDateTime::fromSecsSinceEpoch(entry.value("modified").toInteger()).toString("yyyy/MM/dd HH:mm"));
            const auto size = entry.value("size").toInteger();
            auto* sizeItem = new QTableWidgetItem(isDir ? QString{} : QLocale().formattedDataSize(size));
            sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(static_cast<int>(row), 0, nameItem);
            m_table->setItem(static_cast<int>(row), 1, timeItem);
            m_table->setItem(static_cast<int>(row), 2, sizeItem);
        }
    }

    void addShortcut(const QKeySequence& sequence, std::function<void()> action) {
        auto* shortcut = new QShortcut(sequence, m_table);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, &QShortcut::activated, this, [action = std::move(action)] { action(); });
    }

    QStringList selectedPaths() const {
        QStringList paths;
        QSet<int> rows;
        for (const auto& index : m_table->selectionModel()->selectedRows()) rows.insert(index.row());
        for (const auto row : rows) {
            const auto* item = m_table->item(row, 0);
            if (item) paths.append(item->data(Qt::UserRole).toString());
        }
        return paths;
    }

    static void refreshOpenPanes(const QStringList& directories) {
        for (auto* widget : QApplication::allWidgets()) {
            auto* pane = dynamic_cast<FilePane*>(widget);
            if (pane && directories.contains(QDir::cleanPath(pane->m_path))) pane->refresh();
        }
    }

    void showContextMenu(const QPoint& position) {
        activate();
        const auto index = m_table->indexAt(position);
        if (index.isValid() && !m_table->selectionModel()->isSelected(index)) {
            m_table->selectRow(index.row());
        }
        const auto paths = selectedPaths();
        QMenu menu(m_table);
        auto* open = menu.addAction(tr("開く"));
        open->setEnabled(paths.size() == 1);
        auto* terminal = menu.addAction(terminalActionText());
        QString terminalDirectory = m_path;
        if (paths.size() == 1 && QFileInfo(paths.first()).isDir()) terminalDirectory = paths.first();
        terminal->setEnabled(paths.size() <= 1 && QFileInfo(terminalDirectory).isDir());
        auto explorerDirectory = terminalDirectory;
        if (paths.size() == 1 && !QFileInfo(paths.first()).isDir())
            explorerDirectory = QFileInfo(paths.first()).absolutePath();
        auto* explorer = menu.addAction(tr("Explorerを開く"));
        explorer->setEnabled(paths.size() <= 1 && QFileInfo(explorerDirectory).isDir());
        auto* copy = menu.addAction(tr("コピー"));
        copy->setEnabled(!paths.isEmpty());
        auto* cut = menu.addAction(tr("切り取り"));
        cut->setEnabled(!paths.isEmpty());
        auto* paste = menu.addAction(tr("貼り付け"));
        paste->setEnabled(!QApplication::clipboard()->mimeData()->urls().isEmpty());
        menu.addSeparator();
        auto* newFolder = menu.addAction(tr("新しいフォルダー"));
        auto* rename = menu.addAction(tr("名前を変更"));
        rename->setEnabled(paths.size() == 1);
        auto* remove = menu.addAction(tr("削除"));
        remove->setEnabled(!paths.isEmpty());

        auto* chosen = menu.exec(m_table->viewport()->mapToGlobal(position));
        if (chosen == open) openSelection();
        else if (chosen == terminal && !openTerminalAt(terminalDirectory)) {
            QMessageBox::warning(this, tr("端末を開けません"), tr("この環境でターミナルを起動できませんでした。"));
        }
        else if (chosen == explorer && !openExplorerAt(explorerDirectory)) {
            QMessageBox::warning(this, tr("Explorerを開けません"), tr("このフォルダーを開けませんでした。"));
        }
        else if (chosen == copy) copySelection(false);
        else if (chosen == cut) copySelection(true);
        else if (chosen == paste) pasteClipboard();
        else if (chosen == newFolder) createFolder();
        else if (chosen == rename) renameSelection();
        else if (chosen == remove) deleteSelection();
    }

    void openSelection() {
        const auto paths = selectedPaths();
        if (paths.size() != 1) return;
        const auto path = paths.first();
        if (QFileInfo(path).isDir()) setPath(path);
        else QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }

    void copySelection(bool cut) {
        const auto paths = selectedPaths();
        if (paths.isEmpty()) return;
        auto* mimeData = new QMimeData;
        QList<QUrl> urls;
        for (const auto& path : paths) urls.append(QUrl::fromLocalFile(path));
        mimeData->setUrls(urls);
        mimeData->setData("application/x-lightweight-file-manager-operation", cut ? "move" : "copy");
        QApplication::clipboard()->setMimeData(mimeData);
    }

    void pasteClipboard() {
        const auto* mimeData = QApplication::clipboard()->mimeData();
        const auto urls = mimeData->urls();
        if (urls.isEmpty()) return;
        const auto moving = mimeData->data("application/x-lightweight-file-manager-operation") == "move";
        QStringList errors;
        QStringList affectedDirectories{m_path};
        for (const auto& url : urls) {
            if (!url.isLocalFile()) continue;
            const auto source = url.toLocalFile();
            const auto destination = QDir(m_path).filePath(QFileInfo(source).fileName());
            if (QDir::cleanPath(source) == QDir::cleanPath(destination)) continue;
            const auto error = moving ? coreMovePath(source, destination) : coreCopyPath(source, destination);
            if (!error.isEmpty()) errors.append(QFileInfo(source).fileName() + QStringLiteral(": ") + error);
            else affectedDirectories.append(QFileInfo(source).absolutePath());
        }
        refreshOpenPanes(affectedDirectories);
        if (!errors.isEmpty()) QMessageBox::warning(this, tr("貼り付けできません"), errors.join('\n'));
        else if (moving) QApplication::clipboard()->clear();
    }

    void createFolder() {
        bool accepted = false;
        const auto name = QInputDialog::getText(this, tr("新しいフォルダー"), tr("フォルダー名"),
                                                QLineEdit::Normal, QString{}, &accepted).trimmed();
        if (!accepted || name.isEmpty()) return;
        const auto path = QDir(m_path).filePath(name);
        const auto error = coreCreateDirectory(path);
        if (!error.isEmpty()) QMessageBox::warning(this, tr("作成できません"), error);
        else refreshOpenPanes({m_path});
    }

    void renameSelection() {
        const auto paths = selectedPaths();
        if (paths.size() != 1) return;
        const auto source = paths.first();
        bool accepted = false;
        const auto name = QInputDialog::getText(this, tr("名前を変更"), tr("名前"),
                                                QLineEdit::Normal, QFileInfo(source).fileName(), &accepted).trimmed();
        if (!accepted || name.isEmpty() || name == QFileInfo(source).fileName()) return;
        const auto destination = QDir(QFileInfo(source).absolutePath()).filePath(name);
        const auto error = coreRenamePath(source, destination);
        if (!error.isEmpty()) QMessageBox::warning(this, tr("名前を変更できません"), error);
        else refreshOpenPanes({QFileInfo(source).absolutePath()});
    }

    void deleteSelection() {
        const auto paths = selectedPaths();
        if (paths.isEmpty()) return;
        const auto answer = QMessageBox::question(this, tr("削除の確認"),
            tr("選択した %1 件を完全に削除しますか？").arg(paths.size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) return;
        QStringList errors;
        for (const auto& path : paths) {
            const auto error = coreRemovePath(path);
            if (!error.isEmpty()) errors.append(QFileInfo(path).fileName() + QStringLiteral(": ") + error);
        }
        QStringList affectedDirectories;
        for (const auto& path : paths) affectedDirectories.append(QFileInfo(path).absolutePath());
        refreshOpenPanes(affectedDirectories);
        if (!errors.isEmpty()) QMessageBox::warning(this, tr("削除できません"), errors.join('\n'));
    }
};

class MainWindow final : public QMainWindow {
    struct PaneState {
        QStringList paths;
        int currentTab = 0;
    };

    struct Workspace {
        QString name;
        QString icon;
        int visiblePaneCount = 1;
        QWidget* page = nullptr;
        QSplitter* splitter = nullptr;
        QToolButton* button = nullptr;
        QVector<FilePane*> panes;
        QList<int> savedSplitSizes;
    };

public:
    MainWindow() {
        setWindowTitle(tr("Lightweight File Manager"));
        setWindowIcon(applicationIcon());
        resize(1360, 860);
        menuBar()->setStyleSheet("QMenuBar { color: #cbd2dd; background: #171c24; } QMenuBar::item { padding: 6px 10px; background: transparent; } QMenuBar::item:selected { background: #2a3442; }");

        auto* root = new QWidget(this);
        root->setObjectName("appRoot");
        root->setStyleSheet(R"(
            QWidget#appRoot { background: #11151b; color: #cbd2dd; }
            QWidget#filePane[activePane="true"] { border: 2px solid #71a7ff; }
            QMenuBar { color: #cbd2dd; background: #171c24; }
            QMenuBar::item { padding: 6px 10px; background: transparent; }
            QMenuBar::item:selected { background: #2a3442; }
            QToolBar { background: #1b212a; border: 0; spacing: 8px; padding: 6px 10px; }
            QToolButton, QPushButton { color: #cbd2dd; background: #252d38; border: 1px solid #303946; border-radius: 5px; padding: 6px 10px; }
            QToolButton:hover, QPushButton:hover { background: #303b49; }
            QPushButton#splitButton { font-size: 14pt; font-weight: 600; padding: 5px 12px; }
            QLineEdit { color: #d9e0e9; background: #11161d; border: 1px solid #303946; border-radius: 5px; padding: 7px 9px; selection-background-color: #46658e; }
            QTabBar { background: #171c24; }
            QTabBar::tab { color: #aeb8c6; background: #171c24; border: 0; padding: 8px 14px; min-height: 24px; }
            QTabBar::tab:selected { color: #e6eaf0; background: #2a3442; border-radius: 5px; }
            QTableWidget { color: #d9e0e9; background: #141920; alternate-background-color: #171e27; border: 0; selection-background-color: #365273; selection-color: #ffffff; outline: 0; }
            QTableWidget::item:selected { background: #365273; color: #ffffff; }
            QHeaderView::section { color: #aeb8c6; background: #1d242e; border: 0; border-bottom: 1px solid #303946; padding: 8px 10px; }
            QTableWidget::item { padding: 5px 8px; }
            QSplitter::handle { background: #303946; }
            QStatusBar { color: #9ca7b8; background: #1b212a; border-top: 1px solid #303946; }
            QMenu { color: #cbd2dd; background: #1b212a; border: 1px solid #303946; }
            QMenu::item:selected { background: #2a3442; }
        )");
        auto* row = new QHBoxLayout(root);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(0);

        auto* content = new QWidget(root);
        auto* contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(12, 8, 12, 8);
        contentLayout->setSpacing(8);

        auto* fileMenu = menuBar()->addMenu(tr("ファイル"));
        auto* editMenu = menuBar()->addMenu(tr("編集"));
        auto* viewMenu = menuBar()->addMenu(tr("表示"));
        auto* helpMenu = menuBar()->addMenu(tr("ヘルプ"));

        auto* newTabAction = fileMenu->addAction(tr("新しい場所タブ"));
        auto* exportLayoutAction = fileMenu->addAction(tr("タブ・分割構成をエクスポート..."));
        auto* importLayoutAction = fileMenu->addAction(tr("タブ・分割構成をインポート..."));
        auto* terminalAction = fileMenu->addAction(terminalActionText());
        terminalAction->setToolTip(tr("現在のフォルダーを作業場所として開きます"));
        auto* explorerAction = fileMenu->addAction(explorerIcon(), tr("Explorerを開く"));
        explorerAction->setToolTip(tr("現在のフォルダーをExplorerで開きます"));
        fileMenu->addSeparator();
        auto* exitAction = fileMenu->addAction(tr("終了"));
        connect(newTabAction, &QAction::triggered, this, [this] {
            if (auto* pane = currentPane()) pane->addLocationTabAtCurrent();
        });
        connect(exportLayoutAction, &QAction::triggered, this, [this] { exportWorkspaceConfig(); });
        connect(importLayoutAction, &QAction::triggered, this, [this] { importWorkspaceConfig(); });
        connect(terminalAction, &QAction::triggered, this, [this] {
            auto* pane = currentPane();
            if (pane && !pane->openTerminalAt(pane->currentPath()))
                QMessageBox::warning(this, tr("端末を開けません"), tr("この環境でターミナルを起動できませんでした。"));
        });
        connect(explorerAction, &QAction::triggered, this, [this] {
            auto* pane = currentPane();
            if (pane && !pane->openExplorerAt(pane->currentPath()))
                QMessageBox::warning(this, tr("Explorerを開けません"), tr("このフォルダーを開けませんでした。"));
        });
        connect(exitAction, &QAction::triggered, this, &QWidget::close);

        auto* newFolderAction = editMenu->addAction(tr("新しいフォルダー  (Ctrl+Shift+N)"));
        auto* copyAction = editMenu->addAction(tr("コピー  (Ctrl+C)"));
        auto* cutAction = editMenu->addAction(tr("切り取り  (Ctrl+X)"));
        auto* pasteAction = editMenu->addAction(tr("貼り付け  (Ctrl+V)"));
        auto* renameAction = editMenu->addAction(tr("名前を変更  (F2)"));
        auto* deleteAction = editMenu->addAction(tr("削除  (Delete)"));
        connect(newFolderAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->createNewFolder(); });
        connect(copyAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->copySelectedItems(); });
        connect(cutAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->cutSelectedItems(); });
        connect(pasteAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->pasteItems(); });
        connect(renameAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->renameSelectedItem(); });
        connect(deleteAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->deleteSelectedItems(); });

        auto* refreshAction = viewMenu->addAction(toolbarIcon(QStringLiteral("refresh")), tr("更新 (F5)"));
        refreshAction->setShortcut(QKeySequence(Qt::Key_F5));
        connect(refreshAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->refreshDirectory(); });
        auto* viewSplitMenu = viewMenu->addMenu(tr("分割"));
        auto* viewHorizontalSplit = viewSplitMenu->addAction(tr("左右に分割"));
        auto* viewVerticalSplit = viewSplitMenu->addAction(tr("上下に分割"));
        auto* viewClearSplit = viewSplitMenu->addAction(tr("分割解除"));
        connect(viewHorizontalSplit, &QAction::triggered, this, [this] { setSplitOrientation(Qt::Horizontal); });
        connect(viewVerticalSplit, &QAction::triggered, this, [this] { setSplitOrientation(Qt::Vertical); });
        connect(viewClearSplit, &QAction::triggered, this, [this] { clearSplit(); });

        auto* aboutAction = helpMenu->addAction(tr("バージョン情報"));
        connect(aboutAction, &QAction::triggered, this, [this] {
            QMessageBox::about(this, tr("Lightweight File Managerについて"),
                               tr("Lightweight File Manager\nVersion 0.1.0\n\nRust + Qt 6 Widgets"));
        });

        auto* toolbar = new QToolBar(tr("ナビゲーション"), content);
        toolbar->setMovable(false);
        toolbar->setIconSize(QSize(28, 28));
        m_backAction = toolbar->addAction(toolbarIcon(QStringLiteral("back")), tr("戻る"));
        m_forwardAction = toolbar->addAction(toolbarIcon(QStringLiteral("forward")), tr("進む"));
        auto* upAction = toolbar->addAction(toolbarIcon(QStringLiteral("up")), tr("上へ"));
        toolbar->addAction(refreshAction);
        connect(m_backAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->goBack(); });
        connect(m_forwardAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->goForward(); });
        connect(upAction, &QAction::triggered, this, [this] { if (auto* pane = currentPane()) pane->goUp(); });
        auto* terminalToolbarAction = toolbar->addAction(terminalIcon(), terminalActionText());
        terminalToolbarAction->setToolTip(terminalActionText());
        connect(terminalToolbarAction, &QAction::triggered, this, [this] {
            auto* pane = currentPane();
            if (pane && !pane->openTerminalAt(pane->currentPath()))
                QMessageBox::warning(this, tr("端末を開けません"), tr("この環境でターミナルを起動できませんでした。"));
        });
        auto* explorerToolbarAction = toolbar->addAction(explorerIcon(), tr("Explorerを開く"));
        explorerToolbarAction->setToolTip(tr("現在のフォルダーをExplorerで開きます"));
        connect(explorerToolbarAction, &QAction::triggered, this, [this] {
            auto* pane = currentPane();
            if (pane && !pane->openExplorerAt(pane->currentPath()))
                QMessageBox::warning(this, tr("Explorerを開けません"), tr("このフォルダーを開けませんでした。"));
        });
        m_splitButton = new QPushButton(tr("分割 1"), toolbar);
        m_splitButton->setObjectName("splitButton");
        m_splitButton->setIcon(splitIndicatorIcon(1));
        m_splitButton->setIconSize(QSize(28, 24));
        m_splitButton->setMinimumHeight(42);
        auto* splitMenu = new QMenu(m_splitButton);
        auto* horizontalSplit = splitMenu->addAction(tr("左右に分割"));
        auto* verticalSplit = splitMenu->addAction(tr("上下に分割"));
        splitMenu->addSeparator();
        auto* removeSplit = splitMenu->addAction(tr("分割解除"));
        m_splitButton->setMenu(splitMenu);
        toolbar->addWidget(m_splitButton);
        contentLayout->addWidget(toolbar);

        const auto home = QDir::homePath();
        auto* workspaceArea = new QWidget(content);
        auto* workspaceLayout = new QHBoxLayout(workspaceArea);
        workspaceLayout->setContentsMargins(0, 0, 0, 0);
        workspaceLayout->setSpacing(8);
        auto* workspaceNav = new QWidget(workspaceArea);
        workspaceNav->setFixedWidth(260);
        workspaceNav->setStyleSheet("background: #171c24;");
        auto* workspaceNavLayout = new QVBoxLayout(workspaceNav);
        workspaceNavLayout->setContentsMargins(0, 0, 0, 0);
        workspaceNavLayout->setSpacing(8);
        auto* workspaceTabScroll = new QScrollArea(workspaceNav);
        workspaceTabScroll->setFrameShape(QFrame::NoFrame);
        workspaceTabScroll->setStyleSheet("QScrollArea { background: #171c24; border: 0; }");
        workspaceTabScroll->viewport()->setStyleSheet("background: #171c24;");
        workspaceTabScroll->setWidgetResizable(true);
        workspaceTabScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        workspaceTabScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        workspaceTabScroll->setFixedWidth(210);
        m_workspaceTabsWidget = new QWidget;
        m_workspaceTabsWidget->setObjectName("workspaceTabs");
        m_workspaceTabsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_workspaceTabsLayout = new QVBoxLayout(m_workspaceTabsWidget);
        m_workspaceTabsLayout->setContentsMargins(5, 5, 5, 5);
        m_workspaceTabsLayout->setSpacing(6);
        m_workspaceTabsLayout->addStretch(1);
        workspaceTabScroll->setWidget(m_workspaceTabsWidget);
        m_pages = new QStackedWidget(workspaceArea);
        workspaceNavLayout->addWidget(workspaceTabScroll, 1);
        auto* addButton = new QToolButton(workspaceNav);
        addButton->setText("＋");
        addButton->setToolTip(tr("メインタブを追加"));
        addButton->setFixedHeight(48);
        addButton->setStyleSheet("QToolButton { color: #d8e5f8; background: #252d38; border: 1px solid #303946; border-radius: 6px; font-size: 18pt; }");
        workspaceNavLayout->addWidget(addButton);
        workspaceLayout->addWidget(workspaceNav);
        workspaceLayout->addWidget(m_pages, 1);
        connect(addButton, &QToolButton::clicked, this, [this, home] {
            PaneState pane;
            pane.paths = {home};
            addWorkspace(tr("新規"), QStringLiteral("＋"), Qt::Horizontal, {pane}, {}, true);
        });
        contentLayout->addWidget(workspaceArea, 1);
        connect(horizontalSplit, &QAction::triggered, this, [this] {
            setSplitOrientation(Qt::Horizontal);
        });
        connect(verticalSplit, &QAction::triggered, this, [this] {
            setSplitOrientation(Qt::Vertical);
        });
        connect(removeSplit, &QAction::triggered, this, [this] { clearSplit(); });
        row->addWidget(content, 1);
        setCentralWidget(root);

        loadSettings(home);
        updateStatus();
    }

protected:
    void closeEvent(QCloseEvent* event) override {
        saveSettings();
        QMainWindow::closeEvent(event);
    }

private:
    QStackedWidget* m_pages = nullptr;
    QWidget* m_workspaceTabsWidget = nullptr;
    QVBoxLayout* m_workspaceTabsLayout = nullptr;
    QAction* m_backAction = nullptr;
    QAction* m_forwardAction = nullptr;
    QPushButton* m_splitButton = nullptr;
    FilePane* m_activePane = nullptr;
    QVector<Workspace> m_workspaces;
    bool m_restoring = false;

    void attachPane(FilePane* pane) {
        pane->setCallbacks([this](FilePane* active) {
            setActivePane(active);
        }, [this] {
            updateNavigationActions();
        });
    }

    void setActivePane(FilePane* active) {
        m_activePane = active;
        for (const auto& workspace : m_workspaces)
            for (auto* pane : workspace.panes) pane->setActive(pane == active);
        updateNavigationActions();
    }

    FilePane* currentPane() const {
        const auto index = m_pages->currentIndex();
        if (index < 0 || index >= m_workspaces.size()) return nullptr;
        const auto& panes = m_workspaces[index].panes;
        if (panes.contains(m_activePane)) return m_activePane;
        return panes.isEmpty() ? nullptr : panes.first();
    }

    void updateNavigationActions() {
        auto* pane = currentPane();
        if (m_backAction) m_backAction->setEnabled(pane && pane->canGoBack());
        if (m_forwardAction) m_forwardAction->setEnabled(pane && pane->canGoForward());
    }

    static PaneState defaultPane(const QString& home) {
        PaneState pane;
        pane.paths = {home};
        return pane;
    }

    void loadSettings(const QString& home) {
        m_restoring = true;
        QSettings settings;
        const auto document = QJsonDocument::fromJson(settings.value("uiState").toByteArray());
        const auto root = document.object();
        const auto workspaceArray = root.value("workspaces").toArray();
        for (const auto& workspaceValue : workspaceArray) {
            const auto object = workspaceValue.toObject();
            const auto name = object.value("name").toString();
            if (name.isEmpty()) continue;
            const auto icon = object.value("icon").toString(QStringLiteral("◫"));
            const auto orientation = object.value("orientation").toInt(static_cast<int>(Qt::Horizontal)) == static_cast<int>(Qt::Vertical)
                ? Qt::Vertical : Qt::Horizontal;
            QVector<PaneState> panes;
            for (const auto& paneValue : object.value("panes").toArray()) {
                const auto paneObject = paneValue.toObject();
                PaneState pane;
                for (const auto& path : paneObject.value("paths").toArray()) {
                    const auto value = path.toString();
                    if (!value.isEmpty()) pane.paths.append(value);
                }
                pane.currentTab = paneObject.value("currentTab").toInt(0);
                panes.append(pane);
            }
            if (panes.isEmpty()) panes.append(defaultPane(home));
            QList<int> sizes;
            for (const auto& size : object.value("sizes").toArray()) sizes.append(size.toInt());
            const auto visiblePanes = qBound(1, object.value("visiblePanes").toInt(panes.size()), panes.size());
            addWorkspace(name, icon, orientation, panes, sizes, false, visiblePanes);
        }

        if (m_workspaces.isEmpty()) {
            addWorkspace(tr("Home"), QStringLiteral("⌂"), Qt::Horizontal,
                         {defaultPane(home)}, {}, false);
        }
        const auto current = qBound(0, root.value("currentWorkspace").toInt(0), static_cast<int>(m_workspaces.size()) - 1);
        selectWorkspace(current, false);
        m_restoring = false;
    }

    void addWorkspace(const QString& name, const QString& icon, Qt::Orientation orientation,
                      const QVector<PaneState>& paneStates, const QList<int>& sizes, bool select,
                      int visiblePaneCount = -1) {
        Workspace workspace;
        workspace.name = name;
        workspace.icon = icon;
        workspace.visiblePaneCount = visiblePaneCount < 0 ? qMin(2, paneStates.size()) : qBound(1, visiblePaneCount, qMax(1, paneStates.size()));
        workspace.savedSplitSizes = sizes;
        workspace.page = new QWidget(m_pages);
        auto* pageLayout = new QVBoxLayout(workspace.page);
        pageLayout->setContentsMargins(0, 0, 0, 0);
        workspace.splitter = new QSplitter(orientation, workspace.page);
        workspace.splitter->setChildrenCollapsible(false);
        for (const auto& paneState : paneStates) {
            const auto initialPath = !paneState.paths.isEmpty() && QFileInfo(paneState.paths.first()).isDir()
                ? paneState.paths.first() : QDir::homePath();
            auto* pane = new FilePane(initialPath, paneState.paths, paneState.currentTab,
                                      [this] { if (!m_restoring) saveSettings(); }, workspace.splitter);
            attachPane(pane);
            workspace.splitter->addWidget(pane);
            workspace.panes.append(pane);
            workspace.splitter->setStretchFactor(workspace.panes.size() - 1, 1);
        }
        for (int paneIndex = workspace.visiblePaneCount; paneIndex < workspace.panes.size(); ++paneIndex)
            workspace.panes[paneIndex]->hide();
        pageLayout->addWidget(workspace.splitter);
        workspace.button = new QToolButton(m_workspaceTabsWidget);
        workspace.button->setText(name);
        workspace.button->setIcon(workspaceTabIcon(icon));
        workspace.button->setIconSize(QSize(24, 24));
        workspace.button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        workspace.button->setMinimumWidth(180);
        workspace.button->setFixedHeight(48);
        workspace.button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        workspace.button->setCheckable(true);
        workspace.button->setContextMenuPolicy(Qt::CustomContextMenu);
        workspace.button->setToolTip(name);
        workspace.button->setStyleSheet("QToolButton { color: #aab4c2; background: #171c24; border: 1px solid #303946; border-radius: 6px; padding: 5px 10px; font-size: 11pt; } QToolButton:hover { background: #222a34; } QToolButton:checked { color: #edf2f8; background: #27384c; border-color: #5d82b1; }");
        const auto pageIndex = m_pages->addWidget(workspace.page);
        m_workspaces.append(workspace);
        if (!m_activePane && !workspace.panes.isEmpty()) m_activePane = workspace.panes.first();
        m_workspaceTabsLayout->insertWidget(m_workspaceTabsLayout->count() - 1, workspace.button);
        connectWorkspaceButton(workspace.button);
        if (select) selectWorkspace(pageIndex);

        if (!sizes.isEmpty()) {
            QTimer::singleShot(0, workspace.splitter, [splitter = workspace.splitter, sizes] {
                if (splitter->count() == sizes.size()) splitter->setSizes(sizes);
            });
        }
        if (!m_restoring) saveSettings();
    }

    void selectWorkspace(int index, bool persist = true) {
        if (index < 0 || index >= m_workspaces.size()) return;
        m_pages->setCurrentIndex(index);
        setActivePane(m_workspaces[index].panes.isEmpty() ? nullptr : m_workspaces[index].panes.first());
        for (int i = 0; i < m_workspaces.size(); ++i) m_workspaces[i].button->setChecked(i == index);
        updateStatus();
        if (persist && !m_restoring) saveSettings();
    }

    void renameWorkspace(int index, const QPoint& position) {
        if (index < 0 || index >= m_workspaces.size()) return;
        auto& workspace = m_workspaces[index];
        QMenu menu(workspace.button);
        auto* renameAction = menu.addAction(tr("名前を変更"));
        auto* deleteAction = menu.addAction(tr("削除"));
        if (m_workspaces.size() <= 1) deleteAction->setEnabled(false);
        auto* chosen = menu.exec(workspace.button->mapToGlobal(position));
        if (chosen == deleteAction) {
            auto* button = workspace.button;
            QTimer::singleShot(0, this, [this, button] {
                for (int current = 0; current < m_workspaces.size(); ++current) {
                    if (m_workspaces[current].button == button) {
                        deleteWorkspace(current);
                        return;
                    }
                }
            });
            return;
        }
        if (chosen != renameAction) return;
        bool accepted = false;
        const auto name = QInputDialog::getText(this, tr("メインタブ名を変更"), tr("名前"),
                                                QLineEdit::Normal, workspace.name, &accepted).trimmed();
        if (!accepted || name.isEmpty()) return;
        workspace.name = name;
        workspace.button->setText(name);
        workspace.button->setToolTip(name);
        workspace.button->setToolTip(name);
        saveSettings();
    }

    void connectWorkspaceButton(QToolButton* button) {
        connect(button, &QToolButton::clicked, this, [this, button] {
            for (int index = 0; index < m_workspaces.size(); ++index)
                if (m_workspaces[index].button == button) { selectWorkspace(index); return; }
        });
        connect(button, &QWidget::customContextMenuRequested, this, [this, button](const QPoint& position) {
            for (int index = 0; index < m_workspaces.size(); ++index)
                if (m_workspaces[index].button == button) { renameWorkspace(index, position); return; }
        });
    }

    void deleteWorkspace(int index) {
        if (m_workspaces.size() <= 1 || index < 0 || index >= m_workspaces.size()) return;
        const int currentIndex = m_pages->currentIndex();
        auto workspace = m_workspaces.takeAt(index);
        if (workspace.panes.contains(m_activePane)) m_activePane = nullptr;
        m_workspaceTabsLayout->removeWidget(workspace.button);
        m_pages->removeWidget(workspace.page);
        workspace.button->deleteLater();
        workspace.page->deleteLater();
        const int nextIndex = qBound(0, currentIndex > index ? currentIndex - 1 : currentIndex,
                                     static_cast<int>(m_workspaces.size()) - 1);
        selectWorkspace(nextIndex);
        saveSettings();
    }

    void setSplitOrientation(Qt::Orientation orientation) {
        const auto index = m_pages->currentIndex();
        if (index < 0 || index >= m_workspaces.size()) return;
        auto& workspace = m_workspaces[index];
        workspace.splitter->setOrientation(orientation);
        if (workspace.panes.isEmpty()) return;
        if (workspace.panes.size() == 1) {
            const auto path = workspace.panes.first()->currentPath();
            PaneState pane;
            pane.paths = {path};
            auto* second = new FilePane(path, pane.paths, 0, [this] { if (!m_restoring) saveSettings(); }, workspace.splitter);
            attachPane(second);
            workspace.splitter->addWidget(second);
            workspace.panes.append(second);
            workspace.splitter->setStretchFactor(1, 1);
        }
        workspace.visiblePaneCount = workspace.panes.size();
        for (int paneIndex = 0; paneIndex < workspace.panes.size(); ++paneIndex) {
            workspace.panes[paneIndex]->show();
        }
        if (workspace.panes.size() > 1 && !workspace.savedSplitSizes.isEmpty()) {
            const auto sizes = workspace.savedSplitSizes;
            QTimer::singleShot(0, workspace.splitter, [splitter = workspace.splitter, sizes] {
                if (splitter->count() == sizes.size()) splitter->setSizes(sizes);
            });
        }
        if (!workspace.panes.contains(m_activePane) || !m_activePane->isVisible()) setActivePane(workspace.panes.first());
        saveSettings();
        updateStatus();
        updateNavigationActions();
    }

    void clearSplit() {
        const auto index = m_pages->currentIndex();
        if (index < 0 || index >= m_workspaces.size()) return;
        auto& workspace = m_workspaces[index];
        if (workspace.visiblePaneCount > 1) workspace.savedSplitSizes = workspace.splitter->sizes();
        if (workspace.panes.size() > 1) workspace.panes.last()->hide();
        workspace.visiblePaneCount = 1;
        setActivePane(workspace.panes.isEmpty() ? nullptr : workspace.panes.first());
        saveSettings();
        updateStatus();
    }

    void updateStatus() {
        if (m_workspaces.isEmpty()) return;
        const auto& workspace = m_workspaces[m_pages->currentIndex()];
        updateSplitButton(workspace.visiblePaneCount);
        const auto paneLabel = workspace.visiblePaneCount > 1 ? tr("2 ペイン表示") : tr("1 ペイン表示");
        const auto direction = workspace.splitter->orientation() == Qt::Horizontal ? tr("左右") : tr("上下");
        statusBar()->showMessage(paneLabel + (workspace.visiblePaneCount > 1 ? QStringLiteral("（") + direction + QStringLiteral("）") : QString{})
                                 + tr("　|　フォルダーをダブルクリック　ファイルを右クリックで操作"));
    }

    void updateSplitButton(int paneCount) {
        if (!m_splitButton) return;
        m_splitButton->setText(tr("分割 %1").arg(paneCount));
        m_splitButton->setIcon(splitIndicatorIcon(paneCount));
    }

    QJsonObject workspaceConfig() {
        saveSettings();
        QSettings settings;
        const auto document = QJsonDocument::fromJson(settings.value("uiState").toByteArray());
        return document.object();
    }

    void exportWorkspaceConfig() {
        const auto path = QFileDialog::getSaveFileName(this, tr("構成をエクスポート"),
                                                       QDir::home().filePath("LightweightFileManager-layout.json"),
                                                       tr("JSON ファイル (*.json)"));
        if (path.isEmpty()) return;
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
            file.write(QJsonDocument(workspaceConfig()).toJson(QJsonDocument::Indented)) < 0) {
            QMessageBox::warning(this, tr("エクスポートできません"), tr("ファイルに構成を書き込めませんでした。"));
            return;
        }
        statusBar()->showMessage(tr("構成をエクスポートしました"), 3000);
    }

    void importWorkspaceConfig() {
        const auto path = QFileDialog::getOpenFileName(this, tr("構成をインポート"), QDir::homePath(),
                                                       tr("JSON ファイル (*.json)"));
        if (path.isEmpty()) return;
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("インポートできません"), tr("ファイルを読み込めませんでした。"));
            return;
        }
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
        const auto root = document.object();
        const auto workspaceArray = root.value("workspaces").toArray();
        if (parseError.error != QJsonParseError::NoError || !document.isObject() || workspaceArray.isEmpty()) {
            QMessageBox::warning(this, tr("インポートできません"), tr("有効なタブ構成 JSON ではありません。"));
            return;
        }
        for (const auto& value : workspaceArray) {
            if (!value.isObject() || value.toObject().value("name").toString().trimmed().isEmpty()) {
                QMessageBox::warning(this, tr("インポートできません"), tr("タブ名がない構成が含まれています。"));
                return;
            }
        }

        m_restoring = true;
        while (!m_workspaces.isEmpty()) {
            auto workspace = m_workspaces.takeLast();
            m_workspaceTabsLayout->removeWidget(workspace.button);
            m_pages->removeWidget(workspace.page);
            delete workspace.button;
            delete workspace.page;
        }
        m_activePane = nullptr;
        const auto home = QDir::homePath();
        for (const auto& value : workspaceArray) {
            const auto object = value.toObject();
            QVector<PaneState> panes;
            for (const auto& paneValue : object.value("panes").toArray()) {
                const auto paneObject = paneValue.toObject();
                PaneState pane;
                for (const auto& entry : paneObject.value("paths").toArray())
                    if (entry.isString() && !entry.toString().isEmpty()) pane.paths.append(entry.toString());
                pane.currentTab = paneObject.value("currentTab").toInt(0);
                panes.append(pane);
            }
            if (panes.isEmpty()) panes.append(defaultPane(home));
            while (panes.size() > 2) panes.removeLast();
            QList<int> sizes;
            for (const auto& entry : object.value("sizes").toArray()) sizes.append(entry.toInt());
            const auto orientation = object.value("orientation").toInt(static_cast<int>(Qt::Horizontal)) == static_cast<int>(Qt::Vertical)
                ? Qt::Vertical : Qt::Horizontal;
            const auto visiblePanes = qBound(1, object.value("visiblePanes").toInt(panes.size()), panes.size());
            addWorkspace(object.value("name").toString().trimmed(),
                         object.value("icon").toString(QStringLiteral("◫")),
                         orientation, panes, sizes, false, visiblePanes);
        }
        m_restoring = false;
        const auto current = qBound(0, root.value("currentWorkspace").toInt(0), m_workspaces.size() - 1);
        selectWorkspace(current, false);
        saveSettings();
    }

    void saveSettings() {
        if (m_restoring || m_workspaces.isEmpty()) return;
        QJsonArray workspaceArray;
        for (const auto& workspace : m_workspaces) {
            QJsonObject object;
            object.insert("name", workspace.name);
            object.insert("icon", workspace.icon);
            object.insert("orientation", static_cast<int>(workspace.splitter->orientation()));
            object.insert("visiblePanes", workspace.visiblePaneCount);
            QJsonArray paneArray;
            for (const auto* pane : workspace.panes) {
                QJsonObject paneObject;
                QJsonArray paths;
                for (const auto& path : pane->locationPaths()) paths.append(path);
                paneObject.insert("paths", paths);
                paneObject.insert("currentTab", pane->selectedTab());
                paneArray.append(paneObject);
            }
            object.insert("panes", paneArray);
            QJsonArray sizes;
            const auto storedSizes = workspace.visiblePaneCount > 1 ? workspace.splitter->sizes() : workspace.savedSplitSizes;
            for (const auto size : storedSizes) sizes.append(size);
            object.insert("sizes", sizes);
            workspaceArray.append(object);
        }
        QJsonObject root;
        root.insert("workspaces", workspaceArray);
        root.insert("currentWorkspace", m_pages->currentIndex());
        QSettings settings;
        settings.setValue("uiState", QJsonDocument(root).toJson(QJsonDocument::Compact));
        settings.sync();
    }
};
} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("LightweightFileManager");
    QApplication::setApplicationName("LightweightFileManager");
    app.setWindowIcon(applicationIcon());
    MainWindow window;
    window.show();
    return app.exec();
}

