#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 *  This file is a part of KNOSSOS.
 *
 *  (C) Copyright 2007-2013
 *  Max-Planck-Gesellschaft zur Foerderung der Wissenschaften e.V.
 *
 *  KNOSSOS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 of
 *  the License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * For further information, visit http://www.knossostool.org or contact
 *     Joergen.Kornfeld@mpimf-heidelberg.mpg.de or
 *     Fabian.Svara@mpimf-heidelberg.mpg.de
 */

#include "viewport.h"
#include "knossos-global.h"

#include <array>
#include <memory>

#define FILE_DIALOG_HISTORY_MAX_ENTRIES 10
#define LOCK_VP_ORIENTATION_DEFAULT (true)

#include <QMainWindow>
#include <QDropEvent>
#include <QQueue>
#include <QComboBox>
#include <QUndoStack>

class QLabel;
class QToolBar;
class QToolButton;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QMessageBox;
class QGridLayout;
class QFile;
class WidgetContainer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

    void updateSkeletonFileName(QString &fileName);

    void closeEvent(QCloseEvent *event);
    void updateTitlebar();

    static void reloadDataSizeWin();
    static void datasetColorAdjustmentsChanged();

signals:
    bool changeDatasetMagSignal(uint upOrDownFlag);
    void recalcTextureOffsetsSignal();
    void clearSkeletonSignal(int loadingSkeleton);
    bool loadSkeletonSignal(QString fileName);
    bool saveSkeletonSignal(QString fileName);
    void recentFileSelectSignal(int index);
    void updateToolsSignal();
    void updateTreeviewSignal();
    void updateCommentsTableSignal();
    void userMoveSignal(int x, int y, int z);

    void stopRenderTimerSignal();
    void startRenderTimerSignal(int frequency);
    void updateTreeColorsSignal();
    void loadTreeLUTFallback();

    treeListElement *addTreeListElementSignal(int treeID, color4F color);
    void nextCommentSignal(QString searchString);
    void previousCommentSignal(QString searchString);
    /* */
    void moveToPrevNodeSignal();
    void moveToNextNodeSignal();
    void moveToPrevTreeSignal();
    void moveToNextTreeSignal();
    bool popBranchNodeSignal();
    bool pushBranchNodeSignal(int setBranchNodeFlag, int checkDoubleBranchpoint, nodeListElement *branchNode, int branchNodeID);
    void jumpToActiveNodeSignal();

    bool addCommentSignal(QString content, nodeListElement *node, int nodeID);
    bool editCommentSignal(commentListElement *currentComment, int nodeID, QString newContent, nodeListElement *newNode, int newNodeID);

    void updateTaskDescriptionSignal(QString description);
    void updateTaskCommentSignal(QString comment);

    void treeAddedSignal(treeListElement *tree);
    void branchPushedSignal();
    void branchPoppedSignal();
    void nodeCommentChangedSignal(nodeListElement *node);
protected:
    void resizeEvent(QResizeEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void resizeViewports(int width, int height);
    void becomeFirstEntry(const QString &entry);
    QString openFileDirectory;
    QString saveFileDirectory;
public:
    QSpinBox *xField, *yField, *zField;
    std::array<std::unique_ptr<Viewport>, NUM_VP> viewports;

    // contains all widgets
    WidgetContainer *widgetContainer;

    QAction **historyEntryActions;

    QAction *addNodeAction;
    QAction *linkWithActiveNodeAction;
    QAction *dropNodesAction;

    QAction *dragDatasetAction;
    QAction *recenterOnClickAction;

    QQueue<QString> *skeletonFileHistory;
    QFile *loadedFile;

    QCheckBox *lockVPOrientationCheckbox;

    void createViewports();

    // for creating action, menus and the toolbar
    void createMenus();
    void createToolBar();

    // for save, load and clear settings
    void saveSettings();
    void loadSettings();
    void clearSettings();

public slots:
    // for the recent file menu
    bool loadSkeletonAfterUserDecision(QStringList fileNames);
    bool loadSkeletonAfterUserDecision(const QString &fileName);
    void updateFileHistoryMenu();
    bool alreadyInMenu(const QString &path);
    bool addRecentFile(const QString &fileName);
    //QUndoStack *undoStack;

    /* skeleton menu */
    void openSlot();
    void openSlot(QStringList fileNames); // for the drag n drop version
    void saveSlot();
    void saveAsSlot();

    /* edit skeleton menu*/
    void skeletonStatisticsSlot();
    void clearSkeletonSlotNoGUI();
    void clearSkeletonSlotGUI();
    void clearSkeletonWithoutConfirmation();

    /* view menu */
    void dragDatasetSlot();
    void recenterOnClickSlot();

    /* preferences menu */
    void loadCustomPreferencesSlot();
    void saveCustomPreferencesSlot();
    void defaultPreferencesSlot();

    /* window menu */
    void taskSlot();
    void logSlot();

    /* toolbar slots */
    void copyClipboardCoordinates();
    void pasteClipboardCoordinates();
    void coordinateEditingFinished();

    void updateCoordinateBar(int x, int y, int z);
    void recentFileSelected();
    void treeColorAdjustmentsChanged();
    // viewports
    void resetViewports();
    void resetVPOrientation();
    void lockVPOrientation(bool lock);
    void showVPDecorationClicked();

    // from the event handler
    void newTreeSlot();
    void nextCommentNodeSlot();
    void previousCommentNodeSlot();
    void pushBranchNodeSlot();
    void popBranchNodeSlot();
    void moveToPrevNodeSlot();
    void moveToNextNodeSlot();
    void moveToPrevTreeSlot();
    void moveToNextTreeSlot();
    void jumpToActiveNodeSlot();
    void F1Slot();
    void F2Slot();
    void F3Slot();
    void F4Slot();
    void F5Slot();
};

#endif // MAINWINDOW_H
