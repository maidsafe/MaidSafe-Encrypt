/*
 * copyright maidsafe.net limited 2009
 * The following source code is property of maidsafe.net limited and
 * is not meant for external use. The use of this code is governed
 * by the license file LICENSE.TXT found in the root of this directory and also
 * on www.maidsafe.net.
 *
 * You are not free to copy, amend or otherwise use this source code without
 * explicit written permission of the board of directors of maidsafe.net
 *
 *  Created on: Jan 06, 2010
 *      Author: Stephen Alexander
 */

#ifndef QT_WIDGETS_FILE_BROWSER_H_
#define QT_WIDGETS_FILE_BROWSER_H_

#include <boost/shared_ptr.hpp>

#include <QAction>
#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QProcess>

#include "ui_file_browser.h"

class FileBrowser : public QDialog {
  Q_OBJECT
 public:
  explicit FileBrowser(QWidget* parent = 0);
  virtual ~FileBrowser();

  virtual void setActive(bool active);
  virtual void reset();

  signals:
    void smilyChosen(int, int);

 private:
    void setMenuDirMenu();
    void setMenuFileMenu();
    QString getCurrentTreePath(QTreeWidgetItem* item);
  Ui::FileBrowserPage ui_;
  // QFileSystemModel* model_;
  // QFileSystemWatcher* theWatcher_;
  bool init_;
  QString currentDir_;
  QString currentTreeDir_;
  QString rootPath_;
  QMenu *menu;
  QAction *openFile;
  QAction *openWith;
  QAction *sendFile;
  QAction *copyFile;
  QAction *cutFile;
  QAction *deleteFile;
  QAction *renameFile;
  QAction *saveFile;
  QAction *newFolder;
  QMenu *menu2;
  boost::shared_ptr<QProcess> myProcess_;


  int populateDirectory(const QString);
  int createTreeDirectory(const QString);
  void uploadFileFromLocal(const QString& filePath);
  void saveFileToNetwork(const QString& filePath);
  void getTreeSubFolders(const QString);

  protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);
    void changeEvent(QEvent *event);

  private slots:
    void onItemDoubleClicked(QTreeWidgetItem*, int);
    void onFolderItemPressed(QTreeWidgetItem*, int);
    void onMousePressed(QTreeWidgetItem* item, int column);
    void onItemExpanded(QTreeWidgetItem* item);
    void onReadFileCompleted(int success, const QString& filepath);
    void onSaveFileCompleted(int success, const QString& filepath);
    void onMakeDirectoryCompleted(int success, const QString& dir);
    void onRemoveDirCompleted(int success, const QString& path);
    void onRenameFileCompleted(int success, const QString& filepath,
                                            const QString& newfilepath);

    void onOpenFileClicked();
// #ifdef MAIDSAFE_APPLE // TODO (Alec): Find out why this throws
    void onOpenWithClicked();
    void onSendFileClicked();
    void onCopyFileClicked();
    void onCutFileClicked();
    void onDeleteFileClicked();
    void onRenameFileClicked();
    void onSaveFileClicked();
    void onNewFolderClicked();
    void onBackClicked(bool);
    void onUploadClicked(bool);
    void onOpenError(QProcess::ProcessError);
    void onOpenStarted();
    void onOpenFinished(int, QProcess::ExitStatus);
    // void onWatchedFileChanged(const QString& path);
};

#endif  // QT_WIDGETS_FILE_BROWSER_H_
