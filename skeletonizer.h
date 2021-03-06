#ifndef SKELETONIZER_H
#define SKELETONIZER_H

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


#include <QObject>
#include <QtCore>
#include "knossos-global.h"

class Skeletonizer : public QObject {
    Q_OBJECT
public:
    explicit Skeletonizer(QObject *parent = 0);

signals:
    void saveSkeletonSignal();
    void updateToolsSignal();
    void updateTreeviewSignal();
    void userMoveSignal(int x, int y, int z);
    void setRecenteringPositionSignal(int x, int y, int z);

public slots:
    static nodeListElement *findNearbyNode(treeListElement *nearbyTree, Coordinate searchPosition);
    static nodeListElement *getNodeWithPrevID(nodeListElement *currentNode, bool sameTree);
    static nodeListElement *getNodeWithNextID(nodeListElement *currentNode, bool sameTree);
    static treeListElement *getTreeWithPrevID(treeListElement *currentTree);
    static treeListElement *getTreeWithNextID(treeListElement *currentTree);
    static int addNode(int nodeID, float radius, int treeID, Coordinate *position, Byte VPtype, int inMag, int time, int respectLocks);

    static void *popStack(stack *stack);
    static bool pushStack(stack *stack, void *element);
    static stack *newStack(int size);
    static bool delStack(stack *stack);
    static bool delDynArray(dynArray *array);
    static void *getDynArray(dynArray *array, int pos);
    static bool setDynArray(dynArray *array, int pos, void *value);
    static dynArray *newDynArray(int size);

    static nodeListElement *addNodeListElement(int nodeID, float radius, nodeListElement **currentNode, Coordinate *position, int inMag);
    static segmentListElement* addSegmentListElement (segmentListElement **currentSegment, nodeListElement *sourceNode, nodeListElement *targetNode);

    void WRAP_popBranchNode();
    static void setColorFromNode(struct nodeListElement *node, color4F *color);
    static void setRadiusFromNode(struct nodeListElement *node, float *radius);
    static unsigned int commentContainsSubstr(struct commentListElement *comment, int index);

    void deleteSelectedTrees();
    void deleteSelectedNodes();

    static char *integerChecksum(int32_t in);
    static bool isObfuscatedTime(int time);

    static void resetSkeletonMeta();

    void UI_popBranchNode();
    static bool delTree(int treeID);
    static bool delActiveTree();
    static void clearTreeSelection();
    static void clearNodeSelection();
    static bool clearSkeleton(int loadingSkeleton);
    static bool delActiveNode();
    void autoSaveIfElapsed();
    bool genTestNodes(uint number);
    bool UI_addSkeletonNode(Coordinate *clickedCoordinate, Byte VPtype);
    static bool setActiveNode(nodeListElement *node, int nodeID);
    static bool addTreeComment(int treeID, QString comment);
    static bool unlockPosition();
    static bool lockPosition(Coordinate lockCoordinate);
    commentListElement *nextComment(QString searchString);
    commentListElement *previousComment(QString searchString);
    static bool delSegment(int sourceNodeID, int targetNodeID, segmentListElement *segToDel);
    static bool editNode(int nodeID, nodeListElement *node, float newRadius, int newXPos, int newYPos, int newZPos, int inMag);
    static bool delNode(int nodeID, nodeListElement *nodeToDel);
    static bool addComment(QString content, nodeListElement *node, int nodeID);
    static bool editComment(commentListElement *currentComment, int nodeID, QString newContent, nodeListElement *newNode, int newNodeID);
    static bool delComment(commentListElement *currentComment, int commentNodeID);
    bool jumpToActiveNode();
    static bool setActiveTreeByID(int treeID);

    bool loadXmlSkeleton(QString fileName);
    bool saveXmlSkeleton(QString fileName);

    static bool pushBranchNode(int setBranchNodeFlag, int checkDoubleBranchpoint, nodeListElement *branchNode, int branchNodeID);
    bool moveToNextTree();
    bool moveToPrevTree();
    bool moveToPrevNode();
    bool moveToNextNode();
    static bool moveNodeToTree(nodeListElement *node, int treeID);
    static treeListElement* findTreeByTreeID(int treeID);
    static nodeListElement *findNodeByNodeID(int nodeID);
    static bool addSegment(int sourceNodeID, int targetNodeID);
    static void restoreDefaultTreeColor(treeListElement *tree);
    static void restoreDefaultTreeColor();

    static int splitConnectedComponent(int nodeID);
    treeListElement *addTreeListElement(int treeID, color4F color);
    static bool mergeTrees(int treeID1, int treeID2);
    static bool updateTreeColors();
    static nodeListElement *findNodeInRadius(Coordinate searchPosition);
    static segmentListElement *findSegmentByNodeIDs(int sourceNodeID, int targetNodeID);
    uint addSkeletonNodeAndLinkWithActive(Coordinate *clickedCoordinate, Byte VPtype, int makeNodeActive);

    static QString getDefaultSkelFileName();
    bool searchInComment(char *searchString, commentListElement *comment);
    void popBranchNodeCanceled();
    bool popBranchNode();
    static bool updateCircRadius(struct nodeListElement *node);
    static int xorInt(int xorMe);

public:
    enum TracingMode {
        skipNextLink
        , linkedNodes
        , unlinkedNodes
    };

    TracingMode getTracingMode() const;
    void setTracingMode(TracingMode mode);
private:
    TracingMode tracingMode;
};

#endif // SKELETONIZER_H
