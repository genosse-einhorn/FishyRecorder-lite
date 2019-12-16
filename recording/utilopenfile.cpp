#include "utilopenfile.h"

#ifdef Q_OS_WIN32
#   define STRICT_TYPED_ITEMIDS
#   include <windows.h>
#   include <shlobj.h>
#endif

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>

namespace Recording {

void Util::showFileInExplorer(const QString &file)
{
#if defined(Q_OS_WIN32)
    QString filename = QDir::toNativeSeparators(file);
    PIDLIST_ABSOLUTE pidl = nullptr;
    HRESULT hr = SHParseDisplayName(reinterpret_cast<const wchar_t*>(filename.utf16()), nullptr, &pidl, 0, nullptr);
    if (!SUCCEEDED(hr))
        qWarning() << "Failed SHParseDisplayName:" << hr;

    hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
    if (!SUCCEEDED(hr))
        qWarning() << "Failed SHOpenFolderAndSelectItems:" << hr;

    ILFree(pidl);
#elif defined(Q_OS_UNIX)
    auto url = QUrl::fromLocalFile(file).toString(QUrl::FullyEncoded);
    if (QProcess::execute("dbus-send", {
        "--print-reply",
        "--dest=org.freedesktop.FileManager1",
        "/org/freedesktop/FileManager1",
        "org.freedesktop.FileManager1.ShowItems",
        QString("array:string:%1").arg(url),
        "string:"
    }) != 0) {
        auto dirurl = QUrl::fromLocalFile(QFileInfo(file).absolutePath());
        QDesktopServices::openUrl(dirurl);
    }
#endif
}

} // namespace Recording
